/* */
/*..................................................................*/

#ifndef SYMTABLE_INCLUDED
#define SYMTABLE_INCLUDED
#include <stddef.h>
/* A SymTable_T object ...*/
typedef struct SymTable *SymTable_T;
/*..................................................................*/

/* Return a new SymTable_T object, or NULL if insufficient memory is
   available. */

SymTable_T SymTable_new(void);
/*..................................................................*/

/* Free all memory occupied by oSymTable. */

void SymTable_free(SymTable_T oSymTable);

/*..................................................................*/

/* returns number of bindings in oSymTable. */
size_t SymTable_getLength(SymTable_T oSymTable);
/*..................................................................*/

/* Returns 1 if oSymTable does not contain a binding with 
   key pcKey and adds a new binding to oSymTable consisting of key 
   pcKey and value pvValue.
   Returns 0 if oSymTable already contains a binding with 
   key pcKey or if insufficient memory is available, and leaves 
   oSymTable unchanged. */
int SymTable_put(SymTable_T oSymTable, 
   const char *pcKey, const void *pvValue);
/*..................................................................*/

/* returns old binding value of oSymTable if oSymTable contains 
   binding with key pcKey and replaces the binding's value with 
   pvValue.
   returns NULL otherwise.*/
void *SymTable_replace(SymTable_T oSymTable,
   const char *pcKey, const void *pvValue);
/*..................................................................*/

/* returns 1 if oSymTable contains a binding whose key is pcKey. 
   returns 0 otherwise. */

int SymTable_contains(SymTable_T oSymTable, const char *pcKey);
/*..................................................................*/

/* returns value of binding in oSymTable whose key is pcKey.
   returns NULL otherwise. */

void *SymTable_get(SymTable_T oSymTable, const char *pcKey);
/*..................................................................*/

/* returns binding's value if osymTable contains binding with key 
   pcKey and removes binding from oSymTable. 
   returns NULL otherwise and doesn't change oSymTable. */

void *SymTable_remove(SymTable_T oSymTable, const char *pcKey);
/*..................................................................*/

/* applies function *pfApply to each binding in oSymTable, passing 
   pvExtra as an extra parameter.*/

void SymTable_map(SymTable_T oSymTable,
   void (*pfApply)(const char *pcKey, void *pvValue, void *pvExtra),
   const void *pvExtra);

#endif