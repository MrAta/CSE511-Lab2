/**
 * @file c0.h
 * 
 * Declarations for c0 related things.
 */

/* Includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "c1.h"

/* Functions */
c0_node *Insert(c0_node * T,char *key, char *value, int flag);
c0_node *Delete(c0_node *T,char *key);
c0_node * Get(c0_node *T, char *key);
c0_node * Update(c0_node * T, char * key, char * value, int flag);
void preorder(c0_node *T);
void inorder(c0_node *T);
void dumpToFile(c0_node * T, FILE * f);
int dumpToArray(c0_node * T, c0_node *nodes[], int i);
void c0_dump(c0_node * T);
int c0_size(c0_node* T);
int height( c0_node *T);
c0_node *rotateright(c0_node *T);
c0_node *rotateleft(c0_node *T);
c0_node *RR(c0_node *T);
c0_node *LL(c0_node *T);
c0_node *LR(c0_node *T);
c0_node *RL(c0_node *T);
int BF(c0_node *T);
void c0_init();
