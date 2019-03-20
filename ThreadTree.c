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

//Queue structs
typedef struct QueueRep *Queue;
typedef struct ThreadTreeNode *Link;

typedef struct QueueNode {
	Link value;
	struct QueueNode *next;
} QueueNode;

typedef struct QueueRep {
	QueueNode *head; // ptr to first node
	QueueNode *tail; // ptr to last node
} QueueRep;


// Representation of ThreadTree's



typedef struct ThreadTreeNode {
	MailMessage mesg;
	Link next, replies;
} ThreadTreeNode;

typedef struct ThreadTreeRep {
	Link messages;
} ThreadTreeRep;

// Auxiliary data structures and functions

// Add any new data structures and functions here ...

// create new empty queue
static Queue newQueue (void);
// free memory used by queue
static void dropQueue (Queue);
// add item on queue
static void QueueJoin (Queue, Link);
// remove item from queue
static Link QueueLeave (Queue);
// check for no items
static bool QueueIsEmpty (Queue);


static void doDropThreadTree (Link t);
static void doShowThreadTree (Link t, int level);

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

//function to insert a node at last position free in the List
static void insertAfterNext(Link start, Link newNode){
	while(start->next != NULL){
		start = start->next;
	}
	start->next = newNode;
}


//function to insert a node at last position free in the List
static void insertAfterReplies(Link start, Link newNode){
	//printf("inserting after reply ID Function\n");
	if(start->replies == NULL){
		start->replies = newNode;
	}else{
		insertAfterNext(start->replies, newNode);
	}
}

//function to check if a given reply ID exists in the tree
static int findReplyID(MMTree tree, MailMessage mesg){
	//Traverse through tree and check if any node has the same mesgid as mesg->replyID
	if(MMTreeFind(tree, MailMessageRepliesTo(mesg)) == NULL){
		//false
		return 0;
	}else{
		return 1;
	}

}


static void insertThreadTree(Link curr, Link newNode){
	Queue q = newQueue();

	//Adding root
	QueueJoin(q, curr);

	while(!QueueIsEmpty(q)){
		Link node = QueueLeave(q);
		//printf("Node removed = %s\n ", MailMessageID(node->mesg));
		//printf("NewNode = %s\n", MailMessageID(newNode->mesg));
		//processing node
		if ( (MailMessageRepliesTo(node->mesg) == NULL && MailMessageRepliesTo(newNode->mesg) == NULL) ||
				( MailMessageRepliesTo(node->mesg) != NULL && MailMessageRepliesTo(newNode->mesg) != NULL &&
				strcmp( MailMessageRepliesTo(node->mesg), MailMessageRepliesTo(newNode->mesg)) == 0)  ) {
			//printf("Replies ID's match\n");
			insertAfterNext(node, newNode);
			break;
		}else if((MailMessageID(node->mesg) == NULL && MailMessageRepliesTo(newNode->mesg) == NULL) ||
				( MailMessageID(node->mesg) != NULL && MailMessageRepliesTo(newNode->mesg) != NULL &&
				strcmp( MailMessageID(node->mesg), MailMessageRepliesTo(newNode->mesg) ) == 0) ){
			//printf("MailId of curr and reply id of newNode match\n");
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

//static ThreadTree ThreadTreeInsert(ThreadTree tt, MMList mesgs, MMTree msgids,  )
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
		//printf("Inserted in empty Thread Tree\n");
		tt->messages = newNode;
	}


	Link curr = NULL;
	if(tt->messages != NULL){
		curr = tt->messages;
		//printf("Curr value %s\n",MailMessageID(curr->mesg));
	}
	 
	//Iterating through the MMList
	while((lNode = MMListNext (mesgs)) != NULL){
	
		//printf("LNode value = %s\n", MailMessageID(lNode));

		//create a new node to be inserted
		newNode = newTTNode(lNode);

		//Reset curr 
		curr = tt->messages;
		//printf("Curr value = %s\n", MailMessageID(curr->mesg));
		insertThreadTree(curr, newNode);
		//showThreadTree(tt);
		// if ( strcmp( MailMessageID(newNode->mesg), MailMessageID(curr->mesg) ) == 0 ) {
		// 	printf("Replies ID's match\n");
		// 	insertAfterNext(curr, newNode);
		// 	showThreadTree(tt);
		// }else if(strcmp( MailMessageID(curr->mesg), MailMessageRepliesTo(newNode->mesg) ) == 0){
		// 	printf("MailId of curr and reply id of newNode match\n");
		// 	insertAfterReplies(curr, newNode);
		// 	showThreadTree(tt);
		// }else if( MailMessageID(lNode) == NULL || (findReplyID(msgids,lNode) == 0 ) ){
		// 	//Insert at top level
		// 	printf("Didnt find reply ID at all\n");
		// 	insertAfterNext(curr, newNode);
		// }else{
		// 	printf("Inserting via Queue\n");
		// 	insertThreadTree(curr, newNode);
		// 	showThreadTree(tt);

		// }
	}
	
	return tt; 
}





//Queue Functions


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
