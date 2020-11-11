#include "list.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#define VALIDATE(TRUE) \
    if (!(TRUE)) { \
        printf("Failed at line: %d \n", __LINE__); \
        exit(1);\
    }

static bool compare_ref(void* a_ptr, void* b_ptr){
    if(b_ptr == a_ptr){
        return 1;
    }
    return 0;
}

static int counter = 0;
static void pItemFreeFn(void* node){
    counter++;
}


int main(){
    // create new lists;
    // no more lists created if the number of lists reached LIST_MAX_NUM_HEADS
    List* list1 = List_create();
    VALIDATE(List_count(list1) == 0);

    List* list2 = List_create();
    List* list3 = List_create();
    List* list4 = List_create();
    List* list5 = List_create();
    List* list6 = List_create();
    List* list7 = List_create();
    List* list8 = List_create();
    List* list9 = List_create();
    List* list10 = List_create();
    List* list11 = List_create();
    List* list12 = List_create();

    // new created lists have length of 0
    VALIDATE(list2!=NULL);
    VALIDATE(list3!=NULL);
    VALIDATE(list4!=NULL);
    VALIDATE(list9!=NULL);
    
    VALIDATE(List_count(list5) == 0);

    // can create as many lists as LIST_MAX_NUM_HEADS
    VALIDATE(list10!=NULL);
    VALIDATE(list11==NULL);
    VALIDATE(list12==NULL);


    int one = 1;
    int two = 2;
    int three = 3;
    int four = 4;
    int five = 5;

    // regular manipulations
    VALIDATE(List_add(list1, &one)==0);
    VALIDATE(List_add(list1, &two)==0);
    VALIDATE(List_first(list1)==&one);
    VALIDATE(List_last(list1)==&two);
    
    // list1 : 1, 2
    VALIDATE(List_count(list1)==2);

    VALIDATE(List_add(list1, &three)==0);

    VALIDATE(List_prev(list1)==&two); 
    VALIDATE(List_prev(list1)==&one);
    VALIDATE(List_prev(list1)==NULL);
    

    VALIDATE(List_add(list1, &four)==0);

    VALIDATE(List_count(list1) == 4);
    VALIDATE(List_first(list1)== &four);
    VALIDATE(List_last(list1)== &three);
    
    VALIDATE(List_next(list1)==NULL);
    // add to the last of over the last
    VALIDATE(List_add(list1, &one)==0);
    VALIDATE(List_last(list1)== &one);
    

    VALIDATE(List_next(list1)==NULL);
    VALIDATE(List_insert(list1, &two)==0);
    VALIDATE(List_last(list1)== &two);

    VALIDATE(List_first(list1)== &four);
    VALIDATE(List_prev(list1)==NULL);
    // insert to first if before the head
    VALIDATE(List_insert(list1, &one)==0);
    VALIDATE(List_first(list1)== &one);

    // List1 : 1, 4, 1, 2, 3, 1, 2
    VALIDATE(List_count(list1)==7);

    VALIDATE(List_append(list1, &five)==0);
    VALIDATE(List_prev(list1)==&two);

    VALIDATE(List_prepend(list1, &five)==0);
    VALIDATE(List_prev(list1)==NULL);

    //printWholeList(list1);
    // List1 : 5, 1, 4, 1, 2, 3, 1, 2, 5
    VALIDATE(List_remove(list1)==NULL);
    VALIDATE(List_count(list1)==9);

    VALIDATE(List_next(list1)==&five);
    VALIDATE(List_next(list1)==&one);
    
    VALIDATE(List_remove(list1)==&one);

    VALIDATE(List_count(list1)==8);

    VALIDATE(List_trim(list1)==&five);

    // trim an empty list
    VALIDATE(List_trim(list2)==NULL);


    // CURRENT = 2
    // 5, 4, 1, 2, 3, 1, 2,
    VALIDATE(List_prev(list1)==&one);
    VALIDATE(List_search(list1,compare_ref,&three)==NULL);

    VALIDATE(List_prev(list1)==&two);
    VALIDATE(List_prev(list1)==&one);
    VALIDATE(List_prev(list1)==&three);
    
    // search from the current
    VALIDATE(List_search(list1,compare_ref,&three)==&three);

    VALIDATE(List_first(list1)==&five);
    VALIDATE(List_prev(list1)==NULL);
    VALIDATE(List_search(list1,compare_ref,&four)==&four);
    VALIDATE(List_remove(list1)==&four);
    
    int length = List_count(list1);
    List_free(list1,pItemFreeFn);
    // all nodes are killed
    VALIDATE(counter==length);

    list11 = List_create();
    list12 = List_create();

    // after list killed, a new list can be created
    VALIDATE(list11!=NULL);
    VALIDATE(list12==NULL);

    VALIDATE(List_add(list11, &one)==0);
    VALIDATE(List_add(list11, &two)==0);

    VALIDATE(List_add(list5, &five)==0);
    VALIDATE(List_add(list5, &four)==0);

    List_concat(list11,list5);

    VALIDATE(List_count(list11)==4);

    list12 = List_create();

    VALIDATE(list12!=NULL);

    List_concat(list11,list12);
    VALIDATE(List_count(list11)==4);
    
    List_concat(list2,list11);
    VALIDATE(List_count(list2)==4);
    

    for(int i = 0; i<96; i++){
        if(i%3==0){
            VALIDATE(List_append(list6,&three)==0);
        }else if(i%3==1){
            VALIDATE(List_append(list6,&two)==0);
        }else{
            VALIDATE(List_append(list6,&one)==0);
        }
    }

    // node is full and cannot add more nodes
    VALIDATE(List_append(list7,&three)==-1);


    VALIDATE(List_first(list6)==&three);
    VALIDATE(List_next(list6)==&two);
    VALIDATE(List_last(list6)==&one);
    VALIDATE(List_prev(list6)==&two);

    // after two remove, the current is beyond the last and cannot be removed
    for(int i = 0;i<5; i++){
        if(i == 0){
            VALIDATE(List_remove(list6)==&two);
        }else if(i==1){
            VALIDATE(List_remove(list6)==&one);
        }else{
            VALIDATE(List_remove(list6)==NULL);
        }
        
        List_append(list8,&three);
    }

    VALIDATE(List_count(list8)==2);
    VALIDATE(List_append(list8,&three)==-1);

    printf("THE END, ALL PASSED\n");
}