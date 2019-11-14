#include <stdlib.h>
#include <string.h>
#include "hashtable.h"

/* Daniel J. Bernstein's "times 33" string hash function, from comp.lang.C;
   See https://groups.google.com/forum/#!topic/comp.lang.c/lSKWXiuNOAk */
unsigned long hash(char *str) {
  unsigned long hash = 5381;
  int c;

  while ((c = *str++))
    hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

  return hash;
}

hashtable_t *make_hashtable(unsigned long size) {
  hashtable_t *ht = malloc(sizeof(hashtable_t));
  ht->size = size;
  ht->buckets = calloc(sizeof(bucket_t *), size);
  return ht;
}

void ht_put(hashtable_t *ht, char *key, void *val) {
  /* FIXME: the current implementation doesn't update existing entries */
  unsigned int idx = hash(key) % ht->size;
 
  bucket_t *g = ht->buckets[idx];
  if (g) {
    
    while (g->next && strcmp(g->key, key) != 0) {
      g = g->next;
    }
    if (strcmp(g->key, key) == 0) {
      
      free(g->val);
      free(key);
      g->val = val;
    } else {
      bucket_t *b = malloc(sizeof(bucket_t));
      b->key = key;
      b->val = val;
      b->next = ht->buckets[idx];
      ht->buckets[idx] = b;
    }
  } else {
    bucket_t *b = malloc(sizeof(bucket_t));
    b->key = key;
    b->val = val;
    b->next = ht->buckets[idx];
    ht->buckets[idx] = b;
  }
}

void *ht_get(hashtable_t *ht, char *key) {
  unsigned int idx = hash(key) % ht->size;
 
  bucket_t *b = ht->buckets[idx];
  while (b) {
    if (strcmp(b->key, key) == 0) {
      return b->val;
    }
    b = b->next;
  }
  return NULL;
}

void ht_iter(hashtable_t *ht, int (*f)(char *, void *)) {
  bucket_t *b;
  unsigned long i;
  for (i=0; i<ht->size; i++) {
    b = ht->buckets[i];
    while (b) {
      if (!f(b->key, b->val)) {
        return ; // abort iteration
      }
      b = b->next;
    }
  }
}

void free_hashtable(hashtable_t *ht) {
  

  bucket_t *b;
  bucket_t *temp;
  unsigned long i;
  
  for(i=0; i<ht->size; i++) {
     b=ht->buckets[i];
     while(b) { 
        temp = b;
        b = b->next;
        free(temp->key);
        free(temp->val);
        free(temp);
     }
   }
   free(ht->buckets);
   free(ht); 
}


void  ht_del(hashtable_t *ht, char *key) {
  
  unsigned int idx = hash(key) % ht->size;
  bucket_t *b = ht->buckets[idx];
  bucket_t *temp;
  
  if (b) {
    if (strcmp(b->key, key) == 0) {
      temp = b->next;
      free(b->key);
      free(b->val);
      free(b);
      ht->buckets[idx] = temp;
    } else if (b->next) {
      while (b->next) {
        if (strcmp((b->next)->key, key) == 0) {
          temp = b->next;
          b->next = (b->next)->next;
          free(temp->key);
          free(temp->val);
          free(temp);
          return;
        }
         b = b->next;
      }
    }
  }
}

void  ht_rehash(hashtable_t *ht, unsigned long newsize) {
  
   bucket_t **nbucket = calloc(newsize,sizeof(bucket_t *)); 
   
   unsigned int nidx;
   unsigned long i;
   bucket_t *b;
   bucket_t *temp; 
   for(i=0; i<ht->size; i++) {
     b = ht->buckets[i];
     while(b) { 
        temp = b;
        
        nidx = hash(b->key) % newsize;
        bucket_t *nb = malloc(sizeof(bucket_t));

        nb->key = b->key;
        nb->val = b->val;
        nb->next = nbucket[nidx];
        nbucket[nidx] = nb;
        b = b->next;
        free(temp);
     }
   }

   free(ht->buckets);
   ht->size = newsize;
   ht->buckets = nbucket;
}
