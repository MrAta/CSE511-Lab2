/**
 * @file journal.h
 * 
 * Provides functions to create and issue journal transactions.
 */

#ifndef P1_CRSF_JOURNAL_H
#define P1_CRSF_JOURNAL_H

/* Includes */


/* Defines */

typedef struct TxB {
    int txid;
    char *key; // "disk blocks to be changed" -> c0_node to be changed?
}TxB;

typedef struct Db {
    char *val;
}Db;

typedef struct TxE {
    int complete;
}TxE;

typedef struct transaction {
    TxB txb;
    Db data;
    TxE txe;
}transaction;


/* Function prototypes  */

/**
 * Logs transaction to disk (writes TxB, Db, TxE)
 * 
 * @return 0: success, -1: failure
 */
int log_transaction(transaction);


#endif /* P1_CRSF_JOURNAL_H */
