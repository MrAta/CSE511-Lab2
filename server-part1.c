//
// Created by sps5394 on 10/18/18.
//
#include "server-part1.h"
#include "c0.h"
#include "c1.h"

#define CLOCKID CLOCK_REALTIME
#define SIG (SIGRTMIN + 3)
#define TIMER_FREQ_S 60 // change this back
// #define C0_SIZE_TRSH 4
#define errExit(msg)    do { perror(msg); exit(EXIT_FAILURE); \
                        } while (0)
pthread_mutex_t c0_mutex;
pthread_mutex_t cache_mutex;
c0_node * _T;

static void my_timer_handler(int sig, siginfo_t *si, void *uc) {
  // handled by main thread only

  pthread_mutex_lock(&c0_mutex);

  // only helper threads call log_trans so main thread could never deadlock itself by waiting on this
  pthread_mutex_lock(&journal_mutex);
  c0_node * c0_batch = _T;
  _T = NULL;
  // pthread_mutex_unlock(&c0_mutex);
  c0_dump(c0_batch);
  pthread_mutex_unlock(&journal_mutex);

  pthread_mutex_unlock(&c0_mutex);
}

struct sockaddr_in address;

int server_1_get_request(char *key, char **ret_buffer, int *ret_size) {
  //
  //
  // Local Variables
  //
  //
  struct node *cache_lookup;

  if (ret_buffer == NULL || ret_size == NULL || key == NULL) {
    return EXIT_FAILURE;
  }
  // Perform cache lookup
  // Perform cache lookup
  pthread_mutex_lock(&cache_mutex);
  cache_lookup = cache_get(key);
  pthread_mutex_unlock(&cache_mutex);
  if (cache_lookup != NULL) {
    // HIT
    *ret_buffer = calloc(MAX_ENTRY_SIZE, sizeof(char *));
    strcpy(*ret_buffer, cache_lookup->defn);
    *ret_size = (int) strlen(*ret_buffer);
    return 0;
  }
  // Check c0:
  pthread_mutex_lock(&c0_mutex);
  c0_node *c0_lookup = Get(_T, key);
  pthread_mutex_unlock(&c0_mutex);
  if (c0_lookup != NULL) {
    // HITz
    *ret_buffer = calloc(MAX_ENTRY_SIZE, sizeof(char *));
    strcpy(*ret_buffer, c0_lookup->value);
    *ret_size = (int) strlen(*ret_buffer);
    pthread_mutex_lock(&cache_mutex);
    cache_put(key, *ret_buffer);
    pthread_mutex_unlock(&cache_mutex);
    return 0;
  }else{
    //Go to C1
    char * value = c1_get(key);
    *ret_buffer = calloc(MAX_ENTRY_SIZE, sizeof(char *));
    strcpy(*ret_buffer, value);
    *ret_size = (int) strlen(*ret_buffer);
    pthread_mutex_lock(&cache_mutex);
    cache_put(key, *ret_buffer);
    pthread_mutex_unlock(&cache_mutex);
    return 0;
  }
  return EXIT_SUCCESS;
}

int server_1_put_request(char *key, char *value, char **ret_buffer, int *ret_size) {

  pthread_mutex_lock(&c0_mutex);
  if(Get(_T, key)!=NULL){
    Update(_T, key, value,0);
    pthread_mutex_unlock(&c0_mutex);
  }
  else{
    _T = Insert(_T, key, value,0);
    if (c0_size(_T) == MAX_C0_SIZE){
      c0_node * c0_batch = _T;
      _T = NULL;
      // pthread_mutex_unlock(&c0_mutex);
      c0_dump(c0_batch);
      pthread_mutex_unlock(&c0_mutex);
    } else{
      pthread_mutex_unlock(&c0_mutex);
    }
  }
  *ret_buffer = calloc(MAX_ENTRY_SIZE, sizeof(char *));
  strcpy(*ret_buffer, "SUCCESS");
  *ret_size = 7;
  pthread_mutex_lock(&cache_mutex);
  cache_put(key, value);
  pthread_mutex_unlock(&cache_mutex);

  return EXIT_SUCCESS;
}

int server_1_insert_request(char *key, char *value, char **ret_buffer, int *ret_size) {
  pthread_mutex_lock(&cache_mutex);
  if (cache_get(key) != NULL) { // check if req_key in cache; yes - error: duplicate req_key violation, no - check db
    pthread_mutex_unlock(&cache_mutex);
    printf("%s\n", "error: duplicate req_key violation");
    *ret_buffer = calloc(MAX_ENTRY_SIZE, sizeof(char *));
    strcpy(*ret_buffer, "DUPLICATE");
    *ret_size = 9;
    return EXIT_FAILURE;
  }else{
    pthread_mutex_unlock(&cache_mutex);
  }
  pthread_mutex_lock(&c0_mutex);
  if(Get(_T, key) != NULL){
    pthread_mutex_unlock(&c0_mutex);
    printf("%s\n", "error: duplicate req_key violation");
    *ret_buffer = calloc(MAX_ENTRY_SIZE, sizeof(char *));
    strcpy(*ret_buffer, "DUPLICATE");
    *ret_size = 9;
    return EXIT_FAILURE;
  }
  _T = Insert(_T, key, value,0);
  if (c0_size(_T) == MAX_C0_SIZE){
    c0_node * c0_batch = _T;
    _T = NULL;
    // pthread_mutex_unlock(&c0_mutex);
    c0_dump(c0_batch);
    pthread_mutex_unlock(&c0_mutex);
  }else{
    pthread_mutex_unlock(&c0_mutex);
  }
  pthread_mutex_lock(&cache_mutex);
  cache_put(key, value);
  pthread_mutex_unlock(&cache_mutex);
  *ret_buffer = calloc(MAX_ENTRY_SIZE, sizeof(char *));
  strcpy(*ret_buffer, "SUCCESS");
  *ret_size = 7;
  return EXIT_SUCCESS;
}

int server_1_delete_request(char *key, char **ret_buffer, int *ret_size) {
  pthread_mutex_lock(&c0_mutex);
  c0_node * look_up = Get(_T, key);
  if(look_up != NULL)
  _T = Update(_T, look_up->key, look_up->value, 1);
  else
  _T = Insert(_T, key, "invalid_delete", 1);
  pthread_mutex_unlock(&c0_mutex);
  *ret_buffer = calloc(MAX_ENTRY_SIZE, sizeof(char *));
  strcpy(*ret_buffer, "SUCCESS");
  *ret_size = 7;
  pthread_mutex_lock(&cache_mutex);
  cache_invalidate(key);
  pthread_mutex_unlock(&cache_mutex);
  return EXIT_SUCCESS;

}

void *setup_sigs_and_exec_handler(void *arg) {
    sigset_t mask1;
    sigemptyset(&mask1);
    sigaddset(&mask1, SIG); // block this always
    pthread_sigmask(SIG_BLOCK, &mask1, NULL);

    server_handler(arg);
}

void server_handler(void *arg) {
  int sockfd = *(int *) arg;
  char *input_line = (char *) calloc(MAX_ENTRY_SIZE, sizeof(char));
  char *copy_input_line = (char *) calloc(MAX_ENTRY_SIZE, sizeof(char));
  char *tokens, *response = NULL,  *save_ptr;
  int response_size;
  transaction txn;
  // int re = 0;
  // while (re = read(sockfd, input_line, MAX_ENTRY_SIZE)) {
  while (read(sockfd, input_line, MAX_ENTRY_SIZE) > 0) { // stop if connection closed by peer
    // db_connect();
    strncpy(copy_input_line, input_line, MAX_ENTRY_SIZE);
    tokens = strtok_r(input_line, " ", &save_ptr);
    char *key = strtok_r(NULL, " ", &save_ptr);
    char *value = strtok_r(NULL, " ", &save_ptr);
    if (tokens == NULL || key == NULL) {
      printf("Invalid key/command received (server-part-1)\n");
      printf("errno: %s\n", strerror(errno)); // "errno: Connection reset by peer"
      write(sockfd, "BAD BOI", 8);
    } else if (strncmp(tokens, "GET", 3) == 0) {
      server_1_get_request(key, &response, &response_size);
      write(sockfd, response, (size_t) response_size);
    } else if (strncmp(tokens, "PUT", 3) == 0) {
      txn.db.data = copy_input_line;
      txn.db.data_len = strlen(copy_input_line);
      while (log_transaction(&txn) != 0);
      server_1_put_request(key, value, &response, &response_size);
      write(sockfd, response, (size_t) response_size);
    } else if (strncmp(tokens, "INSERT", 6) == 0) {
      txn.db.data = copy_input_line;
      txn.db.data_len = strlen(copy_input_line);
      while (log_transaction(&txn) != 0);
      server_1_insert_request(key, value, &response, &response_size);
      write(sockfd, "OK", 2);
    } else if (strncmp(tokens, "DELETE", 6) == 0) {
      txn.db.data = copy_input_line;
      txn.db.data_len = strlen(copy_input_line);
      while (log_transaction(&txn) != 0);
      server_1_delete_request(key, &response, &response_size);
      write(sockfd, response, (size_t) response_size);
    } else {
      write(sockfd, "ERROR", 6);
    }
    // db_cleanup();
    if (response != NULL) {
      free(response);
    }
    free(input_line);
    free(copy_input_line);
    input_line = NULL;
    input_line = (char *) calloc(MAX_ENTRY_SIZE, sizeof(char *));
    copy_input_line = (char *) calloc(MAX_ENTRY_SIZE, sizeof(char *));
    response = NULL;
  }
  free(arg);
  free(input_line);
  free(copy_input_line);
  input_line = NULL;
  close(sockfd);
  return NULL;
}

int create_server_1() {
  int server_fd, opt;

  // Creating socket file descriptor
  if (( server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
    perror("socket failed");
    exit(EXIT_FAILURE);
  }

  // Forcefully attaching socket to the port 8080
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                 &opt, sizeof(opt))) {
    perror("setsockopt");
    exit(EXIT_FAILURE);
  }
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(PORT);

  // Forcefully attaching socket to the port 8080
  if (bind(server_fd, (struct sockaddr *) &address,
           sizeof(address)) < 0) {
    perror("bind failed");
    exit(EXIT_FAILURE);
  }

  return server_fd;
}

int loop_and_listen_1() {
  int sock_fd = create_server_1(), *new_sock;
  if (listen(sock_fd, QUEUED_CONNECTIONS) != 0) {
    perror("listen failed");
    return EXIT_FAILURE;
  }

  while (1) {
    socklen_t cli_addr_size = sizeof(address);
    int newsockfd = accept(sock_fd, (struct sockaddr *) &address, &cli_addr_size);
    printf("Got new connection\n");
    if (newsockfd < 0) {
      perror("Could not accept connection");
      continue;
    }
    new_sock = malloc(4);
    *new_sock = newsockfd;
    pthread_t *handler_thread = (pthread_t *) malloc(sizeof(pthread_t));
    if (pthread_create(handler_thread, NULL, setup_sigs_and_exec_handler, (void *) new_sock) != 0) {
      perror("Could not start handler");
      continue;
    }
  }
}

int run_server_1() {
  // Load database
  head = tail = temp_node = NULL;
  // _T = NULL;
  pthread_mutex_init(&c0_mutex, 0);
  pthread_mutex_init(&cache_mutex, 0);
  // db_init();

  timer_t timerid;
  struct sigevent sev;
  struct itimerspec its;
  struct sigaction sa;

  /* Establish handler for timer signal */
  sa.sa_flags = SA_SIGINFO;
  sa.sa_sigaction = my_timer_handler;
  sigemptyset(&sa.sa_mask); // empty set
  sigaddset(&sa.sa_mask, SIG); // block SIG during handler
  if (sigaction(SIG, &sa, NULL) == -1)
      errExit("sigaction");

  /* Create the timer */
  sev.sigev_notify = SIGEV_SIGNAL;
  sev.sigev_signo = SIG;
  sev.sigev_value.sival_ptr = &timerid;
  if (timer_create(CLOCKID, &sev, &timerid) == -1)
      errExit("timer_create");

  /* Start the timer */
  its.it_value.tv_sec = TIMER_FREQ_S;//freq_nanosecs / 1000000000; Each 65 seconds flush c0
  its.it_value.tv_nsec = 0;//freq_nanosecs % 1000000000;
  its.it_interval.tv_sec = its.it_value.tv_sec;
  its.it_interval.tv_nsec = its.it_value.tv_nsec;
  if (timer_settime(timerid, 0, &its, NULL) == -1)
       errExit("timer_settime");

  c1_init();
  
  if (loop_and_listen_1()) {
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
