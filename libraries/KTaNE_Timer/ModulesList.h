#ifndef _MODULESLIST_H_
#define _MODULESLIST_H_

#include <stdint.h>
#include "ModulesInteraction.h"

typedef struct ModuleListNode
{
    ModuleInfo *data;
    struct ModuleListNode *next;
}
ModuleListNode;

typedef struct
{
    ModuleListNode *head;
    ModuleListNode *tail;
}
ModuleList_t;

/// @brief Allocates memory for a Link List
/// @return 
ModuleList_t *ModList_Create();

/// @brief COPIES an item into the linked list at the end of the list
/// @param list 
/// @param item 
/// @return 1 on success, 0 otherwise
uint8_t ModList_Append(ModuleList_t *list, ModuleInfo *item);

/// @brief Remove and free all items in the list (does not free the list itself)
/// @param list 
/// @return 1 on success, 0 otherwise
uint8_t ModList_Clear(ModuleList_t *list);

// Call this with a valid list to get the first time.
// Subsequent calls with a null argument will return
// the next item from the previous call until the end
// of the list. Think strtok
ModuleInfo *ModList_Iterate(ModuleList_t *list);

#endif /* _MODULESLIST_H_ */