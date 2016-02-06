#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#include "adhoc.h"


int main() {
  char device[ADHOC_BUFFER_SIZE], name[ADHOC_BUFFER_SIZE];
  if (select_unique_device(device, name) < 0)
    exit(-1);

  printf("%s\n", device);
  printf("%s\n", name);
  return 0;
}
