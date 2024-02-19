#ifndef TOPAZ_MEMORY_ORDERED_MAP
#define TOPAZ_MEMORY_ORDERED_MAP

#include "../Types.h"


typedef void * OrderedMapItem;
typedef Bool (*LessThanPredicateFunc)(OrderedMapItem, OrderedMapItem);

typedef struct {
    OrderedMapItem *array;
    UInt32 size;
    UInt32 max_size;
    LessThanPredicateFunc less_than_func;
} OrderedArray;


Int8 LessThanPredicateDefault(OrderedMapItem a, OrderedMapItem b);

OrderedArray OrderedArrayCreate(UInt32 max_size, LessThanPredicateFunc less_than_func);
OrderedArray OrderedArrayPlace(void *addr, UInt32 max_size, LessThanPredicateFunc less_than_func);
void OrderedArrayDestroy(OrderedArray *array);
void OrderedArrayAdd(OrderedArray *array, OrderedMapItem item);
OrderedMapItem OrderedArrayGet(OrderedArray *array, UInt32 index);
void OrderedArrayRemove(OrderedArray *array, UInt32 index);




#endif