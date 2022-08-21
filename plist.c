#include "plist.h"


/************************************** 
* insert new process to process list
* 
*/
void insert(struct PCB **head,struct  PCB *newProc) {
	// add the new task to the list	
	struct PCB *temp=*head;	
	// if list is emtpy insert at head
	if(*head==NULL){
		
		*head=newProc;
	}
	else{	// if not emtpy list .. look for last emtpy space
		while(temp->next!=NULL){
			//printf("la\n");
			temp=temp->next;		
		}
		// insert at found location
		temp->next=newProc;  
	}
}
/************************************** 
* find lenth of a queue
* 
*/ 
int length(struct PCB *Q) {
   int length = 0;
   struct PCB *current;
   // loop until reached to last of the list
   for(current = Q; current != NULL; current = current->next) {
      length++;
   }	
   return length;
}

/************************************** 
* print a whole queue just used for debuging
*/
void printList(struct PCB *Q) {
   struct PCB *ptr = Q;
   printf("\n[ ");	
   //start from the beginning
   while(ptr != NULL) {
	// print the node
	printf("(%s,%d) ",ptr->cmd,ptr->id);
	// move to next node
	ptr = ptr->next;
   }	
   printf(" ]\n");
}

/******************************
* find a process by pid
*/
struct PCB * find(struct PCB **head,int key) {

   	//start from the first link
	struct PCB *current=*head;
	
   	//if list is empty
   	if(*head == NULL) {
      		return NULL;
   	}
   	//navigate through list
   	while(current->pid != key) {
      		//if it is last node
      		if(current->next == NULL) {
        		return NULL;
      		} 
		else {
         		//store reference to current lin
         		//move to next link
         		current = current->next;
      		}
   	}
	return current;	
}
// find a process by its  id
struct PCB * find_by_id(struct PCB **head,int key) {

   	//start from the first link
	struct PCB *current=*head;
	
   	//if list is empty
   	if(*head == NULL) {
      		return NULL;
   	}
   	//navigate through list
   	while(current->id != key) {
      		//if it is last node
      		if(current->next == NULL) {
        		return NULL;
      		} 
		else {
         		//store reference to current lin
         		//move to next link
         		current = current->next;
      		}
   	}
	return current;	
}

//delete a link with given key
struct PCB * delete(struct PCB **head,int key) {

   	//start from the first link
	struct PCB *current=*head;
	
   	struct PCB* previous = NULL;
   	//if list is empty
   	if(head == NULL) {
      		return NULL;
   	}
   	//navigate through list
   	while(current->pid != key) {
      		//if it is last node
      		if(current->next == NULL) {
        		return NULL;
      		} 
		else {
         		//store reference to current link
         		previous = current;
         		//move to next link
         		current = current->next;
      		}
   	}

   	//found a match, update the link
   	if(current == *head) {
      		//change first to point to next link
      		*head = (*head)->next;
		
	} 
	else {
      		//bypass the current link
      		previous->next = current->next;
   	}
	return current;	
}

