#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define HASHSIZE 10000
#define LOADFACTOR 0.7

struct Node
{
    char *word;
    int num;
    struct Node *next;
};

typedef struct Node node;

struct password
{
    node **hashmap;
    int curcnt;
    int cursize;
};

unsigned long hashcode(const char *str)
{
    unsigned long hash = 5381;
    while (*str)
    {
        hash = ((hash << 5) + hash) + *str++;
    }
    return hash % HASHSIZE;
}

int compare(const void *a, const void *b)
{
    return (((node *)b)->num) - (((node *)a)->num);
}

void resize(struct password *pw)
{
    if (pw->curcnt / pw->cursize < LOADFACTOR)
    {
        return;
    }

    int oldsize = pw->cursize;
    int newsize = 2 * oldsize;

    node **newmap = (node **)malloc(newsize * sizeof(node *));

    for (int i = 0; i < oldsize; i++)
    {
        node *current = pw->hashmap[i];
        while (current != NULL)
        {
            node *next = current->next;
            unsigned long newhash = hashcode(current->word) % newsize;
            current->next = newmap[newhash];
            newmap[newhash] = current;
            current = next;
        }
    }
    free(pw->hashmap);
    pw->hashmap = newmap;
    pw->cursize = newsize;
}

void insert(struct password *pw, const char *str)
{
    unsigned long hash = hashcode(str);

    node *new = (node *)malloc(sizeof(node));
    new->word = strdup(str);
    new->num = 1;
    new->next = NULL;

    node *current = pw->hashmap[hash];
    node *prev = NULL;

    while (current != NULL)
    {
        if (strcmp(current->word, str) == 0)
        {
            current->num++;
            free(new->word);
            free(new);
            return;
        }

        prev = current;
        current = current->next;
    }

    if (prev == NULL)
    {
        pw->hashmap[hash] = new;
    }
    else
    {
        prev->next = new;
    }

    pw->curcnt++;
    resize(pw);
}

node *allnodes(struct password *pw)
{
    node *all = (node *)malloc(pw->curcnt * sizeof(node));

    int index = 0;

    for (int i = 0; i < HASHSIZE; i++)
    {
        node *current = pw->hashmap[i];
        while (current != NULL)
        {
            all[index].word = current->word;
            all[index].num = current->num;
            all[index].next = current->next;
            index++;
            current = current->next;
        }
    }
    return all;
}

void print(struct password *pw, int n1, int n2)
{
    if (n2 > 0)
        printf("The %d most commonly occuring %d character substrings are:\n", n1, n2);
    else if (n1 > 0)
        printf("The %d most commonly occurring passwords are:\n", n1);
    else
        printf("The most commonly occurring passwords are:\n");

    int n = n1 > 0 ? n1 : pw->curcnt;

    node *all = allnodes(pw);
    qsort(all, pw->curcnt, sizeof(node), compare);

    for (int i = 0; i < n; i++)
    {
        if (all[i].num > 0)
            printf("%d\t%s\n", all[i].num, all[i].word);
    }
    free(all);
}

void freeall(struct password *pw)
{
    for (int i = 0; i < HASHSIZE; i++)
    {
        node *current = pw->hashmap[i];
        while (current != NULL)
        {
            node *temp = current;
            current = current->next;
            free(temp->word);
            free(temp);
        }
    }
}

int main(int argc, char *argv[])
{
    struct password pw;
    pw.hashmap = (node **)malloc(HASHSIZE * sizeof(node *));
    pw.curcnt = 0;
    pw.cursize = HASHSIZE;

    for (int i = 0; i < HASHSIZE; i++)
    {
        pw.hashmap[i] = NULL;
    }

    clock_t s = clock();

    char c[50];
    int n1 = -1;
    int n2 = -1;
    if (argc == 2)
    {
        n1 = atoi(argv[1]);
    }
    else if (argc == 3)
    {
        n1 = atoi(argv[1]);
        n2 = atoi(argv[2]);
    }

    while (fgets(c, sizeof(c), stdin))
    {
        c[strlen(c) - 1] = '\0';
        if (n2 > 0)
        {
            int l = strlen(c);
            for (int i = 0; i <= l - n2; i++)
            {
                char *sub = malloc(n2 + 1);
                strncpy(sub, c + i, n2);
                sub[n2] = '\0';
                insert(&pw, sub);
                free(sub);
            }
        }
        else
        {
            insert(&pw, c);
        }
    }

    print(&pw, n1, n2);

    freeall(&pw);

    clock_t e = clock();
    double ti = (double)(e - s) / CLOCKS_PER_SEC;
    printf("time: %fsec\n", ti);

    return 0;
}