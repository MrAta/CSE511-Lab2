//
// Created by sps5394 on 11/11/18.
//

#ifndef P2_CSRF_DBTREE_H
#define P2_CSRF_DBTREE_H


#define PAGE_SIZE 4 * 2048
//#define NODE_LIMIT 1000
#define ROOT_FNAME "root.db"
#define MAX_FILENAME_SIZE 128
// need to check over these
#define MAX_KEY_SIZE 16
#define MAX_VALUE_SIZE 64
#define MAX_ENTRY_SIZE 80
#define NODE_LIMIT 38

#include <fcntl.h>
#include <sys/mman.h>
#include "db.h"

//extern const int NODE_LIMIT;

typedef struct {
  char *filename;
} BTREE_FILE_INF;

typedef struct {
  char* key;
  char* val;
  time_t timestamp;
} KEY_VAL;
//
//typedef struct {
//  int fd;
//  int isLeaf;
//  int height;
//} BTREE_METADATA;

extern int store_size;
//typedef struct {
//  KEY_VAL store[(int)PAGE_SIZE - sizeof(BTREE_METADATA) - sizeof(int)];
//  int fd;
//} BTREE_DATA;

extern int file_store_size;
//typedef struct {
//  BTREE_FILE_INF files[(int)PAGE_SIZE - sizeof(BTREE_METADATA)];
//} BTREE_PTR;

typedef struct BTREE_NODE {
  char *filename; // File descriptor to the node
  int isLeaf; // Whether the node is a leaf node
  int n; // Current number of keys
  int allocated;
  KEY_VAL *keys[NODE_LIMIT];
  struct BTREE_NODE *children[NODE_LIMIT];
  int fd;
} BTREE_NODE;

BTREE_NODE *btree_create();

void btree_copy(char *string, char *string1);

int btree_insert_non_full(BTREE_NODE *node, KEY_VAL *key_val);

int btree_disk_write(BTREE_NODE *node);

int btree_insert(BTREE_NODE **root, KEY_VAL *key_val);

int btree_split_child(BTREE_NODE *node, int index);

KEY_VAL * btree_search(BTREE_NODE *node, char * key);

BTREE_NODE *allocate_node(char *file_inf);

int marshall(BTREE_NODE *node, char **buffer);

int unmarshall(BTREE_NODE *node, char *buffer);

void cleanup(BTREE_NODE *root, int full);

/************** KEY VALUE PAIR FUNCTIONS **************/
int marshall_kv(KEY_VAL *key_val, char **buffer);

int unmarshall_kv(KEY_VAL *key_val, char *buf);

/*************** HELPERS ***************/
void rand_string(char *str, size_t size);
#endif //P2_CSRF_DBTREE_H
