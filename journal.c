/**
 * @file journal.c
 *
 * Function implementations for journaling.
 */

#include "journal.h"
#include "server-part1.h"

int log_transaction(transaction *tx) {
  pthread_mutex_lock(&journal_mutex);
  FILE *file = fopen("tx_log", "a");
  struct stat st;
  size_t data_len = 0;
  
  if (file == NULL) {
    perror("Could not open tx_log.");
    return -1;
  }

  tx->txb.txid = rand();
  if (!fwrite(&(tx->txb.txid), 1, sizeof(int), file)) { // issue begin (txid)
    perror("Could not write TxB: ");
    fclose(file);
    pthread_mutex_unlock(&journal_mutex);
    return -1;
  }
  if (fflush(file) != 0) {
    stat("tx_log", &st);
    ftruncate(fileno(file), st.st_size - sizeof(int)); // delete txid
    perror("Could not flush TxB.");
    fclose(file);
    pthread_mutex_unlock(&journal_mutex);
    return -1;
  }

  data_len = tx->db.data_len; // length prefix for easy unmarshalling
  if (!fwrite(&(tx->db.data_len), 1, sizeof(size_t), file)) { // issue data length
    stat("tx_log", &st);
    ftruncate(fileno(file), st.st_size - sizeof(int)); // delete txid
    perror("Could not write db.data: ");
    fclose(file);
    pthread_mutex_unlock(&journal_mutex);
    return -1;
  }
  if (fflush(file) != 0) {
    stat("tx_log", &st);
    ftruncate(fileno(file), st.st_size - sizeof(int) - sizeof(size_t)); // delete txid/data_len
    perror("Could not flush data.");
    fclose(file);
    pthread_mutex_unlock(&journal_mutex);
    return -1;
  }

  if (!fwrite(tx->db.data, 1, data_len, file)) { // issue data
    stat("tx_log", &st);
    ftruncate(fileno(file), st.st_size - sizeof(int) - sizeof(size_t)); // delete txid/data_len
    perror("Could not write db.data: ");
    fclose(file);
    pthread_mutex_unlock(&journal_mutex);
    return -1;
  }
  if (fflush(file) != 0) {
    stat("tx_log", &st);
    ftruncate(fileno(file), st.st_size - sizeof(int) - sizeof(size_t) - data_len); // delete txid/data_len/data
    perror("Could not flush data.");
    fclose(file);
    pthread_mutex_unlock(&journal_mutex);
    return -1;
  }

  if (!fwrite("\0", 1, 1, file)) { // issue null byte
    stat("tx_log", &st);
    ftruncate(fileno(file), st.st_size - sizeof(int) - sizeof(size_t) - data_len); // delete txid/data_len/data
    perror("Could not write db.data: ");
    fclose(file);
    pthread_mutex_unlock(&journal_mutex);
    return -1;
  }
  if (fflush(file) != 0) {
    stat("tx_log", &st);
    ftruncate(fileno(file), st.st_size - sizeof(int) - sizeof(size_t) - data_len - 1); // delete txid/data_len/data/nullbyte
    perror("Could not flush data.");
    fclose(file);
    pthread_mutex_unlock(&journal_mutex);
    return -1;
  }

  tx->valid = 1; // mark transaction as valid
  if (!fwrite(&( tx->valid ), 1, sizeof(int), file)) { // issue valid entry indicator
    stat("tx_log", &st);
    ftruncate(fileno(file), st.st_size - sizeof(int) - sizeof(size_t) - data_len - 1); // delete txid/data_len/data/nullbyte
    perror("Could not write valid bit: ");
    fclose(file);
    pthread_mutex_unlock(&journal_mutex);
    return -1;
  }
  if (fflush(file) != 0) {
    stat("tx_log", &st);
    ftruncate(fileno(file), st.st_size - sizeof(int) - sizeof(size_t) - data_len - 1 - sizeof(int)); // delete txid/data_len/data/nullbyte/valid
    perror("Could not flush 'valid'.");
    fclose(file);
    pthread_mutex_unlock(&journal_mutex);
    return -1;
  }

  tx->txe.committed = 1; // other writes good, mark committed flag
  if (!fwrite(&( tx->txe.committed ), 1, sizeof(int), file)) { // issue commit flag
    stat("tx_log", &st);
    ftruncate(fileno(file), st.st_size - sizeof(int) - sizeof(size_t) - data_len - 1 - sizeof(int)); // delete txid/data_len/data/nullbyte/valid
    perror("Could not flush 'committed': ");
    fclose(file);
    pthread_mutex_unlock(&journal_mutex);
    return -1;
  }
  if (fflush(file) != 0) {
    stat("tx_log", &st);
    ftruncate(fileno(file), st.st_size - sizeof(int) - sizeof(size_t) - data_len - 1 - sizeof(int) - sizeof(int)); // delete txid/data_len/data/nullbyte/valid/committed
    perror("Could not flush 'TxE': ");
    fclose(file);
    pthread_mutex_unlock(&journal_mutex);
    return -1;
  }

  if (!fwrite("\n", 1, 1, file)) { // issue entry delimiter
    stat("tx_log", &st);
    ftruncate(fileno(file), st.st_size - sizeof(int) - sizeof(size_t) - data_len - 1 - sizeof(int) - sizeof(int)); // delete txid/data_len/data/nullbyte/valid/committed
    perror("Could not flush 'committed': ");
    fclose(file);
    pthread_mutex_unlock(&journal_mutex);
    return -1;
  }
  if (fflush(file) != 0) {
    stat("tx_log", &st);
    ftruncate(fileno(file), st.st_size - sizeof(int) - sizeof(size_t) - data_len - 1 - sizeof(int) - sizeof(int) - 1); // delete txid/data_len/data/nullbyte/valid/committed/newline
    perror("Could not flush 'TxE': ");
    fclose(file);
    pthread_mutex_unlock(&journal_mutex);
    return -1;
  }

  // log entry successfully written

  fclose(file);

  pthread_mutex_unlock(&journal_mutex);

  // return tx->txb.txid;
  return 0; // OK
}

int unmarshall_journal_entry(transaction *tmp_transaction, char *tmp_entry) {
  char *entry_pos = tmp_entry;

  memcpy(&(tmp_transaction->txb.txid), entry_pos, sizeof(int)); // txid
  entry_pos += sizeof(int);

  memcpy(&(tmp_transaction->db.data_len), entry_pos, sizeof(size_t)); // length prefix
  entry_pos += sizeof(size_t);

  tmp_transaction->db.data = (char *) calloc(tmp_transaction->db.data_len + 1, sizeof(char));
  // printf("db.data: %s, db.data_len: %d\n", tmp_transaction->db.data, tmp_transaction->db.data_len);
  printf("asking for %d bytes: %s\n", (tmp_transaction->db.data_len + 1)*sizeof(char), strerror(errno));
  memcpy(tmp_transaction->db.data, entry_pos, tmp_transaction->db.data_len + 1); // data and null byte
  entry_pos += tmp_transaction->db.data_len + 1;

  memcpy(&(tmp_transaction->valid), entry_pos, sizeof(int)); // valid
  entry_pos += sizeof(int);

  memcpy(&(tmp_transaction->txe.committed), entry_pos, sizeof(int)); // committed
  entry_pos += sizeof(int);

  // dont need newline char

  return 0;
}

int remove_transaction(int txid) {
  pthread_mutex_lock(&journal_mutex);

  char *tmp_entry = (char *) calloc(MAX_JOURNAL_ENTRY_SIZE, sizeof(char));
  transaction *tmp_transaction = (transaction *) calloc(sizeof(transaction), sizeof(char));
  int temp_txid = 0;
  long offset = 0;
  int z = 0;

  FILE *file = fopen("tx_log", "r+");

  if (file == NULL) {
    perror("Could not open tx_log.");
    pthread_mutex_unlock(&journal_mutex);
    return -1;
  }

  while (1) {
    if (fgets(tmp_entry, MAX_JOURNAL_ENTRY_SIZE, file) != NULL) { // read to newline (including newline char, and advances file position to next entry)
      if (unmarshall_journal_entry(tmp_transaction, tmp_entry) != 0) {
        perror("Could not unmarshall journal entry");
        free(tmp_transaction);
        free(tmp_entry);
        tmp_entry = NULL;
        tmp_transaction = NULL;
        fclose(file);

        pthread_mutex_unlock(&journal_mutex);

        return -1;
      }

      temp_txid = tmp_transaction->txb.txid;
      if (temp_txid == txid) { // found transaction in log
        if (!tmp_transaction->valid) { // if already marked as invalid, ignore
          free(tmp_entry);
          tmp_entry = NULL;
          tmp_transaction = NULL;
          fclose(file);
          pthread_mutex_unlock(&journal_mutex);
          return 0;
        }

        // otherwise, seek back and mark invalid
        offset = (sizeof(int) + sizeof(int) + 1) * -1;
        fseek(file, offset, SEEK_CUR);
        fwrite(&z, 1, sizeof(int), file); // mark transaction as invalid (subsequent recovers/removes should ignore it)
        if (fflush(file) != 0) {
          perror("Could not remove transaction.");
          free(tmp_entry);
          tmp_entry = NULL;
          tmp_transaction = NULL;
          fclose(file);
          pthread_mutex_unlock(&journal_mutex);
          return -1;
        }
        free(tmp_entry);
        tmp_entry = NULL;
        tmp_transaction = NULL;
        // rewind(file); // dont need
        fclose(file);
        pthread_mutex_unlock(&journal_mutex);
        return 0;
      }
    } else { // end of file
      break;
    }
  }

  if (tmp_transaction->db.data) {
    free(tmp_transaction->db.data);
  }

  free(tmp_transaction);
  free(tmp_entry);
  tmp_entry = NULL;
  tmp_transaction = NULL;
  fclose(file);

  pthread_mutex_unlock(&journal_mutex);

  return -1; // tx not found
}

int flush_log() {
  int rc;
//  pthread_mutex_lock(&journal_mutex);
  rc = remove("tx_log");
//  pthread_mutex_unlock(&journal_mutex);
  return rc;
}

int recover() {
  pthread_mutex_lock(&journal_mutex);

  char *tmp_entry = (char *) calloc(MAX_JOURNAL_ENTRY_SIZE, sizeof(char));
  transaction *tmp_transaction = (transaction *) calloc(sizeof(transaction), sizeof(char));
  char *key = NULL;
  char *value = NULL;
  char *request_type;
  char *response = NULL;
  char *save_ptr;
  int response_size;

  FILE *file = fopen("tx_log", "r");

  if (file == NULL) {
    perror("Could not open tx_log.");
    pthread_mutex_unlock(&journal_mutex);
    return -1;
  }

  while (1) { // should we be re-journaling while replaying journal? cant read/write to tx_log at the same time though
    if (fgets(tmp_entry, MAX_JOURNAL_ENTRY_SIZE, file) != NULL) {
      if (unmarshall_journal_entry(tmp_transaction, tmp_entry) != 0) {
        perror("Could not unmarshall journal entry");
        free(tmp_transaction);
        free(tmp_entry);
        tmp_entry = NULL;
        tmp_transaction = NULL;
        fclose(file);

        pthread_mutex_unlock(&journal_mutex);

        return -1;
      }

      if (!tmp_transaction->valid) {
        continue; // ignore
      }

      request_type = strtok_r(tmp_transaction->db.data, " ", &save_ptr);
      key = strtok_r(NULL, " ", &save_ptr);
      value = strtok_r(NULL, " ", &save_ptr);
      if (request_type == NULL || key == NULL) {
        printf("Invalid key/command received\n");
      } else if (strncmp(request_type, "GET", 3) == 0) {
        server_1_get_request(key, &response, &response_size);
      } else if (strncmp(request_type, "PUT", 3) == 0) {
        server_1_put_request(key, value, &response, &response_size);
      } else if (strncmp(request_type, "INSERT", 6) == 0) {
        server_1_insert_request(key, value, &response, &response_size);
      } else if (strncmp(request_type, "DELETE", 6) == 0) {
        server_1_delete_request(key, &response, &response_size);
      } else { // incorrectly formatted journal entry, ignore
        continue;
      }

      if (response != NULL) {
        free(response);
      }

      response = NULL;

    } else { // end of file
      break;
    }
  }

  if (tmp_transaction->db.data) {
    free(tmp_transaction->db.data);
  }

  free(tmp_transaction);
  free(tmp_entry);
  fclose(file);

  remove("tx_log"); // flush_log but already have lock

  pthread_mutex_unlock(&journal_mutex);

  return 0;
}
