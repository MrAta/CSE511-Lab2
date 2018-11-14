//
// Created by MrAta on 11/13/18.
//
#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif
#ifndef P2_CSRF_C1_H
#define P2_CSRF_C1_H
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "common.h"


#define DB_DIR "./.db"
#define SSTABLE "sstable"
#define MAX_C0_SIZE 100

extern int file_counter;

typedef struct c0_node {
  char *key;
  char *value;
  struct c0_node *left, *right;
  int ht;
  int flag; //0:valid, 1: invlaid
} c0_node;

int c1_batch_insert(c0_node *nodes[], int size);

char *c1_get(char *key);

/************** PRIVATE FUNCTIONS **************/
int merge(FILE *file1, FILE *file2);


// Code from: https://stackoverflow.com/questions/22697407/reading-text-file-into-char-array

#endif
