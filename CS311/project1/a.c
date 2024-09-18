#include <stdio.h>
#include <stdlib.h>

int main()
{
    int *a = (int *)malloc(sizeof(int) * 3);

    *a = 2;
    *(a + 4) = 4;
    *(a + 8) = 8;

    printf("%d\n", *(a + 8));
    // printf("%x\n", *(a + 6));
    printf("%d\n", *(a + 8));
}