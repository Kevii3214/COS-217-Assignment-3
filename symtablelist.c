/*------------------------------------------------------------------*/
/* symtablelist.c                                                   */
/* Author: Kevin Tran                                               */
/*------------------------------------------------------------------*/
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "symtable.h"
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
    /* address of first SymTableNode*/
    struct SymTableNode *psFirstNode;
    /* number of nodes in symbol table*/
    size_t SymTableLength;
};
/*------------------------------------------------------------------*/
SymTable_T SymTable_new(void) {
    SymTable_T oSymTable;
    oSymTable = (SymTable_T)malloc(sizeof(struct SymTable));
    if (oSymTable == NULL) {
        return NULL;
    }
    oSymTable->psFirstNode = NULL;
    oSymTable->SymTableLength = 0;
    return oSymTable;
}
/*------------------------------------------------------------------*/
void SymTable_free(SymTable_T oSymTable) {
    struct SymTableNode *psCurrentNode;
    struct SymTableNode *psNextNode;
    assert(oSymTable != NULL);
    for (psCurrentNode = oSymTable->psFirstNode; psCurrentNode !=
         NULL; psCurrentNode = psNextNode) {
        psNextNode = psCurrentNode->psNextNode;
        free((char*)psCurrentNode->pcName);
        free(psCurrentNode);
    }
    free(oSymTable);
}
/*------------------------------------------------------------------*/
size_t SymTable_getLength(SymTable_T oSymTable) {
    /* should be constant time */
    assert(oSymTable != NULL);
    return oSymTable->SymTableLength;
}
/*------------------------------------------------------------------*/
int SymTable_put(SymTable_T oSymTable,
    const char *pcKey, const void *pvValue) {
        struct SymTableNode *psNewNode;
        struct SymTableNode *psCurrentNode;
        /* set to NULL to silence splint warning */
        struct SymTableNode *psPreviousNode = NULL;
        char *pcKeyCopy;
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
        for (psCurrentNode = oSymTable->psFirstNode; psCurrentNode 
            != NULL; psCurrentNode = psCurrentNode->psNextNode) {
        /* traverses symboltable and sets psCurrentNode to last node*/
        /* if the key is already in the symbol table*/
        if (strcmp(psCurrentNode->pcName, pcKeyCopy) == 0) {
            /* frees the allocated memory and returns 0*/
            free(pcKeyCopy);
            free(psNewNode);
            return 0;
        }
        psPreviousNode = psCurrentNode;
        }
        /* corner case of empty symbol table*/
        if (oSymTable->psFirstNode == NULL) {
            oSymTable->psFirstNode = psNewNode;
        }
        else{
            psPreviousNode->psNextNode = psNewNode;
        }
        /* We need psPreviousNode because psCurrentNode is NULL */
        oSymTable->SymTableLength++;
        return 1;
    }
/*------------------------------------------------------------------*/
void *SymTable_replace(SymTable_T oSymTable,
    const char *pcKey, const void *pvValue) {
    struct SymTableNode *psCurrentNode;
    struct SymTableNode *psNextNode;
    void *oldpvValue;
    assert(oSymTable != NULL);
    assert(pcKey != NULL);
    for (psCurrentNode = oSymTable->psFirstNode; psCurrentNode !=
         NULL; psCurrentNode = psNextNode) {
        psNextNode = psCurrentNode->psNextNode;
        /* if keys match, swap the item and return old value*/
        if (strcmp(pcKey, psCurrentNode->pcName) == 0) {
            oldpvValue = (void*) psCurrentNode->pvItem;
            psCurrentNode->pvItem = pvValue;
            return oldpvValue;
        }
    }
    return NULL;
    }
/*------------------------------------------------------------------*/
int SymTable_contains(SymTable_T oSymTable, const char *pcKey) {
    struct SymTableNode *psCurrentNode;
    assert(oSymTable != NULL);
    assert(pcKey != NULL);
    for (psCurrentNode = oSymTable->psFirstNode; psCurrentNode !=
         NULL; psCurrentNode = psCurrentNode->psNextNode) {
        /* if keys match and return 1*/
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
    assert(oSymTable != NULL);
    assert(pcKey != NULL);
    for (psCurrentNode = oSymTable->psFirstNode; psCurrentNode !=
         NULL; psCurrentNode = psNextNode) {
        psNextNode = psCurrentNode->psNextNode;
        /* if keys match, get the item and return item*/
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
    assert(oSymTable != NULL);
    assert(pcKey != NULL);
    for (psCurrentNode = oSymTable->psFirstNode; psCurrentNode !=
         NULL; psCurrentNode = psNextNode) {
        psNextNode = psCurrentNode->psNextNode;
        /* if keys match, remove the node and return old value*/
        if (strcmp(pcKey, psCurrentNode->pcName) == 0) {
            oldpvValue = (void*) psCurrentNode->pvItem;
            if (psPrevNode == NULL) {
                /* corner case of removing first node */
                oSymTable -> psFirstNode = psNextNode;
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
        assert(oSymTable != NULL);
        assert(pfApply != NULL);
        /* applies function to all nodes */
        for (psCurrentNode = oSymTable->psFirstNode; psCurrentNode !=
        NULL; psCurrentNode = psCurrentNode->psNextNode) {
            (*pfApply) ((const char*) psCurrentNode->pcName, (void*) 
            psCurrentNode->pvItem, 
                (void*) pvExtra);
            
    }
    }