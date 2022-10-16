#include "ModulesList.h"

// These mod list functions all need to be in their own 
// file, but the compiler doesn't like me.

/// @brief Allocates memory for a Link List
/// @return 
ModuleList_t *ModList_Create()
{
    ModuleList_t *list = (ModuleList_t *)calloc(0,sizeof(ModuleList_t));
    return list;
}

/// @brief Copies an item into the linked list at the end of the list
/// @param list 
/// @param item 
/// @return 1 on success, 0 otherwise
uint8_t ModList_Append(ModuleList_t *list, ModuleInfo *item)
{
    //ModuleInfo *newItem = calloc(0,sizeof(ModuleInfo));
    //memcpy(newItem,item,sizeof(ModuleInfo));
    ModuleListNode *newNode = (ModuleListNode *)calloc(0,sizeof(ModuleListNode));
    newNode->data = item;   

    if(!list->head)
        list->head = newNode;
    
    if(!list->tail)
    {
        list->tail = newNode;
    }
    else
    {
        list->tail->next = newNode;
        list->tail = newNode;
    }

    return 1;
}

ModuleListNode *_node = 0;

// Call this with a valid list to get the first time.
// Subsequent calls with a null argument will return
// the next item from the previous call until the end
// of the list. Think strtok
ModuleInfo *ModList_Iterate(ModuleList_t *list)
{
//    static ModuleListNode *_node = 0;
    if(_node)
        _node = _node->next;
    if(list)
        _node = list->head;
    if(_node)
        return _node->data;
    return 0;
}