//
// Created by sps5394 on 10/18/18.
//
#include "server-part1.h"
#include "c0.h"
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>

#define CLOCKID CLOCK_REALTIME
#define SIG SIGRTMIN
#define C0_SIZE_TRSH 50
#define errExit(msg)    do { perror(msg); exit(EXIT_FAILURE); \
                        } while (0)
pthread_mutex_t c0_mutex;
c0_node * T;

static void
my_timer_handler(int sig, siginfo_t *si, void *uc)
{
  pthread_mutex_lock(&c0_mutex);
  c0_dump(T);
  T = NULL;
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
  cache_lookup = cache_get(key);
  if (cache_lookup != NULL) {
    // HIT
    *ret_buffer = calloc(MAX_ENTRY_SIZE, sizeof(char *));
    strcpy(*ret_buffer, cache_lookup->defn);
    *ret_size = (int) strlen(*ret_buffer);
    return 0;
  }
  // Perform IO
  if (db_get(key, ret_buffer, ret_size)) {
    return EXIT_FAILURE;
  }
  if (strcmp(*ret_buffer, "NOTFOUND") != 0) { // if result was NULL there was some kind of error
    cache_put(key, *ret_buffer);
  }
  return EXIT_SUCCESS;
}

int server_1_put_request(char *key, char *value, char **ret_buffer, int *ret_size) {
  pthread_mutex_lock(&c0_mutex);
  Update(T, key, value);
  pthread_mutex_unlock(&c0_mutex);
  return EXIT_SUCCESS;
}

int server_1_insert_request(char *key, char *value, char **ret_buffer, int *ret_size) {
  pthread_mutex_lock(&c0_mutex);
  printf("ATA: size %d\n", c0_size(T));
  T = Insert(T, key, value);
  // printf("Modri ret: %p and size: %d\n",T , c0_size(T));
  // inorder(T);

  if (c0_size(T) == C0_SIZE_TRSH){
    c0_dump(T);
    T = NULL;
  }
    pthread_mutex_unlock(&c0_mutex);

  return EXIT_SUCCESS;
}

int server_1_delete_request(char *key, char **ret_buffer, int *ret_size) {
  pthread_mutex_lock(&c0_mutex);
  if(Get(T, key) != NULL)
  T = Delete(T, key);
  pthread_mutex_unlock(&c0_mutex);
  return EXIT_SUCCESS;
}

void *server_handler(void *arg) {
  int sockfd = *(int *) arg;
  char *input_line = (char *) calloc(MAX_ENTRY_SIZE, sizeof(char *));
  char *tokens, *response = NULL, *key, *value, *save_ptr;
  int response_size;
  while (read(sockfd, input_line, MAX_ENTRY_SIZE)) {
    db_connect();
    tokens = strtok_r(input_line, " ", &save_ptr);
    key = strtok_r(NULL, " ", &save_ptr);
    value = strtok_r(NULL, " ", &save_ptr);
    if (tokens == NULL || key == NULL) {
      printf("Invalid key/command received\n");
      write(sockfd, "BAD BOI", 8);
    } else if (strncmp(tokens, "GET", 3) == 0) {
      server_1_get_request(key, &response, &response_size);
      write(sockfd, response, (size_t) response_size);
    } else if (strncmp(tokens, "PUT", 3) == 0) {
      server_1_put_request(key, value, &response, &response_size);
      write(sockfd, response, (size_t) response_size);
    } else if (strncmp(tokens, "INSERT", 6) == 0) {
      server_1_insert_request(key, value, &response, &response_size);
      write(sockfd, "OK", 2);
    } else if (strncmp(tokens, "DELETE", 6) == 0) {
      server_1_delete_request(key, &response, &response_size);
      write(sockfd, response, (size_t) response_size);
    } else {
      write(sockfd, "ERROR", 6);
    }
    db_cleanup();
    if (response != NULL) {
      free(response);
    }
    free(input_line);
    input_line = NULL;
    input_line = (char *) calloc(MAX_ENTRY_SIZE, sizeof(char *));
    response = NULL;
  }
  free(arg);
  free(input_line);
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
    if (pthread_create(handler_thread, NULL, server_handler, (void *) new_sock) != 0) {
      perror("Could not start handler");
      continue;
    }
  }
}

int run_server_1() {
  // Load database
  head = tail = temp_node = NULL;
  T = NULL;
  pthread_mutex_init(&c0_mutex, 0);
  db_init();

  timer_t timerid;
  struct sigevent sev;
  struct itimerspec its;
  long long freq_nanosecs;
  sigset_t mask;
  struct sigaction sa;

  /* Establish handler for timer signal */
  sa.sa_flags = SA_SIGINFO;
  sa.sa_sigaction = my_timer_handler;
  sigemptyset(&sa.sa_mask);
  if (sigaction(SIG, &sa, NULL) == -1)
      errExit("sigaction");

  /* Create the timer */
  sev.sigev_notify = SIGEV_SIGNAL;
  sev.sigev_signo = SIG;
  sev.sigev_value.sival_ptr = &timerid;
  if (timer_create(CLOCKID, &sev, &timerid) == -1)
      errExit("timer_create");

  /* Start the timer */
  its.it_value.tv_sec = 65;//freq_nanosecs / 1000000000; Each 65 seconds flush c0
  its.it_value.tv_nsec = 0;//freq_nanosecs % 1000000000;
  its.it_interval.tv_sec = its.it_value.tv_sec;
  its.it_interval.tv_nsec = its.it_value.tv_nsec;
  if (timer_settime(timerid, 0, &its, NULL) == -1)
       errExit("timer_settime");

  if (loop_and_listen_1()) {
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
