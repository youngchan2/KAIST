#include <stdio.h>
#include <stdlib.h> // you can remove this if your code doesn't use it
#include <string.h> // you can remove this if your code doesn't use it
#include <time.h>
#include "quick.h"

size_t cnt;

void swap(void* base, size_t size, size_t a, size_t b){
    char* elementa = (char*)base+size*a;
    char* elementb = (char*)base+size*b;
    char* tmp = (char*)malloc(size);

    memcpy(tmp, elementa, size);
    memcpy(elementa, elementb, size);
    memcpy(elementb, tmp, size);

    free(tmp);
}

size_t medofthree(void* base, size_t size, int (*compar)(const void *, const void *), size_t left, size_t right){
    size_t mid = (left+right)/2;
    char* l = (char*)base + size*left;
    char* m = (char*)base + size*mid;
    char* r = (char*)base + size*right;
    if(compar(l, m)>0)
        swap(base, size, left, mid);
    if(compar(m, r)>0)
        swap(base, size, mid, right);
    if(compar(l,m)>0)
        swap(base, size, left, mid);
    cnt += 3;
    return mid;
}

size_t sort(void* base, size_t size, int (*compar)(const void*, const void*), size_t left, size_t right){
    size_t i = left;
    size_t j = right+1;
    char* pivot = (char*)base+left*size;
    while(1){
        while(compar(pivot, base+(++i)*size)>0){
            cnt++;
            if(i==right) break;
        }
        cnt++;
        while(compar(pivot, base+(--j)*size)<0){
            cnt++;
            if(j==left) break;
        }
        cnt++;
        if(i>=j) break;
        swap(base, size, i, j);
    }
    swap(base, size, left, j);
    return j;
}

void firstsort(void* base, size_t size, int (*compar)(const void *, const void *), size_t left, size_t right){
    if(right-left+1<=1)
        return;
    size_t j = sort(base, size, compar, left, right);
    firstsort(base, size, compar, left, j-1);
    firstsort(base, size, compar, j+1, right);
}

void lastsort(void* base, size_t size, int (*compar)(const void *, const void *), size_t left, size_t right){
    if(right-left+1<=1)
        return;
    swap(base, size, left, right);
    size_t j = sort(base, size, compar, left, right);
    lastsort(base, size, compar, left, j-1);
    lastsort(base, size, compar, j+1, right);
}

void randsort(void* base, size_t size, int (*compar)(const void *, const void *), size_t left, size_t right){
    if(right-left+1<=1)
        return;
    srand(time(NULL));
    size_t pi = rand()%(right-left+1)+left;
    swap(base, size, left, pi);
    size_t j = sort(base, size, compar, left, right);
    randsort(base, size, compar, left, j-1);
    randsort(base, size, compar, j+1, right);
}

void medsort(void* base, size_t size, int (*compar)(const void *, const void *), size_t left, size_t right){
    if(right-left+1<=1)
        return;
    size_t pi = medofthree(base, size, compar, left, right);
    swap(base, size, left, pi);
    size_t j = sort(base, size, compar, left, right);
    medsort(base, size, compar, left, j-1);
    medsort(base, size, compar, j+1, right);
}

void quick(void* base, size_t nmemb, size_t size, enum pivotSelType pivotType, int (*compar)(const void *, const void *))
{
    // default implementation is using qsort()
    // replace the following body with yours!
    
    void* rbase;
    switch (pivotType) {
        case FIRST :    
            cnt = 0;
            firstsort(base, size, compar, 0, nmemb-1);
            printf("First: %ld\n", cnt);
            break;
        case LAST :     
            cnt = 0;
            lastsort(base, size, compar, 0, nmemb-1);
            printf("Last: %ld\n", cnt);
            break;
        case RANDOM :
            cnt = 0;            
            rbase = malloc(nmemb*size);
            for(int i=0;i<10;i++){
                memcpy(rbase, base, nmemb*size);
                randsort(base, size, compar, 0, nmemb-1);
            }
            free(rbase);
            printf("Random: %ld\n", cnt/10);
            break;
        case MEDIANOF3 :
            cnt = 0;            
            medsort(base, size, compar, 0, nmemb-1);
            printf("Median-of-three: %ld\n", cnt);
            break;
        }

    return;
}