#include <stdio.h>

#include "journal.h"

int log_transaction(transaction *tx)
{
    FILE *fp = fopen("tx_log", "a");

    if (fp == NULL) {
        perror("Could not open tx_log.");
        return -1;
    }

    fwrite(tx->txb.txid, 1, sizeof(int), fp); // begin.1
    fwrite(*(tx->txb.key), 1, strlen(*(tx->txb.key)), fp); // begin.2
    fwrite(*(tx->data.val), 1, strlen(*(tx->data.val)), fp); // data
    fwrite(tx->txe.complete, 1, sizeof(int), fp); // commit
    fwrite("\n", 1, 1, fp); // entry delimiter (sizeof(transaction) positions delimits them too)
    
    if (fflush(fp) != 0) {
        perror("Could not flush stream.");
        fclose(fp);
        return -1;
    }

    // log entry successfully written
    
    fclose(fp);
    return 0;
}
