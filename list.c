#include <assert.h>
#include <stdio.h>
#include <stdbool.h>

#include "list.h"

static List listArray[LIST_MAX_NUM_HEADS+3];
static Node nodeArray[LIST_MAX_NUM_NODES+2];
static Node fakeNodeArray[LIST_MAX_NUM_HEADS*2+2];
static List* freeNodeList = &(listArray[LIST_MAX_NUM_HEADS+2]);

static void getOrCreateFreeNodeList(){
    if(freeNodeList->fakeListHead){
        return;
    }
    
    freeNodeList->listLength = 0;
    freeNodeList->length = 0;
    freeNodeList->fakeNodesLength = 0;

    // Build a chain of lists
    // the last item(list) in the list is the freeNodeList
    listArray[0].nextList = &(listArray[1]);
    listArray[LIST_MAX_NUM_HEADS+1].prevList = &(listArray[LIST_MAX_NUM_HEADS]);
    for(int i = 1; i<LIST_MAX_NUM_HEADS+1; i++){
        listArray[i].prevList = &(listArray[i-1]);
        listArray[i].nextList = &(listArray[i+1]);

        freeNodeList->listLength++;
    }

    // Build a chain of nodes
    nodeArray[0].next = &(nodeArray[1]);
    nodeArray[LIST_MAX_NUM_NODES+1].prev = &(nodeArray[LIST_MAX_NUM_NODES]);
    for(int i = 1; i<LIST_MAX_NUM_NODES+1; i++){
        nodeArray[i].prev = &(nodeArray[i-1]);
        nodeArray[i].next = &(nodeArray[i+1]);

        freeNodeList->length++;
    }

    // build a chain for list to use, all of them are fakeNodes for the lists
    fakeNodeArray[0].next = &(fakeNodeArray[1]);
    fakeNodeArray[LIST_MAX_NUM_HEADS*2+1].prev = &(fakeNodeArray[LIST_MAX_NUM_HEADS*2]);
    for(int i = 1; i<LIST_MAX_NUM_HEADS*2+1; i++){
        fakeNodeArray[i].prev = &(fakeNodeArray[i-1]);
        fakeNodeArray[i].next = &(fakeNodeArray[i+1]);

        freeNodeList->fakeNodesLength++;
        //printf("fakeNodesLength is: %d\n", freeNodeList->fakeNodesLength);
    }

    // Define a fake node Head and a fake node Tail to freeNodeList
    freeNodeList->fakeNodeHead = &(nodeArray[0]);
    freeNodeList->fakeNodeTail = &(nodeArray[LIST_MAX_NUM_NODES+1]);
    
    // Define a fake list Head and a fake list Tail to freeNodeList
    freeNodeList->fakeListHead = &(listArray[0]);
    freeNodeList->fakeListTail = &(listArray[LIST_MAX_NUM_HEADS+1]);

    // Define the head of fake nodes and the tail of fake nodes
    freeNodeList->headOfFakeNodes = &(fakeNodeArray[0]);
    freeNodeList->tailOfFakeNodes = &(fakeNodeArray[LIST_MAX_NUM_HEADS*2+1]);


    // Define currfreeList as the last List in the array
    freeNodeList->currList = freeNodeList->fakeListTail->prevList;
    // Define currfreeNode as the last Node in the array
    freeNodeList->currNode = freeNodeList->fakeNodeTail->prev;
    // Define currFakeNode as the last Node in the fake node array
    freeNodeList->currFakeNode = freeNodeList->tailOfFakeNodes->prev;
    return;
}

// return NULL if no more space for Node
static Node* takeAFreeNode(){
    if(freeNodeList->fakeNodeHead->next!=freeNodeList->fakeNodeTail){
        Node* curr = freeNodeList->currNode;
        freeNodeList->currNode = curr->prev;
        Node* prev = curr->prev;
        Node* next = curr->next;
        prev->next = next;
        next->prev = prev;

        freeNodeList->length--;
        return curr;
    }else{
        return NULL;
    }
}

static Node* recycleANodeFree(Node* node){
    Node* prev = freeNodeList->fakeNodeTail->prev;
    Node* next = freeNodeList->fakeNodeTail;
    prev->next = node;
    node->next = next;
    next->prev = node;
    node->prev = prev;
    freeNodeList->currNode = freeNodeList->fakeNodeTail->prev;

    freeNodeList->length++;
    return node;
}

static Node* takeAFreeFakeNode(){
    Node* curr = freeNodeList->currFakeNode;
    freeNodeList->currFakeNode = curr->prev;
    Node* prev = curr->prev;
    Node* next = curr->next;
    prev->next = next;
    next->prev = prev;
    
    freeNodeList->fakeNodesLength--;
    return curr;
}

// return NULL if no more space for List
static List* takeAFreeList(){
    if(freeNodeList->fakeListHead->nextList!=freeNodeList->fakeListTail){
        // take a free list out of the freeNodeList
        List* curr = freeNodeList->currList;

        curr->prevList->nextList = curr-> nextList;
        curr->nextList->prevList = curr-> prevList;

        freeNodeList->currList = freeNodeList->fakeListTail->prevList;
        freeNodeList->listLength--;

        // initialize currNode and Length for new List. 
        // initialize listHead and listTail
        curr->fakeNodeHead = takeAFreeFakeNode();
        curr->fakeNodeTail = takeAFreeFakeNode();

        curr->fakeNodeHead->next = curr->fakeNodeTail;
        curr->fakeNodeTail->prev = curr->fakeNodeHead;
        curr->currNode = curr->fakeNodeHead;

        curr->length=0;

        return curr;
    }else{
        return NULL;
    }
}

// Makes a new, empty list, and returns its reference on success. 
// Returns a NULL pointer on failure.
List* List_create(){
    getOrCreateFreeNodeList();
    return takeAFreeList();
}

// Returns the number of items in pList.
int List_count(List* pList){
    return pList->length;
}

static bool isListEmpty(List* pList){
    Node* head = pList->fakeNodeHead;
    Node* tail = pList->fakeNodeTail;
    Node* next = head->next;

    if(next == tail){
        return true;
    }
    return false;
}

// Returns a pointer to the first item in pList and makes the first item the current item.
// Returns NULL and sets current item to NULL if list is empty.
void* List_first(List* pList){
    if(isListEmpty(pList)){
        pList->currNode = NULL;
        return NULL;
    }
    pList->currNode = pList->fakeNodeHead->next;
    return pList->fakeNodeHead->next->ptr;
}


// Returns a pointer to the last item in pList and makes the last item the current item.
// Returns NULL and sets current item to NULL if list is empty.
void* List_last(List* pList){
    if(isListEmpty(pList)){
        pList->currNode = NULL;
        return NULL;
    }
    pList->currNode = pList->fakeNodeTail->prev;
    return pList->fakeNodeTail->prev->ptr;
}

// Advances pList's current item by one, and returns a pointer to the new current item.
// If this operation advances the current item beyond the end of the pList, a NULL pointer 
// is returned and the current item is set to be beyond end of pList.
void* List_next(List* pList){
    if(isListEmpty(pList)){
        return NULL;
    }
    if(pList->currNode==pList->fakeNodeTail || pList->currNode->next==pList->fakeNodeTail){
        pList->currNode=pList->fakeNodeTail;
        return NULL;
    }
    pList->currNode = pList->currNode->next;
    return pList->currNode->ptr;
}

// Backs up pList's current item by one, and returns a pointer to the new current item. 
// If this operation backs up the current item beyond the start of the pList, a NULL pointer 
// is returned and the current item is set to be before the start of pList.
void* List_prev(List* pList){
    if(isListEmpty(pList)){
        return NULL;
    }
    if(pList->currNode==pList->fakeNodeHead || pList->currNode->prev==pList->fakeNodeHead){
        pList->currNode=pList->fakeNodeHead;
        return NULL;
    }
    pList->currNode = pList->currNode->prev;
    return pList->currNode->ptr;
}

// Returns a pointer to the current item in pList.
void* List_curr(List* pList){
    if(pList->currNode==NULL){
        return NULL;
    }
    if(pList->currNode==pList->fakeNodeHead || pList->currNode==pList->fakeNodeTail){
        return NULL;
    }else{
        return pList->currNode->ptr;
    }
}


static bool beforeTheStart(List* pList){
    return pList->currNode == pList->fakeNodeHead;
}

static bool beyondTheEnd(List* pList){
    return pList->currNode == pList->fakeNodeTail;
}

static bool usePrependOrAppend(List* pList, Node* newNode){
    if(isListEmpty(pList) || pList->currNode == pList->fakeNodeHead){
        Node* next = pList->fakeNodeHead->next;
        pList->fakeNodeHead->next = newNode;
        newNode->next = next;
        next->prev = newNode;
        newNode->prev = pList->fakeNodeHead;

        pList->currNode = newNode;
        pList->length++;
        return true;
    }else if(pList->currNode == pList->fakeNodeTail){
        Node* prev = pList->fakeNodeTail->prev;
        prev->next = newNode;
        newNode->prev = prev;
        newNode->next = pList->fakeNodeTail;
        pList->fakeNodeTail->prev = newNode;

        pList->currNode = newNode;
        pList->length++;
        return true;
    }else{
        return false;
    }
}


// Adds the new item to pList directly after the current item, and makes item the current item. 
// If the current pointer is before the start of the pList, the item is added at the start. If 
// the current pointer is beyond the end of the pList, the item is added at the end. 
// Returns 0 on success, -1 on failure.
int List_add(List* pList, void* pItem){
    Node* newNode = takeAFreeNode();
    if(newNode==NULL){
        return -1;
    }
    newNode->ptr = pItem;

    if(usePrependOrAppend(pList, newNode)){
        return 0;
    }

    Node* prev = pList->currNode;


    Node* next = pList->currNode->next;

    prev->next = newNode;
    newNode->next = next;
    newNode->prev = prev;
    next->prev = newNode;

    pList->length++;
    pList->currNode = newNode;
    return 0; 
}


// Adds item to pList directly before the current item, and makes the new item the current one. 
// If the current pointer is before the start of the pList, the item is added at the start. 
// If the current pointer is beyond the end of the pList, the item is added at the end. 
// Returns 0 on success, -1 on failure.
int List_insert(List* pList, void* pItem){
    Node* newNode = takeAFreeNode();

    if(newNode==NULL){
        return -1;
    }

    newNode->ptr = pItem;
    if(usePrependOrAppend(pList, newNode)){
        return 0;
    }
    Node* prev = pList->currNode->prev;
    Node* next = pList->currNode;

    prev->next = newNode;
    newNode->next = next;
    newNode->prev = prev;
    next->prev = newNode;

    pList->length++;
    pList->currNode = newNode;
    return 0; 
}


// Adds item to the end of pList, and makes the new item the current one. 
// Returns 0 on success, -1 on failure.
int List_append(List* pList, void* pItem){
    Node* newNode = takeAFreeNode();
    if(newNode==NULL){
        return -1;
    }
    newNode->ptr = pItem;

    Node* prev = pList->fakeNodeTail->prev;
    prev->next = newNode;
    newNode->prev = prev;
    newNode->next = pList->fakeNodeTail;
    pList->fakeNodeTail->prev = newNode;

    pList->currNode = newNode;
    pList->length++;
    return 0;
}


// Adds item to the front of pList, and makes the new item the current one. 
// Returns 0 on success, -1 on failure.
int List_prepend(List* pList, void* pItem){
    Node* newNode = takeAFreeNode();
    if(newNode==NULL){
        return -1;
    }
    newNode->ptr = pItem;


    Node* next = pList->fakeNodeHead->next;
    pList->fakeNodeHead->next = newNode;
    newNode->next = next;
    next->prev = newNode;
    newNode->prev = pList->fakeNodeHead;

    pList->currNode = newNode;
    pList->length++;
    return 0;
}

// Return current item and take it out of pList. Make the next item the current one.
// If the current pointer is before the start of the pList, or beyond the end of the pList,
// then do not change the pList and return NULL.
void* List_remove(List* pList){
    if(isListEmpty(pList)){
        return NULL;
    }
    if(beforeTheStart(pList) || beyondTheEnd(pList)){
        return NULL;
    }
    
    Node* curr = pList->currNode;

    Node* next = curr->next;
    Node* prev = curr->prev;
    prev->next=next;
    next->prev=prev;
    pList->currNode=next;

    pList->length--;
    Node* removed = recycleANodeFree(curr);
    return removed->ptr;
}




static void recycleAFakeNodeFree(List* pList){
    Node* recycledHead = pList->fakeNodeHead;
    Node* recycledTail = pList->fakeNodeTail;
    Node* newHead = freeNodeList->tailOfFakeNodes->prev;


    newHead->next = recycledHead;
    recycledHead->prev = newHead;
    recycledHead->next = recycledTail;
    recycledTail->prev = recycledHead;
    recycledTail->next = freeNodeList->tailOfFakeNodes;
    freeNodeList->tailOfFakeNodes->prev = recycledTail;

    freeNodeList->currFakeNode = freeNodeList->tailOfFakeNodes->prev;

    freeNodeList->fakeNodesLength++;
    freeNodeList->fakeNodesLength++;
    
}

static void recycleAListFree(List* pList){
    List* newHead = freeNodeList->fakeListTail->prevList;
    newHead->nextList = pList;
    pList->prevList = newHead;
    pList->nextList = freeNodeList->fakeListTail;

    freeNodeList->fakeListTail->prevList = pList;
    freeNodeList->currList=freeNodeList->fakeListTail->prevList;

    freeNodeList->listLength++;
}

// Adds pList2 to the end of pList1. The current pointer is set to the current pointer of pList1. 
// pList2 no longer exists after the operation; its head is available
// for future operations.
void List_concat(List* pList1, List* pList2){
    Node* fakeTailOfList1 = pList1->fakeNodeTail;
    Node* fakeHeadOfList2 = pList2->fakeNodeHead;
    Node* fakeTailOfList2 = pList2->fakeNodeTail;

    if(fakeHeadOfList2->next==fakeTailOfList2){
        recycleAFakeNodeFree(pList2);
        recycleAListFree(pList2);
        return;
    }
    pList1->length += pList2->length;

    Node* newHead = fakeTailOfList1->prev;
    newHead->next = fakeHeadOfList2->next;
    fakeHeadOfList2->next->prev = newHead;

    fakeTailOfList2->prev->next = fakeTailOfList1;
    fakeTailOfList1->prev = fakeTailOfList2->prev;
    
    recycleAFakeNodeFree(pList2);
    recycleAListFree(pList2);
}


// Delete pList. pItemFreeFn is a pointer to a routine that frees an item. 
// It should be invoked (within List_free) as: (*pItemFreeFn)(itemToBeFreedFromNode);
// pList and all its nodes no longer exists after the operation; its head and nodes are 
// available for future operations.
//typedef void (*FREE_FN)(void* pItem);
void List_free(List* pList, FREE_FN pItemFreeFn){
    Node* head = pList->fakeNodeHead;
    Node* tail = pList->fakeNodeTail;

    int count = 0;
    while(head->next!= tail){
        count++;
        Node* next = head->next;
        Node* nextnext = head->next->next;
        head->next = nextnext;
        pItemFreeFn(next);
        recycleANodeFree(next);
    }

    recycleAFakeNodeFree(pList);
    recycleAListFree(pList);
}

// Return last item and take it out of pList. Make the new last item the current one.
// Return NULL if pList is initially empty.
void* List_trim(List* pList){
    if(pList->fakeNodeHead->next == pList->fakeNodeTail){
        return NULL;
    }
    pList->currNode=pList->fakeNodeTail->prev;
    void* res = List_remove(pList);
    if(isListEmpty(pList)){
        pList->currNode=NULL;
    }else{
        pList->currNode=pList->fakeNodeTail->prev;
    }
    return res;
}

// Search pList, starting at the current item, until the end is reached or a match is found. 
// In this context, a match is determined by the comparator parameter. This parameter is a
// pointer to a routine that takes as its first argument an item pointer, and as its second 
// argument pComparisonArg. Comparator returns 0 if the item and comparisonArg don't match, 
// or 1 if they do. Exactly what constitutes a match is up to the implementor of comparator. 
// 
// If a match is found, the current pointer is left at the matched item and the pointer to 
// that item is returned. If no match is found, the current pointer is left beyond the end of 
// the list and a NULL pointer is returned.
// 
// If the current pointer is before the start of the pList, then start searching from
// the first node in the list (if any).
// typedef bool (*COMPARATOR_FN)(void* pItem, void* pComparisonArg);
void* List_search(List* pList, COMPARATOR_FN pComparator, void* pComparisonArg){
    Node* curr = NULL;
    if(pList->currNode==pList->fakeNodeHead){
        curr = pList->fakeNodeHead->next;
    }else{
        curr = pList->currNode;
    }

    while(curr!=pList->fakeNodeTail){
        if(pComparator(curr->ptr, pComparisonArg)==1){
            
            pList->currNode=curr;
            return curr->ptr;
        }
        curr=curr->next;
    }
    pList->currNode=pList->fakeNodeTail;
    return NULL;
}

