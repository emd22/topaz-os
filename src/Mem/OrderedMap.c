#include "OrderedMap.h"
#include "Paging.h"
#include "TzString.h"

#include "TzKern.h"

Bool LessThanPredicateDefault(OrderedMapItem a, OrderedMapItem b) {
    return (a < b) ? 1 : 0;
}

OrderedArray OrderedArrayCreate(UInt32 max_size, LessThanPredicateFunc less_than_func) {
    OrderedArray oa;

    oa.array = (void * )TzKernAlloc(max_size * sizeof(OrderedMapItem));
    TzMemorySet(oa.array, 0, max_size * sizeof(OrderedMapItem));
    oa.size = 0;
    oa.max_size = max_size;
    oa.less_than_func = less_than_func;

    return oa;
}

OrderedArray OrderedArrayPlace(void *addr, UInt32 max_size, LessThanPredicateFunc less_than_func) {
    OrderedArray oa;


    oa.array = (OrderedMapItem * )addr;
    TzMemorySet(oa.array, 0, max_size * sizeof(OrderedMapItem));
    oa.size = 0;
    oa.max_size = max_size;
    oa.less_than_func = less_than_func;

    return oa;
}


void OrderedArrayDestroy(OrderedArray *array) {
    return;
}

void OrderedArrayAdd(OrderedArray *array, OrderedMapItem item) {
    TzAssert(array->less_than_func != NULL);
    UInt32 index = 0;
    while (index < array->size && array->less_than_func(array->array[index], item)) {
        index++;
    }

    if (index == array->size) {
        array->array[array->size++] = item;
    }
    else {
        OrderedMapItem temp = array->array[index];
        array->array[index] = item;

        while (index < array->size) {
            index++;

            OrderedMapItem temp2 = array->array[index];
            array->array[index] = temp;
            temp = temp2;
        }
        array->size++;
    }
}


OrderedMapItem OrderedArrayGet(OrderedArray *array, UInt32 index) {
    TzAssert(index < array->size);
    return array->array[index];
}

void OrderedArrayRemove(OrderedArray *array, UInt32 index) {
    while (index < array->size) {
        array->array[index] = array->array[++index];
    }
    array->size--;
}


