#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "memlib.h"
#include "mm.h"

#define WSIZE 4
#define DSIZE 8
#define CHUNKSIZE (1 << 12)
#define LISTSIZE (20)

#define PACK(size, alloc) (size | alloc)
#define GET(p) (*(unsigned int *)(p))
#define PUT(p, val) (*(unsigned int *)(p) = (unsigned int)(val))
#define GETSIZE(p) (GET(p) & ~0x7)
#define GETALLOC(p) (GET(p) & 0x1)

#define HEADER(bp) ((char *)(bp)-WSIZE)
#define FOOTER(bp) ((char *)(bp) + GETSIZE(HEADER(bp)) - DSIZE)
#define NEXTBLOCK(bp) ((char *)(bp) + GETSIZE(((char *)(bp)-WSIZE)))
#define PREVBLOCK(bp) ((char *)(bp)-GETSIZE(((char *)(bp)-DSIZE)))

/*
 * seglist macro
 */
static char *heap;
#define PREVSEG(bp) (*(char **)((char *)(bp)))
#define NEXTSEG(bp) (*(char **)((char *)(bp) + WSIZE))
#define ROOT(class) (*(char **)((heap) + (WSIZE * (class))))

int calclass(size_t size)
{
    int log = 0;
    size_t tmp = size;
    while ((log < LISTSIZE) && (tmp > 1))
    {
        tmp >>= 1;
        log++;
    }
    return log - 4;
}

static void insert(void *bp)
{
    int class = calclass(GETSIZE(HEADER(bp)));
    if (ROOT(class) == NULL)
    {
        ROOT(class) = bp;
        NEXTSEG(bp) = NULL;
        PREVSEG(bp) = NULL;
    }
    else
    {
        NEXTSEG(bp) = NEXTSEG(ROOT(class));
        PREVSEG(bp) = ROOT(class);
        NEXTSEG(ROOT(class)) = bp;

        if (NEXTSEG(bp) != NULL)
            PREVSEG(NEXTSEG(bp)) = bp;
    }
    return;
}

static void delete(void *bp)
{
    int class = calclass(GETSIZE(HEADER(bp)));
    if (bp == ROOT(class))
    {
        ROOT(class) = NEXTSEG(bp);
    }
    else
    {
        NEXTSEG(PREVSEG(bp)) = NEXTSEG(bp);
        if (NEXTSEG(bp) != NULL)
            PREVSEG(NEXTSEG(bp)) = PREVSEG(bp);
    }
    return;
}

static void *coalesce(void *bp)
{
    size_t prevalloc = GETALLOC(FOOTER(PREVBLOCK(bp)));
    size_t nextalloc = GETALLOC(HEADER(NEXTBLOCK(bp)));
    size_t size = GETSIZE(HEADER(bp));

    if (prevalloc && nextalloc)
    {
        insert(bp);
        return bp;
    }
    else if (prevalloc && !nextalloc)
    {
        delete (NEXTBLOCK(bp));
        size += GETSIZE(HEADER(NEXTBLOCK(bp)));
        PUT(HEADER(bp), PACK(size, 0));
        PUT(FOOTER(bp), PACK(size, 0));
    }
    else if (!prevalloc && nextalloc)
    {
        delete (PREVBLOCK(bp));
        size += GETSIZE(HEADER(PREVBLOCK(bp)));
        PUT(HEADER(PREVBLOCK(bp)), PACK(size, 0));
        PUT(FOOTER(bp), PACK(size, 0));
        bp = PREVBLOCK(bp);
    }
    else
    {
        delete (PREVBLOCK(bp));
        delete (NEXTBLOCK(bp));
        size += GETSIZE(HEADER(PREVBLOCK(bp))) + GETSIZE(FOOTER(NEXTBLOCK(bp)));
        PUT(HEADER(PREVBLOCK(bp)), PACK(size, 0));
        PUT(FOOTER(NEXTBLOCK(bp)), PACK(size, 0));
        bp = PREVBLOCK(bp);
    }
    insert(bp);
    return bp;
}

static void *extend(size_t words)
{
    char *bp;
    size_t size = (words % 2) ? (words + 1) * WSIZE : words * WSIZE;

    if ((long)(bp = mem_sbrk(size)) == -1)
        return NULL;

    PUT(HEADER(bp), PACK(size, 0));
    PUT(FOOTER(bp), PACK(size, 0));
    PUT(HEADER(NEXTBLOCK(bp)), PACK(0, 1));

    return coalesce(bp);
}

static void *fit(size_t size)
{
    int class = calclass(size);
    char *bp;

    for (int i = class; i < LISTSIZE; i++)
    {
        bp = ROOT(i);
        while (bp != NULL)
        {
            if (size <= GETSIZE(HEADER(bp)))
                return bp;
            bp = NEXTSEG(bp);
        }
    }
    return NULL;
}

static void place(void *bp, size_t size)
{
    delete (bp);

    size_t csize = GETSIZE(HEADER(bp));

    if ((csize - size) >= (4 * WSIZE))
    {
        PUT(HEADER(bp), PACK(size, 1));
        PUT(FOOTER(bp), PACK(size, 1));
        bp = NEXTBLOCK(bp);
        PUT(HEADER(bp), PACK((csize - size), 0));
        PUT(FOOTER(bp), PACK((csize - size), 0));
        insert(bp);
    }
    else
    {
        PUT(HEADER(bp), PACK(csize, 1));
        PUT(FOOTER(bp), PACK(csize, 1));
    }
}

/*
 * mm_init - initialize the malloc package.
 * The return value should be -1 if there was a problem in performing the initialization, 0
 * otherwise
 */
int mm_init(void)
{
    if ((heap = mem_sbrk((LISTSIZE + 4) * WSIZE)) == (void *)-1)
        return -1;
    PUT(heap, 0);
    PUT(heap + WSIZE, PACK((LISTSIZE + 2) * WSIZE, 1));
    for (int i = 0; i < LISTSIZE; i++)
        PUT(heap + ((2 + i) * WSIZE), NULL);
    PUT(heap + ((LISTSIZE + 2) * WSIZE), PACK((LISTSIZE + 2) * WSIZE, 1));
    PUT(heap + ((LISTSIZE + 3) * WSIZE), PACK(0, 1));
    heap += (2 * WSIZE);
    if (extend(CHUNKSIZE / WSIZE) == NULL)
        return -1;
    return 0;
}

/*
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    size_t asize;
    size_t extendsize;
    char *bp;

    if (size == 0)
        return NULL;

    if (size <= DSIZE)
        asize = 2 * DSIZE;
    else
        asize = DSIZE * ((size + DSIZE + DSIZE - 1) / DSIZE);

    if ((bp = fit(asize)) != NULL)
    {
        place(bp, asize);
        return bp;
    }

    extendsize = asize > CHUNKSIZE ? asize : CHUNKSIZE;
    if ((bp = extend(extendsize / WSIZE)) == NULL)
        return NULL;
    place(bp, asize);
    return bp;
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *bp)
{
    size_t size = GETSIZE(HEADER(bp));
    PUT(HEADER(bp), PACK(size, 0));
    PUT(FOOTER(bp), PACK(size, 0));
    coalesce(bp);
    return;
}

/*
 * mm_realloc
 */
void *mm_realloc(void *ptr, size_t size)
{
    void *new = NULL;
    size_t asize;
    size_t csize = GETSIZE(HEADER(ptr));
    size_t sizesum = csize;
    int space;

    if (size <= 0)
        return NULL;
    else if (size <= DSIZE)
        asize = 2 * DSIZE;
    else
        asize = DSIZE * ((size + DSIZE + DSIZE - 1) / DSIZE);

    space = csize - asize;

    if (space < 0)
    {
        if (!GETALLOC(HEADER(NEXTBLOCK(ptr))))
        {
            sizesum += GETSIZE(HEADER(NEXTBLOCK(ptr)));
            if (sizesum >= asize)
            {
                delete (NEXTBLOCK(ptr));
                PUT(HEADER(ptr), PACK(asize, 1));
                PUT(FOOTER(ptr), PACK(asize, 1));
                PUT(HEADER(NEXTBLOCK(ptr)), PACK(sizesum - asize, 0));
                PUT(FOOTER(NEXTBLOCK(ptr)), PACK(sizesum - asize, 0));
                insert(NEXTBLOCK(ptr));
                return ptr;
            }
        }
        else
        {
            new = mm_malloc(asize);
            memcpy(new, ptr, csize);
            mm_free(ptr);
            return new;
        }
    }
    return ptr;
}
