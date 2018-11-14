//
// Created by sps5394 on 11/13/18.
//

#ifndef __linux__

#include <zconf.h>
#include <memory.h>

#endif

#include "c1.h"

int file_counter = 0;

char *c1_search(FILE* file, char *key, int startline, int endline) ;

int c1_batch_insert(c0_node nodes[], int size) {
  //
  //
  // LOCAL VARIABLES
  //
  //
  int fd, oldfd = 0, rc;
  char *filename;
  char *keyval;

  asprintf(&filename, "%s/%d", DB_DIR, file_counter++);
  if (( fd = open(filename, O_WRONLY | O_CREAT, S_IWUSR)) == -1) {
    free(filename);
    perror("Failed to open new file: ");
    return -1;
  }
  free(filename);
  filename = NULL;
  for (int i = 0; i < size; i++) {
    if (( rc = asprintf(&keyval, "%.*s %.*s %d\n", (int) MAX_KEY_SIZE, nodes[i].key,
                        (int) MAX_VALUE_SIZE,
                        nodes[i].value, nodes[i].flag)) == -1) {
      close(fd);
      fprintf(stderr, "Failed to create db entry\n");
      return -1;
    }
    if (write(fd, keyval, LINE_SIZE) == -1) {
      free(keyval);
      close(fd);
      return -1;
    }
    free(keyval);
    keyval = NULL;
  }

  // Trigger a merge
  if (file_counter > 0) {
    // Merge with existing file
    asprintf(&filename, "%s/%d", DB_DIR, file_counter - 1);
    if (( oldfd = open(filename, O_RDONLY, S_IRUSR)) == -1) {
      perror("Failed to open old file: ");
      free(filename);
      close(fd);
      return -1;
    }
    free(filename);
    filename = NULL;

    if (merge(fdopen(fd, "r"), fdopen(oldfd, "r")) == -1) {
      close(fd);
      close(oldfd);
      return -1;
    }
  }

  // Update SSTable
  // TODO: Fill if not using binary search

  // Complete
  close(fd);
  if (oldfd != 0) {
    close(oldfd);
  }
  return 0;
}

int merge(FILE *file1, FILE *file2) {
  //
  //
  // LOCAL VARIABLES
  //
  //
  char *filename;
  char *line1 = NULL, *line2 = NULL, *key1, *key2, *val1, *val2;
  int mergefd, valid1, valid2;
  size_t len1, len2;
  ssize_t size1 = 0, size2 = 0;

  asprintf(&filename, "%s/%d", DB_DIR, file_counter++);
  if (( mergefd = open(filename, O_WRONLY | O_CREAT, S_IWUSR)) == -1) {
    perror("Failed to open a new merge file: ");
    free(filename);
    return -1;
  }

  while (1) {
    if (line1 == NULL) {
      size1 = getline(&line1, &len1, file1);
      if (size1 == -1) {
        break;
      }
    }
    if (line2 == NULL) {
      size2 = getline(&line2, &len2, file2);
      if (size2 == -1) {
        break;
      }
    }
    sscanf(line1, "%ms %ms %d", &key1, &val1, &valid1);
    sscanf(line2, "%ms %ms %d", &key2, &val2, &valid2);
    // Compare
    if (strcmp(key1, key2) < 0) {
      if (!valid1) {
        if (write(mergefd, line1, LINE_SIZE) == -1) {
          perror("Failed to write line: ");
          return -1;
        }
      }
      free(line1);
      line1 = NULL;
    } else if (strcmp(key1, key2) > 0) {
      if (!valid2) {
        if (write(mergefd, line2, LINE_SIZE) == -1) {
          perror("Failed to write line: ");
          return -1;
        }
      }
      free(line2);
      line2 = NULL;
    } else {
      // Equal
      if (!valid2) {
        if (write(mergefd, line2, LINE_SIZE) == -1) {
          perror("Failed to write line: ");
          return -1;
        }
      }
      free(line2);
      line2 = NULL;
      free(line1);
      line1 = NULL;
    }
    free(key1);
    free(key2);
    free(val1);
    free(val2);
  }

  if (size1 != -1) {
    while (( size1 = getline(&line1, &len1, file1)) != -1) {
      if (write(mergefd, line1, LINE_SIZE) == -1) {
        perror("Failed to write line: ");
        return -1;
      }
    }
  }

  if (size2 != -1) {
    while (( size2 = getline(&line2, &len2, file2)) != -1) {
      if (write(mergefd, line2, LINE_SIZE) == -1) {
        perror("Failed to write line: ");
        return -1;
      }
    }
  }
  if (line1 != NULL) {
    free(line1);
  }
  if (line2 != NULL) {
    free(line2);
  }
  fclose(file1);
  fclose(file2);
  return 0;
}

char *c1_get(char *key) {
  //
  //
  // LOCAL VARIABLES
  //
  //
  FILE *file;
  size_t size;
  int numlines = 0;
  char *fname, *val;

  asprintf(&fname, "%s/%d", DB_DIR, file_counter);
  file = fopen(fname, "r");
  free(fname);
  if (file == NULL) {
    fprintf(stderr, "Failed to open current counter file\n");
    return NULL;
  }
  fseek(file, 0, SEEK_END);
  size = (size_t) ftell(file);
  numlines = (int) ( size / LINE_SIZE );
  val = c1_search(file, key, 0, numlines);
  fclose(file);
  return val;
}

char *c1_search(FILE* file, char *key, int startline, int endline) {
  char *line, *r_key, *val;
  size_t len, valid;

  if (endline >= 1) {
    int mid = (( endline - startline ) / 2 ) + 1;
    fseek(file, mid * LINE_SIZE, SEEK_SET);
    getline(&line, &len, file);
    sscanf(line, "%ms %ms %d", &r_key, &val, &valid);
    free(line);
    if (strcmp(r_key, key) == 0) {
      free(r_key);
      return val;
    } else if (strcmp(r_key, key) > 0) {
      free(r_key);
      free(val);
      return c1_search(file, key, startline, mid - 1);
    } else {
      free(r_key);
      free(val);
      return c1_search(file, key, mid + 1, endline);
    }
  } else {
    return NULL;
  }
}
