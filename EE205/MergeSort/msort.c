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

void mergesort(void *base, size_t start, size_t end, size_t size, int (*compare)(const void *, const void *)) {
    if(start<end){
        size_t mid = (end+start) / 2;
        /*
        divide the base array by two
        */
        mergesort(base, start, mid, size, compare);
        mergesort(base, mid+1, end, size, compare);

        merge(base, start, mid, end, size, compare);
    }
}

void msort(void* base, size_t nmemb, size_t size, int (*compar)(const void *, const void *))
{
    // default implementation is using qsort()
    // replace the following body with yours!
    mergesort(base, 0, nmemb-1, size, compar);
}

