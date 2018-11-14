/**
 * @file journal.h
 * 
 * Provides functions to create and issue journal transactions.
 */

#ifndef P1_CRSF_JOURNAL_H
#define P1_CRSF_JOURNAL_H

/* Includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"

/* Defines */
#define MAX_JOURNAL_ENTRY_SIZE (sizeof(transaction) - sizeof(char *) + MAX_ENTRY_SIZE)

typedef struct TxB {
    int txid;
} TxB;

typedef struct Db {
    char *data; // client request string
} Db;

typedef struct TxE {
    int committed;
} TxE;

typedef struct transaction {
    TxB txb;
    Db data;
    int valid;
    TxE txe;
} transaction;


/* Function prototypes  */

/**
 * Logs transaction to disk (writes TxB, Db, TxE)
 * @param tx the transaction to log
 * @return txid: success, -1: error
 */
int log_transaction(transaction *tx);

/**
 * Remove transaction (on failed c0 invocation?)
 * @param txid the transaction id to search for and remove
 * @return 0: success, -1: error
 */
int remove_transaction(int txid);

/**
 * Flush log entries (done when c0 entries are flushed to c1)
 * @return 0: success, -1: error
 */
int flush_log();

/**
 * Replay log since last c0_flush
 * @return 0: success, -1: error
 */
int recover();

#endif /* P1_CRSF_JOURNAL_H */
