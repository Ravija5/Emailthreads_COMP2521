// ThreadTree.c ... implementation of Tree-of-Mail-Threads ADT
// Written by John Shepherd, Feb 2019


#include <assert.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <stdbool.h>

#include "MMList.h"
#include "MMTree.h"
#include "MailMessage.h"
#include "ThreadTree.h"

typedef struct QueueRep *Queue;
typedef struct ThreadTreeNode *Link;

//Queue Structs 
typedef struct QueueNode {
	Link value;
	struct QueueNode *next;
} QueueNode;

typedef struct QueueRep {
	QueueNode *head; // ptr to first node
	QueueNode *tail; // ptr to last node
} QueueRep;


// ThreadTree Structs
typedef struct ThreadTreeNode {
	MailMessage mesg;
	Link next, replies;
} ThreadTreeNode;

typedef struct ThreadTreeRep {
	Link messages;
} ThreadTreeRep;

// Auxiliary data structures and functions
// Add any new data structures and functions here ...

//Queue prototypes
static Queue newQueue (void);
static void dropQueue (Queue);
static void QueueJoin (Queue, Link);
static Link QueueLeave (Queue);
static bool QueueIsEmpty (Queue);

//ThreadTree prototypes
static void doDropThreadTree (Link t);
static void doShowThreadTree (Link t, int level);
static Link newTTNode(MailMessage message);
static void insertThreadTree(Link root, Link newNode);
static void insertAfterReplies(Link start, Link newNode);
static void insertAfterNext(Link start, Link newNode);
static bool isReplyIDNull(Link link);
static bool isMessageIDNull(Link link);
static bool isMessageAndReplyIDEqual(Link n1, Link n2);


// END auxiliary data structures and functions


// create a new empty ThreadTree
ThreadTree newThreadTree (void)
{
	ThreadTreeRep *new = malloc (sizeof *new);
	if (new == NULL) err (EX_OSERR, "couldn't allocate ThreadTree");
	*new = (ThreadTreeRep) { };
	return new;
}

void dropThreadTree (ThreadTree tt)
{
	assert (tt != NULL);
	doDropThreadTree (tt->messages);
}

// free up memory associated with list
static void doDropThreadTree (Link t)
{
	if (t == NULL)
		return;

	for (Link curr = t, next; curr != NULL; curr = next) {
		next = curr->next;
		doDropThreadTree (curr->replies);
		// don't drop curr->mesg, in case referenced elsehwere
		free (curr);
	}
}

void showThreadTree (ThreadTree tt)
{
	assert (tt != NULL);
	doShowThreadTree (tt->messages, 0);
}

// display thread tree as hiearchical list
static void doShowThreadTree (Link t, int level)
{
	if (t == NULL)
		return;
	for (Link curr = t; curr != NULL; curr = curr->next) {
		showMailMessage (curr->mesg, level);
		doShowThreadTree (curr->replies, level + 1);
	}
}

//create a new Thread Tree Node
static Link newTTNode(MailMessage message){
	Link new = malloc (sizeof (ThreadTreeNode));
	assert (new != NULL);
	new->mesg = message;
	new->next = new->replies = NULL;
	return new;
}


/*
Function to add a Link to the end of a list
Link start: root of the list
Link newNode: node to be inserted in list
*/
static void insertAfterNext(Link start, Link newNode){
	
	while(start->next != NULL){
		start = start->next;
	}
	start->next = newNode;
}


/*
Function to add a Link to the end of it's Replies list
Link start: root of the list
Link newNode: node to be inserted in list
*/
static void insertAfterReplies(Link start, Link newNode){
	if(start->replies == NULL){
		start->replies = newNode;
	}else{
		insertAfterNext(start->replies, newNode);
	}
}

//Function to check if a given Link has a replyID == NULL
static bool isReplyIDNull(Link link){
	if ( MailMessageRepliesTo(link->mesg) == NULL){
		return true;
	}else{
		return false;
	}
}

//Function to check if a given Link has a messageID == NULL
static bool isMessageIDNull(Link link){
	if ( MailMessageID(link->mesg) == NULL){
		return true;
	}else{
		return false;
	}
}

//Function to check if two given Links have the same reply ID
static bool isReplyIDEqual(Link n1, Link n2){
	//NOTE: strcmp does not take null as a valid argument
	if ( !isReplyIDNull(n1) && !isReplyIDNull(n2) &&
			strcmp( MailMessageRepliesTo(n1->mesg), MailMessageRepliesTo(n2->mesg)) == 0 ){
		return true;
	}else{
		return false;
	}
}

//Function to compare the messageID of Link n1 with replyID of Link n2
static bool isMessageAndReplyIDEqual(Link n1, Link n2){
	//NOTE: strcmp does not take null as a valid argument
	if ( !isMessageIDNull(n1) && !isReplyIDNull(n2) &&
			strcmp( MailMessageID(n1->mesg), MailMessageRepliesTo(n2->mesg)) == 0 ){
		return true;
	}else{
		return false;
	}
}

/*
Function to insert a given Link to a ThreadTree
Link root: the root of the ThreadTree
Link newNode: the node to be inserted into ThreadTree
*/
static void insertThreadTree(Link root, Link newNode){
	Queue q = newQueue();

	QueueJoin(q, root);

	while( !QueueIsEmpty(q) ){

		Link node = QueueLeave(q);
		
		//Processing node 
		if ( (isReplyIDNull(node) && isReplyIDNull(newNode)) || isReplyIDEqual(node, newNode) ) {
			//Replies ID's of node and newNode match
			insertAfterNext(node, newNode);
			break;
		}else if( (isMessageIDNull(node) && isMessageIDNull(newNode)) || isMessageAndReplyIDEqual(node, newNode) ){
			//MailID of node and replyID of newNode match
			insertAfterReplies(node, newNode);
			break;
		}
		
		if(node->replies != NULL){
			QueueJoin(q, node->replies);
		}
		if(node->next != NULL){
			QueueJoin(q, node->next);
		}
	}
	dropQueue(q);
}

// insert mail message into ThreadTree
// if a reply, insert in appropriate replies list
// whichever list inserted, must be in timestamp-order
ThreadTree ThreadTreeBuild (MMList mesgs, MMTree msgids)
{
	//Create thread tree
	ThreadTreeRep* tt = newThreadTree();
	tt->messages = NULL;

	MMListStart(mesgs);
	MailMessage lNode = MMListNext(mesgs);

	//Create Thread Tree Node to be inserted
	Link newNode = newTTNode(lNode);

	if(tt->messages == NULL){
		//Thread tree is empty
		tt->messages = newNode;
	}

	//Setting curr as the root of ThreadTree
	Link curr = NULL;
	if(tt->messages != NULL){
		curr = tt->messages;
	}
	 
	//Iterating through the MMList
	while((lNode = MMListNext (mesgs)) != NULL){
		
		//create a new threadtree node to be inserted
		newNode = newTTNode(lNode);

		//Reset curr after each iteration
		curr = tt->messages;
		insertThreadTree(curr, newNode);
	}
	return tt; 
}



// Implementations of Queue Functions

// create new empty Queue
static Queue newQueue (void)
{
	Queue new = malloc (sizeof *new);
	if (new == NULL) err (EX_OSERR, "couldn't allocate Queue");
	*new = (QueueRep) { .head = NULL, .tail = NULL };
	return new;
}

// free memory used by Queue
static void dropQueue (Queue Q)
{
	assert (Q != NULL);

	// free list nodes
	QueueNode *curr = Q->head;
	while (curr != NULL) {
		QueueNode *next = curr->next;
		free (curr);
		curr = next;
	}
	// free queue rep
	free (Q);
}

// add item at end of Queue
static void QueueJoin (Queue Q, Link it)
{
	assert (Q != NULL);

	QueueNode *new = malloc (sizeof *new);
	if (new == NULL) err (EX_OSERR, "couldn't allocate Queue node");
	*new = (QueueNode) { .value = it, .next = NULL };

	if (Q->head == NULL)
		Q->head = new;
	if (Q->tail != NULL)
		Q->tail->next = new;
	Q->tail = new;
}

// remove item from front of Queue
static Link QueueLeave (Queue Q)
{
	assert (Q != NULL);
	assert (Q->head != NULL);
	Link it = Q->head->value;
	QueueNode *old = Q->head;
	Q->head = old->next;
	if (Q->head == NULL)
		Q->tail = NULL;
	free (old);
	return it;
}

// check for no items
static bool QueueIsEmpty (Queue Q)
{
	return Q->head == NULL;
}
