//
// Created by sps5394 on 11/10/18.
//

#ifndef P2_CSRF_LSM_H
#define P2_CSRF_LSM_H

typedef char * lsm_key_t;
typedef char * lsm_val_t;
#include <stdlib.h>
#include <string.h>

typedef struct {
  size_t capacity;
  size_t size;
//  hb_tree *avl;
} lsm_t;

typedef struct {
} _lsm_node_t;

/**
 * This function initializes the memtable and
 * files on disk
 * @param size The size of the memtable which will
 * be held in memory
 * @return lsm_t * to the generated lsm tree; NULL if failure
 */
lsm_t *lsm_init(size_t size);

/**
 * Performs any necessary cleanup of the LSM and then frees the
 * allocated memory
 * @param lsm instance of lsm tree
 * @return 0 if successful; -1 if failed
 */
int lsm_destroy(lsm_t *lsm);

/**
 * Retrieves a value from the LSM based on the provided key
 * @param lsm instance of lsm tree
 * @param key key to search with
 * @return value from associated storage. NULL if failure
 */
lsm_val_t lsm_get(lsm_t *lsm, lsm_key_t key);

/**
 * Adds the passed key value to the storage
 * @param lsm instance of lsm tree
 * @param key key to insert for
 * @param val value associated with key
 * @return 0 if successful; -1 if failure
 */
int lsm_insert(lsm_t *lsm, lsm_key_t key, lsm_val_t val);

/**
 * Changes an existing value in the storage
 * @param lsm instance of lsm tree
 * @param key key to insert for
 * @param val value associated with key
 * @return 0 if successful; -1 if failure
 */
int lsm_put(lsm_t *lsm, lsm_key_t key, lsm_val_t val);

/**
 * Deletes the key and associated value from underlying storage
 * @param lsm instance of lsm tree
 * @param key key to insert for
 * @return 0 if successful; -1 if failure
 */
int lsm_delete(lsm_t *lsm, lsm_key_t key);

/**
 * Internal function to search persistent storage for the provided
 * key
 * @param lsm instance of the lsm tree
 * @param key key to search with
 * @return instance of _lsm_node_t; NULL if failure
 */
_lsm_node_t *lsm_search_disk(lsm_t *lsm, lsm_key_t key);

/**
 * Triggers an LSM flush to disk.
 * @param lsm instance of the lsm tree
 * @return 0 if successful; -1 if failure
 */
int _lsm_flush(lsm_t *lsm);

/**
 * Triggers an lsm compaction
 * @param lsm instance of the lsm tree
 * @return 0 if successful; -1 if failure
 */
int _lsm_compact(lsm_t *lsm);

/**
 * Runs a comparison on the value and returns the result.
 * -ve if k1 < k2, - if k1 == k2, +ve if k1 > k2.
 */
int lsm_key_cmp(lsm_key_t k1, lsm_key_t k2);

#endif //P2_CSRF_LSM_H
