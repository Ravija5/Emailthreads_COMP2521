// MMTree.c ... implementation of Mail Message search tree ADT
// Each node contains an (ID,message) pair; search is based on ID
// Written by John Shepherd, Feb 2019

#include <assert.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>

#include "MMTree.h"
#include "MailMessage.h"


// data structures representing MMTree
typedef struct MMTNode *Link;

//Helper function for Inserting in MMTree
static Link MMTreeInsertRecursive(Link l,char *id, MailMessage msg);

typedef struct MMTNode {
	char *msgid;		 // unique ID for message
	MailMessage message; // details of message
	Link left, right;	// sub-trees
} MMTNode;

typedef struct MMTreeRep {
	Link root;
} MMTreeRep;

static Link newMMTNode (char *id, MailMessage message);
static void doDropMMTree (Link t);
static void doShowMMTree (Link t, int level);
static MailMessage doMMTreeFind (Link t, char *id);

// create a new empty MMTree
MMTree newMMTree (void)
{
	MMTreeRep *new = malloc (sizeof *new);
	if (new == NULL) err (EX_OSERR, "couldn't allocate MMTree");
	*new = (MMTreeRep) { };
	return new;
}

void dropMMTree (MMTree mmt)
{
	assert (mmt != NULL);
	doDropMMTree (mmt->root);
	free (mmt);
}

// free memory associated with MMTree
// note: does not free ID or MailMessage
static void doDropMMTree (Link t)
{
	if (t == NULL) return;
	doDropMMTree (t->left);
	doDropMMTree (t->right);
	free (t);
}

void showMMTree (MMTree mmt)
{
	assert (mmt != NULL);
	doShowMMTree (mmt->root, 0);
}

// display a MMTree (sideways)
static void doShowMMTree (Link t, int level)
{
	if (t == NULL) return;
	doShowMMTree (t->right, level + 1);
	for (int i = 0; i < level; i++)
		printf ("   ");
	printf ("%s\n", t->msgid);
	doShowMMTree (t->left, level + 1);
}

MailMessage MMTreeFind (MMTree mmt, char *id)
{
	assert (mmt != NULL);
	return doMMTreeFind (mmt->root, id);
}

// check whether a message with ID is in a MMTree
static MailMessage doMMTreeFind (Link t, char *id)
{
	if (t == NULL)
		return NULL;
	else if (strcmp (id, t->msgid) <  0)
		return doMMTreeFind (t->left, id);
	else if (strcmp (id, t->msgid)  > 0)
		return doMMTreeFind (t->right, id);
	else // (strcmp (id, t->msgid) == 0)
		return t->message;
}

//Helper function for MMTreeInsert
static Link MMTreeInsertRecursive(Link l, char* id, MailMessage mesg){
	if(l == NULL){
		//create new node
		Link newNode = newMMTNode(id,mesg); 
		return newNode;
	}
	if(strcmp(l->msgid, id) < 0){
		l->right = MMTreeInsertRecursive(l->right,id,mesg);
	}else{
		l->left = MMTreeInsertRecursive(l->left,id, mesg);
	}
	return l;
}

// insert a new message into a MMTree
// message is indexed by a string ID
MMTree MMTreeInsert (MMTree t, char *id, MailMessage mesg)
{
	if(t->root == NULL){
		//tree is empty
		//Create node
		Link newNode = newMMTNode(id,mesg);
		t->root = newNode;
		return t;
	}
	
	if( strcmp(t->root->msgid, id) < 0 ){
		//id is bigger than curr link
		t->root->right = MMTreeInsertRecursive(t->root->right,id, mesg);
	}else{
		t->root->left = MMTreeInsertRecursive(t->root->left,id, mesg);
	}

	return t;
}

// make a new node containing a value
// (this function is used only within the ADT)
static Link newMMTNode (char *id, MailMessage message)
{
	Link new = malloc (sizeof (MMTNode));
	assert (new != NULL);
	new->msgid = id;
	new->message = message;
	new->left = new->right = NULL;
	return new;
}
