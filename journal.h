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
#include <sys/stat.h>
#include "common.h"

/* Defines */
#define MAX_JOURNAL_ENTRY_SIZE (22 + MAX_ENTRY_SIZE + 2 + 6) // 22 bytes for metadata, key/val ENTRY size, 2 spaces and 6 chars for INSERT

typedef struct TxB {
    int txid;
} TxB;

typedef struct Db { // server_handler should supply this
    char *data; // client request string
    size_t data_len;
    // int fd; // socket will be closed by the time we try to recover; during recovery just retry the requests, no notification to client
} Db;

typedef struct TxE {
    int committed;
} TxE;

typedef struct transaction {
    TxB txb;
    Db db;
    int valid;
    TxE txe;
} transaction;

/* Globals */
pthread_mutex_t journal_mutex;

/* Function prototypes  */

/**
 * Logs transaction to disk (writes TxB, Db, TxE)
 * @param tx the transaction to log
 * @return txid: success, -1: error
 */
int log_transaction(transaction *tx);

/**
 * Unmarshalls data into transaction struct from serialized journal entry
 * @param tmp_transaction output struct for data
 * @param tmp_entry serialized data
 * @return 0: success, -1: error
 */
int unmarshall_journal_entry(transaction *tmp_transaction, char *tmp_entry);

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
