//
// Created by sps5394 on 11/12/18.
//
#include <assert.h>
#include "dbtree.h"

void cleanup(BTREE_NODE *cleanup, int full);

BTREE_NODE *setup();

void assert_file_exists(char *filename);

void generate_value(char *value);

void generate_key(char *key);

int main() {
  //
  //
  // LOCAL VARIABLES
  //
  //
  BTREE_NODE *root;
  char *key;
  char *value;
  KEY_VAL key_val;
  time_t current_time;
  int rc;

  // SETUP
  srand(0);
  // DEFINE UNIT TESTS
  printf("Running test 0\n");
  root = setup();
  // TEST 0: BTREE CREATION WORKS
  assert(root != NULL);
  assert(root->n == 0);
  assert(root->filename != NULL);
  assert(root->isLeaf == 1);
  assert_file_exists(root->filename);

  cleanup(root, 1);
  root = NULL;
  printf("Success test 0\n");


  // TEST 1: BTREE KEY INSERTION
  printf("Running test 1\n");
  root = setup();
  key = malloc(MAX_KEY_SIZE);
  value = malloc(MAX_VALUE_SIZE);
  generate_key(key);
  generate_value(value);
  key_val.key = key;
  key_val.val = value;
  key_val.timestamp = time(&current_time);
  rc = btree_insert(&root, &key_val);
  assert(rc == 0);
  printf("Success test 1\n");

  printf("Running test 1.2\n");
  for (int i = 0; i < 1024; i++) {
    printf("Inserting key number %d\n", i);
    key = malloc(MAX_KEY_SIZE);
    value = malloc(MAX_VALUE_SIZE);
    generate_key(key);
    generate_value(value);
    key_val.key = key;
    key_val.val = value;
    key_val.timestamp = time(&current_time);
    rc = btree_insert(&root, &key_val);
    assert(rc == 0);
  }
  printf("Success test 1.2\n");

  // TEST 2: BTREE READ WORKS

  printf("Running test 2\n");


  printf("Success test 2\n");

  printf("All tests passed\n");
}

void generate_key(char *key) {
  rand_string(key, MAX_KEY_SIZE);
}

void generate_value(char *value) {
  rand_string(value, MAX_VALUE_SIZE);
}

BTREE_NODE *setup() {
  chdir("/cse511-p2");
  return btree_create();
}

void assert_file_exists(char *filename) {
  assert(!access(filename, R_OK | W_OK));
}
