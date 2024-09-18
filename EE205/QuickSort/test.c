#include <stdio.h>
#include <stdlib.h>
#include "quick.h"

#define MAX_INPUT 1000000

/*-----------------------------------------------------------------------*/
static int 
IntIncreasingOrder(const void *A, const void *B)
{
    const int a = *(int *)A, b = *(int *)B;
    return (a > b) ? 1 : ((a < b) ? -1 : 0);
}
/*-----------------------------------------------------------------------*/
static int 
IntDecreasingOrder(const void *A, const void *B)
{
    return -IntIncreasingOrder(A, B);    
}
/*-----------------------------------------------------------------------*/
int 
main(void)
{
    int *inputA, *inputB, *inputC, *inputD, *inputE;
    int i = 0, j, count = 0;
    int increasing = 0;
    int match = 0;

    inputA = malloc (sizeof(int) * MAX_INPUT);
    inputB = malloc (sizeof(int) * MAX_INPUT);
    inputC = malloc (sizeof(int) * MAX_INPUT);
    inputD = malloc (sizeof(int) * MAX_INPUT);
    inputE = malloc (sizeof(int) * MAX_INPUT);

    if (inputA == NULL|| inputB == NULL) {
        fprintf(stderr, "memory allocation failed\n");
        return -1;
    }

    // get the input
    while (count < MAX_INPUT) {
        if (scanf("%d", &j) != 1)
            break; 
        // printf("input %d\n", j);
        inputA[i] = inputB[i] = inputC[i] = inputD[i] = inputE[i] = j;
        i++;
        count++;
    }
    if (count == 0) {
        fprintf(stderr, "no input inserted\n");
        return -1;
    }

    increasing = (count % 2 == 0);
    printf("Sorting %d numbers in %s order\n", 
            count, increasing ? "increasing" : "decreasing");
    
    // your quick sort. // UPDATE 8/10 - added parameter for pivot choice 
    quick(inputA, count, sizeof(int), FIRST, 
            increasing ? IntIncreasingOrder : IntDecreasingOrder);

    // native quick sort
    qsort(inputB, count, sizeof(int), 
            increasing ? IntIncreasingOrder : IntDecreasingOrder);

    quick(inputC, count, sizeof(int), LAST, 
            increasing ? IntIncreasingOrder : IntDecreasingOrder);

    quick(inputD, count, sizeof(int), RANDOM, 
            increasing ? IntIncreasingOrder : IntDecreasingOrder);

    quick(inputE, count, sizeof(int), MEDIANOF3, 
            increasing ? IntIncreasingOrder : IntDecreasingOrder);

    // comparison
    for (i = 0; i < count; i++){
        // printf("msort:\t%d \t%d \t%d \t%d \tqsort:\t%d\n",inputA[i], inputC[i],inputD[i],inputE[i], inputB[i]);
        if(inputC[i]==inputB[i]&&inputA[i]==inputD[i]&&inputD[i]==inputE[i]&&inputA[i]==inputC[i])
            match++;
    }
        

    printf("%d\n", match);
    
    // clean up
    free(inputA);
    free(inputB);
    
    return 0;
}