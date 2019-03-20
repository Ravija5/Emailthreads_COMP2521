// ThreadTree.c ... implementation of Tree-of-Mail-Threads ADT
// Written by John Shepherd, Feb 2019


#include <assert.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>

#include "MMList.h"
#include "MMTree.h"
#include "MailMessage.h"
#include "ThreadTree.h"


// Representation of ThreadTree's

typedef struct ThreadTreeNode *Link;

typedef struct ThreadTreeNode {
	MailMessage mesg;
	Link next, replies;
} ThreadTreeNode;

typedef struct ThreadTreeRep {
	Link messages;
} ThreadTreeRep;

// Auxiliary data structures and functions

// Add any new data structures and functions here ...

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
	printf("inserting after reply ID Function\n");
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
		printf("Inserted in empty Thread Tree\n");
		tt->messages = newNode;
	}


	Link curr = NULL;
	if(tt->messages != NULL){
		curr = tt->messages;
		//printf("Curr value %s\n",MailMessageID(curr->mesg));
	}
	 
	//Iterating through the MMList
	while((lNode = MMListNext (mesgs)) != NULL){
	
		printf("LNode value = %s\n", MailMessageID(lNode));

		//create a new node to be inserted
		newNode = newTTNode(lNode);

		//Reset curr 
		curr = tt->messages;
		printf("Curr value = %s\n", MailMessageID(curr->mesg));

		if ( strcmp( MailMessageID(newNode->mesg), MailMessageID(curr->mesg) ) == 0 ) {
			printf("Replies ID's match\n");
			insertAfterNext(curr, newNode);
		}else if(strcmp( MailMessageID(curr->mesg), MailMessageRepliesTo(newNode->mesg) ) == 0){
			printf("MailId of curr and reply id of newNode match\n");
			insertAfterReplies(curr, newNode);
			showThreadTree(tt);
		}else if( MailMessageID(lNode) == NULL || (findReplyID(msgids,lNode) == 0 ) ){
			//Insert at top level
			printf("Didnt find reply ID at all\n");
			insertAfterNext(curr, newNode);
		}else{
			printf("TBD\n");
			//Traverse the thread tree
			//For each link, check if the curr->msgid == newNode->replyID
			//If yes, insert after replies

		}
	}
	
	return tt; // change this line
}


