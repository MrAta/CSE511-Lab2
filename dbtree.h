//
// Created by sps5394 on 11/11/18.
//

#ifndef P2_CSRF_DBTREE_H
#define P2_CSRF_DBTREE_H


#define PAGE_SIZE 4 * 2048
//#define NODE_LIMIT 1000
#define ROOT_FNAME "root"
#define DB_DIR "./.db"
#define MAX_FILENAME_SIZE 128
// need to check over these
#define MAX_KEY_SIZE 16
#define MAX_VALUE_SIZE 64
#define MAX_ENTRY_SIZE 80
#define NODE_LIMIT 35

#include <fcntl.h>
#include <sys/mman.h>
#include "db.h"

//extern const int NODE_LIMIT;
//
//typedef struct {
//  char *filename;
//} BTREE_FILE_INF;

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
  int num_children; // Current number of children
  int allocated;
  KEY_VAL *keys[NODE_LIMIT];
  struct BTREE_NODE *children[NODE_LIMIT];
  int fd;
} BTREE_NODE;

/**************** API *************************/
/**
 * This function instantiates a fresh btree on disk.
 * If the db exists, it will allocate based on the values there.
 * @return Allocated btree_node
 */
BTREE_NODE *btree_create();

/**
 * Main insert function. Inserts the key_value pair to
 * the C0 BTree. This function may change the root node of the
 * BTree.
 * @param root Address of root BTree node
 * @param key_val Key Value pair to insert
 * @return 0 if successful, -1 if failure
 */
int btree_insert(BTREE_NODE **root, KEY_VAL *key_val);

/**
 * This function searches through the BTree for a particular
 * key. This function runs in O(logn) time.
 * @param node The node to search for the key
 * @param key The key to search for
 * @return KEY_VAL struct allocated with the key value pair
 */
KEY_VAL * btree_search(BTREE_NODE *node, char * key);

/************ PRIVATE FUNCTIONS **************/
/**
 * @deprecated DEPRECATED
 * @param string
 * @param string1
 */
void btree_copy(char *string, char *string1);

/**
 * Insert a key value pair into a non-full node.
 * This may or may not flush the value out to disk
 * depending on whether the nodes get full.
 * This function is mostly used as a helper, it should
 * not be used as an API function.
 * @param node The node to insert the key to
 * @param key_val The key_value pair to insert
 * @return 0 if successful, -1 if failure
 */
int btree_insert_non_full(BTREE_NODE *node, KEY_VAL *key_val);

/**
 * Flushes the node out to disk with marshalling as needed.
 * @param node The node to flush data to disk
 * @return 0 if successful, -1 if failure
 */
int btree_disk_write(BTREE_NODE *node);

/**
 * Splits a full non-leaf child into smaller nodes as needed by
 * the BTree algorithm. This is the only function that causes
 * the tree to grow
 * @param node The node to split into two.
 * @param index The index at which to split the child node
 * @return 0 if successful, -1 if failure
 */
int btree_split_child(BTREE_NODE *node, int index);

/**
 * Allocates a new node based on the passed file name
 * This function will perform a full disk read of the node
 * and read the data into memory.
 * @param file_inf Name of the db file to allocate as a node.
 * Allocates a brand new file if NULL.
 * @return BTREE_NODE allocated BTree node.
 */
BTREE_NODE *allocate_node(char *file_inf);

/**
 * Allocates and packs the provided data from the node
 * into a buffer that can be written out to file.
 * @param node Node to read data from
 * @param buffer address of buffer to write data to. This function
 * allocates the buffer based on the size needed
 * @return -1 if failure, number of bytes written if success
 */
int marshall(BTREE_NODE *node, char **buffer);

/**
 * Unmarshalls a buffer of data into the Btree Node.
 * node must be allocated, but any internal properties will be
 * allocated by this function.
 * @param node The node to unpack data to
 * @param buffer The buffer containing unmarshallable data.
 * @return -1 if failure; number of bytes transferred if successful
 */
int unmarshall(BTREE_NODE *node, char *buffer);

/**
 * NOTE: WIP.
 * This function handles de-allocating memory and file closure for the provided
 * node and its children.
 * @param root The node to cleanup
 * @param full if 1, this function will also remove the files from the
 * file system. if 0, it will keep the files on disk.
 */
void cleanup(BTREE_NODE *root, int full);

/************** KEY VALUE PAIR FUNCTIONS **************/
/**
 * Allocates and packs the provided data from the key value pair
 * into a buffer that can be written out to file.
 * @param key_val Key value pair to read data from
 * @param buffer address of buffer to write data to. This function
 * allocates the buffer based on the size needed
 * @return -1 if failure, number of bytes written if success
 */
int marshall_kv(KEY_VAL *key_val, char **buffer);

/**
 * Unmarshalls a buffer of data into the key value pair
 * key_val must be allocated, but any internal properties will be
 * allocated by this function.
 * @param key_val The key value pair to unpack data to
 * @param buffer The buffer containing unmarshallable data.
 * @return -1 if failure; number of bytes transferred if successful
 */
int unmarshall_kv(KEY_VAL *key_val, char *buf);

/*************** HELPERS ***************/
void rand_string(char *str, size_t size);
#endif //P2_CSRF_DBTREE_H
