#ifndef _MSORT_H_
#define _MSORT_H_

void msort(void* base, size_t nmemb, size_t size, int (*compar)(const void *, const void *));

#endif