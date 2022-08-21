#ifndef _plist_h_
#define _plist_h_
#include <stdlib.h>
#include <stdio.h>

struct PCB{
	char cmd[256];
	int id;
	char status[256];
	int is_bg;
	int pid;
	int pgid;
	char exec_status[256];
	struct PCB *next;
};
struct PCB *q;

void insert(struct PCB **head,struct  PCB *newProc);
// find lenth of a queue
int length(struct PCB *Q);
// print a whole queue
void printList(struct PCB *Q) ;
struct PCB * find(struct PCB **head,int key);
//delete a link with given key
struct PCB * delete(struct PCB **head,int key);
struct PCB * find_by_id(struct PCB **head,int key);
#endif
