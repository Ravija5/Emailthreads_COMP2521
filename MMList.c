// MMList.c ... implementation of List-of-Mail-Messages ADT
// Written by John Shepherd, Feb 2019

#include <assert.h>
#include <err.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>

#include "MMList.h"
#include "MailMessage.h"

// data structures representing MMList

typedef struct MMListNode* Link;

typedef struct MMListNode {
	MailMessage data; // message associated with this list item
	Link next;		  // pointer to node containing next element
} MMListNode;

typedef struct MMListRep {
	Link first; // node containing first value
	Link last;  // node containing last value
	Link curr;  // current node (for iteration)
} MMListRep;

static Link newMMListNode (MailMessage mesg);

//Helper functions for Inserting into MMList
static void MMListInsertBefore(MMList L, Link curr, Link n);
static void MMListInsertAfter(MMList L, Link curr, Link n);

// create a new empty MMList
MMList newMMList (void)
{
	MMListRep *new = malloc (sizeof *new);
	if (new == NULL) err (EX_OSERR, "couldn't allocate MMList");
	*new = (MMListRep) { };
	return new;
}

// free up memory associated with list
// note: does not remove Mail Messages
void dropMMList (MMList L)
{
	assert (L != NULL);
	Link curr = L->first;
	while (curr != NULL) {
		Link next = curr->next;
		free (curr);
		curr = next;
	}
	free (L);
}

// display list as one integer per line on stdout
void showMMList (MMList L)
{
	assert (L != NULL);
	for (Link curr = L->first; curr != NULL; curr = curr->next)
		showMailMessage (curr->data, 0);
}

//the only time MMListInsertBefore is called is inserting before first element in the list
static void MMListInsertBefore(MMList L, Link curr, Link n){
	//inserting before the first element
	n->next = L->first;
	L->first = n;
}

static void MMListInsertAfter(MMList L, Link curr, Link n){
	if(L->curr == L->last){
		//inserting after the last element 
		L->curr->next = n;
		L->last = n;
		n->next = NULL;
	}else{
		//inserting in the middle
		//printf("Inserting in the middle\n");
		n->next = curr->next;
		curr->next = n;
		
	}
}

// insert mail message in order
// ordering based on MailMessageDateTime
void MMListInsert (MMList L, MailMessage mesg)
{
	assert (L != NULL);
	assert (MMListIsOrdered (L));

	assert (mesg != NULL);

	//Create a new node
	Link newLink = newMMListNode(mesg);

	if(L->first == NULL){
		//empty list
		L->first = L->last = newLink;
		newLink->next = NULL;
		return;
	}

	//Getting Timestamps
	DateTime firstDate =  MailMessageDateTime(L->first->data);
	DateTime lastDate =  MailMessageDateTime(L->last->data);
	DateTime newNodeDate = MailMessageDateTime(mesg);

	if(DateTimeBefore(newNodeDate, firstDate)){
		//insert newLink at the beginning
		//printf("Inserting newLink at the beginning\n");
		MMListInsertBefore(L,L->first, newLink);
	}else if(DateTimeAfter(newNodeDate, lastDate)){
		//insert newLink at the end
		//printf("Inserting newLink at the end\n");
		L->curr = L->last;
		MMListInsertAfter(L,L->last, newLink);
	}else{
		//insertion in the middle
		//inserts before the old msg if timestamps are same
		//printf("Inserting newLink in the middle\n");
		L->curr = L->first;
		DateTime currNextDate = MailMessageDateTime(L->curr->next->data);

		while(L->curr != NULL && L->curr->next != NULL && DateTimeAfter(newNodeDate,currNextDate)){
			L->curr = L->curr->next;
			currNextDate = MailMessageDateTime(L->curr->next->data);
		}	
		//insert after curr
	
		MMListInsertAfter(L,L->curr, newLink);
	}
}

// create a new MMListNode for mail message
// (this function is used only within the ADT)
static Link newMMListNode (MailMessage mesg)
{
	Link new = malloc (sizeof *new);
	if (new == NULL) err (EX_OSERR, "couldn't allocate MMList node");
	*new = (MMListNode) { .data = mesg };
	return new;
}

// check whether a list is ordered (by MailMessageDate)
bool MMListIsOrdered (MMList L)
{
	DateTime prevDate = NULL;
	for (Link n = L->first; n != NULL; n = n->next) {
		DateTime currDate = MailMessageDateTime (n->data);
		if (prevDate != NULL && DateTimeAfter (prevDate, currDate))
			return false;
		prevDate = currDate;
	}
	return true;
}

// start scan of an MMList
void MMListStart (MMList L)
{
	assert (L != NULL);
	L->curr = L->first;
}

// get next item during scan of an MMList
MailMessage MMListNext (MMList L)
{
	assert (L != NULL);
	if (L->curr == NULL)
		// this is probably an error
		return NULL;

	MailMessage mesg = L->curr->data;
	L->curr = L->curr->next;
	return mesg;
}

// check whether MMList scan is complete
bool MMListEnd (MMList L)
{
	assert (L != NULL);
	return (L->curr == NULL);
}
