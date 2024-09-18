#include <stdio.h>
#include <stdlib.h>

typedef struct product {
    int id;              // product id number # (1..n)
    int salesVolume;     // # of units sold
    int meanRating;      // mean user rating of product
} Product;

/*-----------------------------------------------------------------*/
int compareProducts(const void *A, const void *B)
{
    int meana = ((Product*)A)->meanRating;
    int meanb = ((Product*)B)->meanRating;
    int vola = ((Product*)A)->salesVolume;
    int volb = ((Product*)B)->salesVolume;
    return (meana>meanb)?-1:((meana<meanb)?1:((vola>volb)?-1:(vola<volb)?1:0));
}
/*-----------------------------------------------------------------*/
int 
main()
{
    Product *products;
    int i, n;
    
    if (scanf("%d\n", &n) != 1) {
        fprintf(stderr, "Error in input\n");
        return -1;  
    }

    products = malloc(n *sizeof(Product));
    if (products == NULL) {
        fprintf(stderr, "malloc() failed\n");
        return -1;
    }

    // get the input
    for (i = 0; i < n; i++) {
        if (scanf("%d %d\n", 
                  &products[i].salesVolume, &products[i].meanRating) != 2)
            break;  
        products[i].id = i + 1; 
    }
    if (i != n) {
        fprintf(stderr, "wrong input!\n");
        free(products);
        return -1;
    }
    // compute and print out the sequence of products ranked by rating (first) and units sold (second)
    qsort(products, n, sizeof(Product), compareProducts);
    for(i=0;i<n;i++)
        printf("%d ", products[i].id);
    printf("\n");
    free(products);

}