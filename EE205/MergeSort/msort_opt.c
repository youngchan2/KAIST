#include <stdlib.h> // you can remove this if your code doesn't use it
#include <string.h> // you can remove this if your code doesn't use it

void merge(void *base, size_t start, size_t mid, size_t end, size_t size, int (*compar)(const void *, const void *)) {
    char *tmp = malloc((end-start+1)*size);
    char *left = (char*)base+start*size;
    char *right = (char*)base+((mid+1)*size);
    char *tmpinit = tmp;

    size_t i = start;
    size_t j = mid+1;

    /*
    choose one element in left or right array by using 'compar' function
    until the one of the array has been finished.
    */
    while(i<=mid && j<=end){
        if(compar(left, right)<=0){
            for(size_t k=0;k<size;k++){
                *(tmpinit+k) = *(left+k);
            }
            left+=size;
            i++;
        }
        else{
            for(size_t k=0;k<size;k++){
                *(tmpinit+k) = *(right+k);
            }
            right+=size;
            j++;
        }
        tmpinit+=size;
    }

    /*
    if left array still has elements, then copy all the remains to 'tmp' array
    */
    while(i<=mid){
        for(size_t k=0;k<size;k++){
            *(tmpinit+k) = *(left+k);
        }
        left+=size;
        i++;
        tmpinit+=size;
    }

    /*
    if right array still has elements, then copy all the remains to 'tmp' array
    */
    while(j <= end){
        for(size_t k=0;k<size;k++){
            *(tmpinit+k)=*(right+k);
        }
        right+=size;
        j++;
        tmpinit+=size;
    }

    /*
    copy all of the elements in tmp array to base
    */
    for(size_t k=0;k<(end-start+1)*size;k++){
        *(char*)(base+start*size+k) = *(tmp+k);
    }
    free(tmp);
}

void mergesort(void *base, size_t start, size_t end, size_t size, int (*compar)(const void *, const void *)) {
    if(start<end){
        size_t mid = (end+start) / 2;
        /*
        divide the base array by two
        */
        mergesort(base, start, mid, size, compar);
        mergesort(base, mid+1, end, size, compar);
        /*
        check whether the array is sorted or not
        */
        if(compar((char*)base+(mid+1)*size, (char*)base+mid*size)>0)
            return;
        else
            merge(base, start, mid, end, size, compar);
    }
}
/*
implement the insertion sort
*/
void isort(void *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *)) {
    char *basearray = (char *)base;
    
    for (size_t i=1;i<nmemb;i++) {
        for (size_t j=i;j>0;j--) {
            char *current = basearray+j*size;
            char *previous = basearray+(j - 1)*size;
            if (compar(previous, current)>0) {
                for (size_t k=0;k<size;k++) {
                    char tmp = *(previous+k);
                    *(previous+k) = *(current+k);
                    *(current+k) = tmp;
                }
            }
            else
                break;
        }
    }
}

void msort(void* base, size_t nmemb, size_t size, int (*compar)(const void *, const void *))
{
    // default implementation is using qsort()
    // replace the following body with yours!
    if(nmemb<17)
        isort(base, nmemb, size, compar);
    else
        mergesort(base, 0, nmemb-1, size, compar);
}
