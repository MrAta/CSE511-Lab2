//
// Created by sps5394 on 11/10/18.
//


#include "lsm.h"

int _lsm_flush(lsm_t *lsm) {

}

int lsm_key_cmp(lsm_key_t k1, lsm_key_t k2) {
  return strcmp(k1, k2);
}
