//
// Created by sps5394 on 10/18/18.
//

#include <stdio.h>
#include <stdlib.h>
#include "server-part1.h"
#include "c0.h"


int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Must provide value for what sort of server to run\n");
    return  1;
  }
  switch(atoi(argv[1])) {
    case 1:
      return run_server_1();
    default:
      printf("Illegal argument value. Exiting\n");
      return 1;
  }
}
