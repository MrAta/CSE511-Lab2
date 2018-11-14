#include <stdio.h>

#include "journal.h"

int log_transaction(transaction *tx)
{
    FILE *fp = fopen("tx_log", "a");

    if (fp == NULL) {
        perror("Could not open tx_log.");
        return -1;
    }

    tx->txb.txid = rand();
    fwrite(tx->txb.txid, 1, sizeof(int), fp); // begin.1
    fwrite(*(tx->txb.key), 1, strlen(*(tx->txb.key)), fp); // begin.2
    if (fflush(fp) != 0) {
        perror("Could not flush TxB.");
        fclose(fp);
        return -1;
    }

    fwrite(*(tx->data), 1, strlen(*(tx->data.val)), fp); // data
    if (fflush(fp) != 0) {
        perror("Could not flush Db.");
        fclose(fp);
        return -1;
    }

    tx->txe = 1; // other writes good, mark committed flag
    fwrite(tx->txe, 1, sizeof(int), fp); // commit
    fwrite("\n", 1, 1, fp); // entry delimiter
    if (fflush(fp) != 0) {
        perror("Could not flush TxE.");
        fclose(fp);
        return -1;
    }

    // log entry successfully written
    
    fclose(fp);
    return 0;
}
