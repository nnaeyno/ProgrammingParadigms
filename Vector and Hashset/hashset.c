#include "hashset.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void HashSetNew(hashset *h, int elemSize, int numBuckets,
		HashSetHashFunction hashfn, HashSetCompareFunction comparefn, HashSetFreeFunction freefn){
	assert(elemSize > 0 && numBuckets > 0 && comparefn != NULL && hashfn != NULL);
	h->compFn = comparefn;
	h->hashFn = hashfn;
	h->elemSize = elemSize;
	h->numBuckets = numBuckets;
	h->size = 0;
	h->buckets = malloc(numBuckets * sizeof(vector*));
	for(int i = 0; i < numBuckets; i++){
		h->buckets[i] = malloc(sizeof(vector));
		VectorNew(h->buckets[i], elemSize, freefn, 4);
	}
}

void HashSetDispose(hashset *h){
	for(int i = 0; i < h->numBuckets; i++){
		VectorDispose(h->buckets[i]);
		free(h->buckets[i]);
	}
	free(h->buckets);
}

int HashSetCount(const hashset *h){ 
	return h->size; 
}

void HashSetMap(hashset *h, HashSetMapFunction mapfn, void *auxData){
	assert(mapfn != NULL);
	for(int i = 0; i < h->numBuckets; i++){
		VectorMap(h->buckets[i], mapfn, auxData);
	}
}

void HashSetEnter(hashset *h, const void *elemAddr){ 
	assert(elemAddr != NULL);
	int hash = h->hashFn(elemAddr, h->numBuckets);
	assert(hash >= 0 && hash < h->numBuckets);
	
	int lookUp =VectorSearch(h->buckets[hash], elemAddr, h->compFn, 0, false);

	if(lookUp == -1){	
	
		VectorAppend(h->buckets[hash], elemAddr); 
		
	} else {
		VectorReplace(h->buckets[hash], elemAddr, lookUp);
	}
	h->size++;

}

void *HashSetLookup(const hashset *h, const void *elemAddr){ 
	assert(elemAddr != NULL);
	int hash = h->hashFn(elemAddr, h->numBuckets);
	assert(hash >= 0 && hash < h->numBuckets);
	int ind = VectorSearch(h->buckets[hash], elemAddr, h->compFn, 0, false);
	if(ind == -1)
		return NULL; 
	return VectorNth(h->buckets[hash], ind);
}
