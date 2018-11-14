//
// Created by sps5394 on 11/14/18.
//
#include <assert.h>
#include "c1.h"

int main() {
  //
  //
  // LOCAL VARIABLES
  //
  //
  c0_node *node[MAX_C0_SIZE];
  int rc;
  printf("%d\n", truncate("./.db/0", 0));

  c1_init();
  printf("Running TEST 1\n");
  printf("Insert key\n");
  node[0] = malloc(sizeof(c0_node));
  node[0]->key = "Testkey";
  node[0]->value = "TestValue";
  node[0]->flag = 0;

  rc = c1_batch_insert(node, 1);
  assert(rc == 0);
  printf("Finished running TEST 1\n");
  // RESET TO INITIAL STATE
  free(node[0]);
  printf("%d\n", truncate("./.db/0", 0));
  file_counter = 0;

  printf("Running TEST 2\n");
  for (int i = 0; i < MAX_C0_SIZE; i++) {
    node[i] = malloc(sizeof(c0_node));
    node[i]->key = malloc(MAX_KEY_SIZE);
    snprintf(node[i]->key, MAX_KEY_SIZE, "%dTestkey", i);
    node[i]->value = malloc(MAX_VALUE_SIZE);
    snprintf(node[i]->value, MAX_VALUE_SIZE, "TestValue%d", i);
  }
  rc = c1_batch_insert(node, MAX_C0_SIZE);
  assert(rc == 0);
  printf("Finished running TEST 2\n");

  printf("Running TEST 3\n");
  for (int i = 0; i < MAX_C0_SIZE; i++) {
    node[i] = malloc(sizeof(c0_node));
    node[i]->key = malloc(MAX_KEY_SIZE);
    snprintf(node[i]->key, MAX_KEY_SIZE, "%dTestkey", i);
    node[i]->value = malloc(MAX_VALUE_SIZE);
    snprintf(node[i]->value, MAX_VALUE_SIZE, "TestValue%d", i);
  }
  rc = c1_batch_insert(node, MAX_C0_SIZE);
  assert(rc == 0);
  int check = access("./.db/1", R_OK);
  assert(check == 0);

  printf("Successfully ran all tests\n");
  return 0;
}
