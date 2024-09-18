#ifndef _QUICK_H_
#define _QUICK_H_

#ifndef _PIVOT_TYPES
#define _PIVOT_TYPES

enum pivotSelType {
  FIRST,
  LAST,
  RANDOM,
  MEDIANOF3
};

#endif

// UPDATE 8/10 - added parameter for pivot choice
void quick(void* base, size_t nmemb, size_t size, enum pivotSelType pivotType, int (*compar)(const void *, const void *));

#endif