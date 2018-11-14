//
// Created by sps5394 on 11/11/18.
//

#include <assert.h>
#include "dbtree.h"
#include "lsm.h"

#define BT_FILENAME_SIZE 20

//const int NODE_LIMIT =
//  ( PAGE_SIZE - sizeof(BTREE_NODE)) / ( MAX_ENTRY_SIZE + sizeof(int) + MAX_FILENAME_SIZE );
//int store_size  = (int)PAGE_SIZE - sizeof(BTREE_METADATA) - sizeof(int);
//int file_store_size  = (int)PAGE_SIZE - sizeof(BTREE_METADATA);

BTREE_NODE *btree_create() {
  //
  //
  // LOCAL VARIABLES
  //
  //
  BTREE_NODE *root;
  int first_run = 1;
  if (chdir(DB_DIR) == -1) {
    perror("Could not open db dir: ");
    exit(1);
  }
  if (access(ROOT_FNAME, F_OK) != -1) {
    first_run = 0;
  }
  root = allocate_node(ROOT_FNAME);
  if (first_run) {
    root->isLeaf = 1;
    root->n = 0;
    btree_disk_write(root);
  }
  return root;
}

// TODO: Change to use KEY_VAL in order to keep everything together
void btree_copy(char *string, char *string1) {

}

int btree_insert_non_full(BTREE_NODE *node, KEY_VAL *key_val) {
  //
  //
  // LOCAL VARIABLES
  //
  //
  int i;

  i = node->n - 1;
  if (node->isLeaf) {
    while (i >= 0 && lsm_key_cmp(key_val->key, node->keys[i]->key) < 0) {
      node->keys[i + 1] = node->keys[i];
      i--;
    }
//    if (node->keys == NULL) {
//      node->keys = malloc(sizeof(KEY_VAL *) * NODE_LIMIT); // TODO: Figure out actual size
//    }
    node->keys[i + 1] = key_val;
    node->n += 1;
    // TODO: Defer disk_write?
    return btree_disk_write(node);
  } else {
    while (i >= 0 && lsm_key_cmp(key_val->key, node->keys[i]->key) < 0) {
      i--;
    }
    i += 1;
    // TODO: Fix possible mem leak here
//    if (node->children == NULL) {
//      node->children = malloc(sizeof(BTREE_NODE *) * NODE_LIMIT);
//      node->children[0] = calloc(1, sizeof(BTREE_NODE));
//    }
    if (!node->children[i]->allocated) {
      node->children[i] = allocate_node(
        node->children[i]->filename);// TODO: Fix memory leak here. Add cleanup call
    }
    if (node->children[i]->n == NODE_LIMIT) {
      btree_split_child(node, i);
      if (lsm_key_cmp(key_val->key, node->keys[i]->key) > 0) {
        i++;
      }
    }
    BTREE_NODE *next_child = node->children[i];
    if (!next_child->allocated) {
      next_child = allocate_node(next_child->filename);
    }
    // TODO: Debug cleanup
//    cleanup(node->children[i], 0);
    node->children[i] = next_child;
    return btree_insert_non_full(node->children[i], key_val);
  }
}

int btree_disk_write(BTREE_NODE *node) {
  //
  //
  // LOCAL VARIABLES
  //
  //
  char *mbuf;
  int size;

  ftruncate(node->fd, 0);
  lseek(node->fd, 0, SEEK_SET);
  size = marshall(node, &mbuf);
  if (write(node->fd, mbuf, size) < 0) {
    perror("Write DB node failed: ");
    exit(1);
  }
  free(mbuf);
  node->allocated = 0;
  close(node->fd);
//  if (msync(node, PAGE_SIZE, MS_SYNC) == -1) {
//    perror("Msync failed: ");
//    exit(1);
//  }
//  if (munmap(node, PAGE_SIZE)) {
//    perror("Munmap failed: ");
//    exit(1);
//  }
//  node->allocated = 0;
  return 0;
}

int btree_insert(BTREE_NODE **root, KEY_VAL *key_val) {
  BTREE_NODE *oldroot = *root;
  if (!oldroot->allocated) {
    oldroot = allocate_node(oldroot->filename);
//    cleanup(*root, 0);
    *root = oldroot;
  }
  if (( *root )->n == NODE_LIMIT) {
    BTREE_NODE *newnode = allocate_node(NULL);
    char *tmp = "tmp";
    char *newname = newnode->filename;
    rename(( *root )->filename, tmp);
    ( *root )->filename = tmp;
    rename(newnode->filename, ROOT_FNAME);
    newnode->filename = ROOT_FNAME;
    rename(( *root )->filename, newname);
    ( *root )->filename = newname;
    *root = newnode;
    ( *root )->n = 0;
    ( *root )->children[0] = oldroot;
    ( *root )->num_children = 1;
    ( *root )->isLeaf = 0;
    btree_split_child(newnode, 0);
    return btree_insert_non_full(newnode, key_val);
  } else {
    return btree_insert_non_full(*root, key_val);
  }
}

int btree_split_child(BTREE_NODE *node, int index) {
  //
  //
  // LOCAL VARIABLES
  //
  //
  BTREE_NODE *z = allocate_node(NULL);
  BTREE_NODE *next_node;
  if (!node->children[index]->allocated) {
    next_node = allocate_node(node->children[index]->filename);
  } else {
    next_node = node->children[index];
  }
  z->isLeaf = next_node->isLeaf;
  int median = (( NODE_LIMIT + 1 ) / 2 ) - 1;
  z->n = median;
  for (int j = 0; j < median; j++) {
    z->keys[j] = next_node->keys[j + median];
  }
  if (!next_node->isLeaf) {
    for (int j = 0; j < median + 1; j++) {
      z->children[j] = next_node->children[j + median + 1];
    }
  }
  next_node->n = median;
  for (int j = node->n; j >= index; j--) {
    node->children[j + 1] = node->children[j];
  }
  node->children[index + 1] = z;
  node->num_children += 1;
  for (int j = node->n - 1; j >= index; j--) {
    node->keys[j + 1] = node->keys[j];
  }
  node->keys[index] = next_node->keys[median + 1];
  node->n = node->n + 1;
  if (btree_disk_write(next_node)) {
    return -1;
  }
  if (btree_disk_write(z)) {
    return -1;
  }
  if (btree_disk_write(node)) {
    return -1;
  }
  return 0;
}

KEY_VAL *btree_search(BTREE_NODE *node, char *key) {
  //
  //
  // LOCAL VARIABLES
  //
  //
  int i = 0;
//  KEY_VAL *found = malloc(sizeof(KEY_VAL));
  while (i < node->n && lsm_key_cmp(key, node->keys[i]) > 0) {
    i += 1;
  }
  if (i < node->n && lsm_key_cmp(key, node->keys[i]) == 0) {
    // TODO: Return the expected value
    return NULL;
  } else if (node->isLeaf) {
    return NULL;
  } else {
    BTREE_NODE *next_node = allocate_node(node->children[i]->filename);
    return btree_search(next_node, key);
  }
}

BTREE_NODE *allocate_node(char *file_inf) {
  //
  //
  // LOCAL VARIABLES
  //
  //
  BTREE_NODE *new_node = calloc(1, sizeof(BTREE_NODE));
  char *dbuf = calloc(PAGE_SIZE, 1);
  ssize_t rc = 0;
//  int new_file = 0;
  char *newfname = calloc(BT_FILENAME_SIZE, sizeof(char));
  if (file_inf == NULL) {
    rand_string(newfname, BT_FILENAME_SIZE);
  } else {
    strcpy(newfname, file_inf);
  }
//  if (access(newfname, R_OK | W_OK) == -1) {
//    new_file = 1;
//  }
  int fd = open(newfname, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
//  if (new_file) {
//    if (!posix_fallocate(fd, 0, PAGE_SIZE)) {
//      perror("Failed to allocate file size: ");
//      exit(1);
//    }
//  }
  if (fd == -1) {
    printf("LOG [PANIC]: Could not open fd for filename: %s\n", newfname);
    exit(1);
  }
  new_node->fd = fd; // mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
  if (( rc = read(fd, dbuf, PAGE_SIZE)) == -1) { // Failure
    perror("LOG [PANIC] Could not read from file: ");
    exit(1);
  } else if (rc > 0) { // Existing allocation
    unmarshall(new_node, dbuf);
  } else { // Fresh new allocation from NULL DB page
    perror("Maybe there was an error?: ");
    new_node->filename = malloc(MAX_FILENAME_SIZE);
    strcpy(new_node->filename, newfname);
    new_node->isLeaf = 1;
    new_node->n = 0;
  }
  new_node->allocated = 1;
  free(newfname);
  return new_node;
}

int marshall(BTREE_NODE *node, char **buffer) {
  //
  //
  // LOCAL VARIABLES
  //
  //
  int current_offset = 0;
  *buffer = calloc(PAGE_SIZE, sizeof(char));
  char *dest = *buffer;
  char *kvbuf = NULL;
  int kvsize;

  memcpy(dest + current_offset, node->filename, strlen(node->filename));
  current_offset += MAX_FILENAME_SIZE;

  memcpy(dest + current_offset, &( node->isLeaf ), sizeof(int));
  current_offset += sizeof(int);

  memcpy(dest + current_offset, &( node->n ), sizeof(int));
  current_offset += sizeof(int);

  memcpy(dest + current_offset, &( node->num_children ), sizeof(int));
  current_offset += sizeof(int);

  for (int i = 0; i < node->n; i++) {
    // Marshall individual Key Values
    kvsize = marshall_kv(node->keys[i], &kvbuf);
    memcpy(dest + current_offset, kvbuf, kvsize);
    free(kvbuf);
    kvbuf = NULL;
    current_offset += kvsize;
  }
  if (!node->isLeaf) {
    for (int i = 0; i < node->num_children; i++) {
      memcpy(dest + current_offset, node->children[i]->filename,
             strlen(node->children[i]->filename));
      current_offset += MAX_FILENAME_SIZE;
    }
  }
  return current_offset;
}

int unmarshall(BTREE_NODE *node, char *buf) {
  //
  //
  // LOCAL VARIABLES
  //
  //
  int seek = 0;
  char *kvbuf;

  node->filename = calloc(MAX_FILENAME_SIZE, sizeof(char));
  memcpy(node->filename, buf + seek, MAX_FILENAME_SIZE);
  seek += MAX_FILENAME_SIZE;

  memcpy(&( node->isLeaf ), buf + seek, sizeof(int));
  seek += sizeof(int);

  memcpy(&( node->n ), buf + seek, sizeof(int));
  seek += sizeof(int);

  memcpy(&( node->num_children ), buf + seek, sizeof(int));
  seek += sizeof(int);

//  if (node->keys == NULL) {
//    node->keys = malloc(sizeof(KEY_VAL *) * NODE_LIMIT); // TODO: Figure out actual size
//  }

  for (int i = 0; i < node->n; i++) {
    // Unarshall individual Key Values
    kvbuf = buf + seek;
    if (node->keys[i] == NULL) {
      node->keys[i] = malloc(sizeof(KEY_VAL));
    }
    seek += unmarshall_kv(node->keys[i], kvbuf);
  }
  if (!node->isLeaf) {
    for (int i = 0; i < node->num_children; i++) {
      node->children[i] = malloc(sizeof(BTREE_NODE));
      node->children[i]->filename = malloc(MAX_FILENAME_SIZE);
      memcpy(node->children[i]->filename, buf + seek, MAX_FILENAME_SIZE);
      seek += MAX_FILENAME_SIZE;
    }
  }
  return seek;
}

void cleanup(BTREE_NODE *root, int full) {
  if (root->isLeaf) {
    for (int i = 0; i < root->n; i++) {
      free(root->keys[i]->key);
      free(root->keys[i]->val);
    }
  } else {
    assert(root->children != NULL);
    for (int i = 0; i < root->n; i++) {
      cleanup(root->children[i], full);
    }
  }
  if (full) {
    close(root->fd);
    remove(root->filename);
  }
  free(root->filename);
  free(root);
}

int marshall_kv(KEY_VAL *key_val, char **buffer) {
  //
  //
  // LOCAL VARIABLES
  //
  //
  *buffer = malloc(MAX_ENTRY_SIZE + sizeof(time_t));
  memcpy(*buffer, key_val->key, strlen(key_val->key));
  memcpy(( *buffer ) + MAX_KEY_SIZE, key_val->val, strlen(key_val->val));
  memcpy(( *buffer ) + MAX_KEY_SIZE + MAX_VALUE_SIZE, &( key_val->timestamp ), sizeof(time_t));
  return MAX_KEY_SIZE + MAX_VALUE_SIZE + sizeof(time_t);
}

int unmarshall_kv(KEY_VAL *key_val, char *buf) {
  //
  //
  // LOCAL VARIABLES
  //
  //
  if (key_val->key == NULL) {
    key_val->key = malloc(MAX_KEY_SIZE);
  }
  if (key_val->val == NULL) {
    key_val->val = malloc(MAX_VALUE_SIZE);
  }
  memcpy(key_val->key, buf, MAX_KEY_SIZE);
  memcpy(key_val->val, buf + MAX_KEY_SIZE, MAX_VALUE_SIZE);
  memcpy(&( key_val->timestamp ), buf + MAX_KEY_SIZE + MAX_VALUE_SIZE, sizeof(time_t));
  return MAX_KEY_SIZE + MAX_VALUE_SIZE + sizeof(time_t);
}

/******** HELPERS ************/

void rand_string(char *str, size_t size) {
  const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
  if (size - 1 && str) {
    --size;
    for (size_t n = 0; n < size - 1; n++) {
      int key = rand() % (int) ( sizeof charset - 1 );
      str[n] = charset[key];
    }
    str[size - 1] = '\0';
  }
}
