#include "journal.h"

int log_transaction(transaction *tx)
{
    FILE *file = fopen("tx_log", "a");

    if (file == NULL) {
        perror("Could not open tx_log.");
        return -1;
    }

    tx->txb.txid = rand();
    fwrite(tx->txb.txid, 1, sizeof(int), file); // issue begin
    if (fflush(file) != 0) {
        perror("Could not flush TxB.");
        fclose(file);
        return -1;
    }

    fwrite(*(tx->Db.data), 1, strlen(*(tx->Db.data)), file); // issue data
    if (fflush(file) != 0) {
        perror("Could not flush Db.");
        fclose(file);
        return -1;
    }

    tx->valid = 1; // mark transaction as valid
    fwrite(tx->valid, 1, sizeof(int), file); // issue valid entry indicator
    if (fflush(file) != 0) {
        perror("Could not flush 'valid'.");
        fclose(file);
        return -1;
    }

    tx->txe = 1; // other writes good, mark committed flag
    fwrite(tx->txe, 1, sizeof(int), file); // issue commit
    fwrite("\n", 1, 1, file); // entry delimiter
    if (fflush(file) != 0) {
        perror("Could not flush TxE.");
        fclose(file);
        return -1;
    }

    // log entry successfully written
    
    fclose(file);
    return tx->txb.txid;
}

int remove_transaction(int txid)
{
    char *tmp_entry = NULL;
    transaction *tmp_transaction = NULL;
    int temp_txid = 0;
    int offset = 0;

    FILE *file = fopen("tx_log", "r+");

    if (file == NULL) {
        perror("Could not open tx_log.");
        return -1;
    }

    tmp_entry = (char *) calloc(MAX_JOURNAL_ENTRY_SIZE, sizeof(char));

    while (1) {
        if (fgets(tmp_entry, MAX_JOURNAL_ENTRY_SIZE, file) != NULL) { // read to newline
            tmp_transaction = (transaction *)tmp_entry;
            temp_txid = tmp_transaction->txb.txid;
            if (temp_txid == txid) { // found transaction in log
                if (!tmp_transaction->valid) { // if already marked as invalid, ignore
                    free(tmp_entry);
                    tmp_entry = NULL;
                    tmp_transaction = NULL;
                    fclose(file);
                    return 0;
                }

                // otherwise, seek back and mark invalid
                offset = (sizeof(int) + sizeof(int)) * -1; // seek back to position to overwrite 4 byte 'valid' variable
                fseek(file, offset, SEEK_CUR);
                fwrite(0, 1, sizeof(int), file); // mark transaction as invalid (subsequent recovers/removes should ignore it)
                if (fflush(fclose(file);) != 0) {
                    perror("Could not remove transaction.");
                    free(tmp_entry);
                    tmp_entry = NULL;
                    tmp_transaction = NULL;
                    fclose(file);
                    return -1;
                }
                free(tmp_entry);
                tmp_entry = NULL;
                tmp_transaction = NULL;
                // rewind(file); // dont need
                fclose(file);
                return 0;
            } else {
                continue;
            }
        } else { // end of file
            break;
        }
    }

    free(tmp_entry);
    tmp_entry = NULL;
    tmp_transaction = NULL;
    fclose(file);

    return -1; // tx not found
}

int flush_log()
{
    return remove("tx_log");
}

int recover()
{
    // during recover, check if tx is valid, otherwise ignore it bc it was removed
    // replay in correct order, i.e. tx_log begin line to end
}