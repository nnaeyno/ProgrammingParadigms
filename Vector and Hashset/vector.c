#include "vector.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <search.h>

void VectorNew(vector *v, int elemSize, VectorFreeFunction freeFn, int initialAllocation){
    assert(elemSize > 0 && initialAllocation >= 0);
    if(initialAllocation == 0) initialAllocation = 4;
    v->actLen = initialAllocation;
    v->logLen = 0;
    v->toGrow = initialAllocation;
    v->elemSize = elemSize;
    v->freeFn = freeFn;
    v->elems = malloc(elemSize * initialAllocation);
    assert(v->elems != NULL);
}

void VectorDispose(vector *v){
    if(v->freeFn != NULL){
        for(int i = 0; i < v->logLen; i++){
            v->freeFn((char*)v->elems + i * v->elemSize);
        }
    }
    free(v->elems);
}

int VectorLength(const vector *v) { 
    return v->logLen; 
}

void *VectorNth(const vector *v, int position) { 
    assert(position >= 0 && position < v->logLen);
    return (char*)v->elems + position * v->elemSize; 
}

void VectorReplace(vector *v, const void *elemAddr, int position){
    assert(position >= 0 && position < v->logLen && elemAddr != NULL);
    if(v->freeFn != NULL)
    v->freeFn((char*)v->elems + position * v->elemSize);
    memcpy((char*)v->elems + position * v->elemSize, elemAddr, v->elemSize);
}
void grow(vector* v) {
    //void* testArr = realloc(v, v->actLen * 2 * v->elemSize);
    v->elems = realloc(v->elems, (v->actLen + v->toGrow) * v->elemSize);
    assert(v->elems != NULL);
    v->actLen += v->toGrow;
  //  v->elems = testArr;
}

void VectorInsert(vector *v, const void *elemAddr, int position){
    assert(position >= 0 && position <= v->logLen && elemAddr != NULL);
    if(v->logLen == v->actLen) grow(v);
    
    for(int i = v->logLen-1; i >= position; i--){
       memmove((char*)v->elems + (i+1) * v->elemSize, (char*)v->elems + i * v->elemSize, v->elemSize);
    }
    memmove((char*)v->elems + position * v->elemSize, elemAddr, v->elemSize);
    v->logLen++;
}

void VectorAppend(vector *v, const void *elemAddr){
    assert(elemAddr != NULL);
   // printf("%d\n", 2);
    VectorInsert(v, elemAddr, v->logLen);
}

void VectorDelete(vector *v, int position){
    assert(position >= 0 && position < v->logLen);
    if(v->freeFn != NULL){
        v->freeFn((char*)v->elems + position * v->elemSize);
    }
    for(int i = position+1; i < v->logLen; i++){
       memmove((char*)v->elems + (i-1) * v->elemSize, (char*)v->elems + i * v->elemSize, v->elemSize);
    }
    v->logLen--;
}

void VectorSort(vector *v, VectorCompareFunction compare){
    assert(compare != NULL);
    qsort(v->elems, v->logLen, v->elemSize, compare);
}

void VectorMap(vector *v, VectorMapFunction mapFn, void *auxData){
    assert(mapFn != NULL);
    for (int i = 0; i < v->logLen; i++) {
        mapFn((char*)v->elems + i * v->elemSize, auxData);
    }
    
}

static const int kNotFound = -1;
int VectorSearch(const vector *v, const void *key, VectorCompareFunction searchFn, int startIndex, bool isSorted) {
    assert(startIndex >= 0 && startIndex <= v->logLen && searchFn != NULL && key != NULL);
    void* found;
    size_t num = v->logLen - startIndex;
    if(isSorted){
        found = bsearch(key, (char*)v->elems + startIndex*v->elemSize, num, v->elemSize, searchFn);
    } else {
       
        found = lfind(key, (char*)v->elems + v->elemSize * startIndex, &num, v->elemSize, searchFn);
    }
    if(found == NULL)
        return kNotFound;
    return ((char*)found - (char*)v->elems) / v->elemSize;
} 