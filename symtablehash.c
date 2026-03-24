/*------------------------------------------------------------------*/
/* symtablehash.c                                                   */
/* Author: Kevin Tran                                               */
/*------------------------------------------------------------------*/
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "symtable.h"
/* initial number of buckets */
#define INITIAL_BUCKET_COUNT 509
/* number of possible expansions in the bucket count sequence*/
#define BUCKET_COUNT_SIZE 8
/*------------------------------------------------------------------*/
/* Each item/value and name/key is stored in a SymTableNode. 
   SymTableNodes are linked to form a linked list */
struct SymTableNode {
    /* the Item/Value in the symbol table*/
    const void *pvItem;
    /* the Name/Key in the symbol table*/
    const char *pcName;
    /* the next node in the linked list*/
    struct SymTableNode *psNextNode;
};
/*------------------------------------------------------------------*/
/* A SymTable is a structure that points to teh first SymTableNode 
   and also includes its length */
struct SymTable {
    /* number of buckets in symbol table*/
    size_t bucketCount;
    /* pointer to actual buckets with nodes */
    struct SymTableNode **symTable;
    /* number of nodes in symbol table*/
    size_t SymTableLength;
};
/*------------------------------------------------------------------*/
/* Return a hash code for pcKey that is between 0 and uBucketCount-1,
   inclusive. */

static size_t SymTable_hash(const char *pcKey, size_t uBucketCount)
{
   const size_t HASH_MULTIPLIER = 65599;
   size_t u;
   size_t uHash = 0;

   assert(pcKey != NULL);

   for (u = 0; pcKey[u] != '\0'; u++)
      uHash = uHash * HASH_MULTIPLIER + (size_t)pcKey[u];

   return uHash % uBucketCount;
}
/*------------------------------------------------------------------*/

SymTable_T SymTable_new(void) {
    SymTable_T oSymTable = (SymTable_T)
    malloc(sizeof(struct SymTable));
    if (oSymTable == NULL) {
        return NULL;
    }
    oSymTable->bucketCount = INITIAL_BUCKET_COUNT;
    oSymTable->SymTableLength = 0;
    oSymTable->symTable = calloc(INITIAL_BUCKET_COUNT, 
        sizeof(struct SymTableNode *));
    if (oSymTable->symTable == NULL) {
        free(oSymTable);
        return NULL;
    }
    return oSymTable;
}
/*------------------------------------------------------------------*/
void SymTable_free(SymTable_T oSymTable) {
    struct SymTableNode *psCurrentNode;
    struct SymTableNode *psNextNode;
    size_t hash = 0;
    assert(oSymTable != NULL);
    for (hash = 0; hash < oSymTable->bucketCount; hash++) {
        for (psCurrentNode = oSymTable->symTable[hash]; psCurrentNode
            != NULL; psCurrentNode = psNextNode) {
               /* free the key's defensive copy and node */
            psNextNode = psCurrentNode->psNextNode;
            free((char*)psCurrentNode->pcName);
            free(psCurrentNode); 
        }
        
    }
    free(oSymTable->symTable);
    free(oSymTable);
}
/*------------------------------------------------------------------*/
size_t SymTable_getLength(SymTable_T oSymTable) {
    assert(oSymTable != NULL);
    return oSymTable->SymTableLength;
}
/*------------------------------------------------------------------*/
/* Expands the hash table to the new bucket count stored in
   oSymTable->bucketCount. Rehashes all existing bindings into the
   new array. If memory allocation fails, restores bucketCount to
   oldBucketCount and returns without expanding. */
static void SymTable_expand(SymTable_T oSymTable,
    size_t oldBucketCount) {
    struct SymTableNode *psCurrentNode;
    struct SymTableNode *psNextNode;
    size_t newHash;
    size_t hash;
    struct SymTableNode **tempsymTable = calloc(
        oSymTable->bucketCount,
        sizeof(struct SymTableNode *));
    /* if calloc fails, implementation must proceed */
    /* restore bucketCount or else out of bounds later */
    if (tempsymTable == NULL) {
        oSymTable->bucketCount = oldBucketCount;
        return;
    }
    for (hash = 0; hash < oldBucketCount; hash++) {
        for (psCurrentNode = oSymTable->symTable[hash];
             psCurrentNode != NULL;
             psCurrentNode = psNextNode) {
            psNextNode = psCurrentNode->psNextNode;
            newHash = SymTable_hash(psCurrentNode->pcName,
                oSymTable->bucketCount);
            /* insert at head of new bucket's chain */
            psCurrentNode->psNextNode = tempsymTable[newHash];
            tempsymTable[newHash] = psCurrentNode;
        }
    }
    free(oSymTable->symTable);
    oSymTable->symTable = tempsymTable;
}
int SymTable_put(SymTable_T oSymTable,
    const char *pcKey, const void *pvValue) {
        struct SymTableNode *psNewNode;
        struct SymTableNode *psCurrentNode;
        /* set to NULL to silence splint warning */
        struct SymTableNode *psPreviousNode = NULL;
        size_t bucketCounts[] = {509, 1021, 2039, 4093, 8191, 16381,
            32749, 65521};
        char *pcKeyCopy;
        size_t hash;
        int i;
        assert(oSymTable != NULL);
        assert(pcKey != NULL);
        /* create defensive copy */
        pcKeyCopy = malloc(strlen(pcKey) + 1);
        if (pcKeyCopy == NULL) {
            return 0;
        }
        strcpy(pcKeyCopy, pcKey);
        psNewNode = (struct SymTableNode*)malloc(sizeof(struct
            SymTableNode));
        if (psNewNode == NULL) {
            /* if this malloc is null, must free first malloc */
            free(pcKeyCopy);
            return 0;
        }
        psNewNode->pvItem = pvValue;
        psNewNode->pcName = pcKeyCopy;
        psNewNode->psNextNode = NULL;
        if (oSymTable->SymTableLength > oSymTable->bucketCount) {
            size_t oldBucketCount = oSymTable->bucketCount;
            /* sets bucketCount to the next value in the bucketCount
               array unless it is the last index */
            for (i = 0; i < BUCKET_COUNT_SIZE - 1; i++) {
                if (oSymTable->bucketCount == bucketCounts[i]) {
                    oSymTable->bucketCount = bucketCounts[i + 1];
                    break;
                }
            }
            /* just for efficiency, won't run if bucket count
               was 65521 before and didn't change */
            if (oSymTable->bucketCount != oldBucketCount) {
                SymTable_expand(oSymTable, oldBucketCount);
            }
        }
        hash = SymTable_hash(pcKeyCopy, oSymTable->bucketCount);
        for (psCurrentNode = oSymTable->symTable[hash]; psCurrentNode
            != NULL; psCurrentNode = psCurrentNode->psNextNode) {
            /* traverses symboltable and sets psCurrentNode to
               last node */
            /* if the key is already in the symbol table */
            if (strcmp(psCurrentNode->pcName, pcKeyCopy) == 0) {
                /* frees the allocated memory and returns 0 */
                free(pcKeyCopy);
                free(psNewNode);
                return 0;
            }
            psPreviousNode = psCurrentNode;
        }
        /* corner case of empty bucket */
        if (oSymTable->symTable[hash] == NULL) {
            oSymTable->symTable[hash] = psNewNode;
        }
        else {
            /* We need psPreviousNode because psCurrentNode is NULL */
            psPreviousNode->psNextNode = psNewNode;
        }
        oSymTable->SymTableLength++;
        return 1;
    }
/*------------------------------------------------------------------*/
int SymTable_contains(SymTable_T oSymTable, const char *pcKey) {
    struct SymTableNode *psCurrentNode;
    size_t hash = SymTable_hash(pcKey, oSymTable->bucketCount);
    assert(oSymTable != NULL);
    assert(pcKey != NULL);
    for (psCurrentNode = oSymTable->symTable[hash]; psCurrentNode !=
         NULL; psCurrentNode = psCurrentNode->psNextNode) {
        if (strcmp(pcKey, psCurrentNode->pcName) == 0) {
            return 1;
        }
    }
    return 0;
}
/*------------------------------------------------------------------*/
void *SymTable_get(SymTable_T oSymTable, const char *pcKey) {
    struct SymTableNode *psCurrentNode;
    struct SymTableNode *psNextNode;
    size_t hash = SymTable_hash(pcKey, oSymTable->bucketCount);
    assert(oSymTable != NULL);
    assert(pcKey != NULL);
    for (psCurrentNode = oSymTable->symTable[hash]; psCurrentNode !=
         NULL; psCurrentNode = psNextNode) {
        psNextNode = psCurrentNode->psNextNode;
        if (strcmp(pcKey, psCurrentNode->pcName) == 0) {
            return (void*) psCurrentNode->pvItem;
        }
    }
    return NULL;
}
/*------------------------------------------------------------------*/
void *SymTable_remove(SymTable_T oSymTable, const char *pcKey) {
    struct SymTableNode *psCurrentNode;
    struct SymTableNode *psNextNode;
    /* for the first node corner case */
    struct SymTableNode *psPrevNode = NULL;
    void *oldpvValue;
    size_t hash = SymTable_hash(pcKey, oSymTable->bucketCount);
    assert(oSymTable != NULL);
    assert(pcKey != NULL);
    for (psCurrentNode = oSymTable->symTable[hash]; psCurrentNode !=
         NULL; psCurrentNode = psNextNode) {
        psNextNode = psCurrentNode->psNextNode;
        if (strcmp(pcKey, psCurrentNode->pcName) == 0) {
            oldpvValue = (void*) psCurrentNode->pvItem;
            if (psPrevNode == NULL) {
                /* corner case of removing first node */
                oSymTable -> symTable[hash] = psNextNode;
            }
            else {
                /* skips past current node */
                psPrevNode->psNextNode = psNextNode;
            }
            /* free the key's defensive copy and node */
            free((char*)psCurrentNode->pcName);
            free(psCurrentNode);
            oSymTable->SymTableLength--;
            return oldpvValue;
        }
        psPrevNode = psCurrentNode;
        
    }
    return NULL;
}
/*------------------------------------------------------------------*/
void SymTable_map(SymTable_T oSymTable,
    void (*pfApply)(const char *pcKey, void *pvValue, void *pvExtra),
    const void *pvExtra) {
    struct SymTableNode *psCurrentNode;
    size_t hash = 0;
    assert(oSymTable != NULL);
    assert(pfApply != NULL);
    for (hash = 0; hash < oSymTable->bucketCount; hash++) 
    {
        for (psCurrentNode = oSymTable->symTable[hash]; 
            psCurrentNode != NULL; psCurrentNode = 
            psCurrentNode->psNextNode) {
            (*pfApply) ((const char*) psCurrentNode->pcName, (void*) 
            psCurrentNode->pvItem, 
                (void*) pvExtra);
            
            }
        }
    }

