#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#include "adhoc.h"


int main(int num_args, char **args) {
  int fd;
  struct adhoc *adhoc;

  fd = open_unique_device(num_args, args);
  adhoc = adhoc_init(fd);
  if (adhoc == NULL) {
    fprintf(stderr, "could not initialize adhoc library\n");
    goto error_init;
  }

  {
    printf("press ctrl+d to end collection\n");

    struct adhoc_callbacks print_callbacks = {
      .mouse_down    = [&](void *, float x, float y) {
        printf("%f %f\n", x, y);
      },
      .mouse_up      = nullptr,
      .mouse_move    = nullptr,
      .select_finger = nullptr, 
      .select_rubber = nullptr,
      .select_pen    = nullptr
    };
    
    adhoc_set_callbacks(adhoc, &print_callbacks);
    
    for (;;) {
      pollfd fds[2];
      fds[0].fd = 0;
      fds[0].events = POLLIN;
      fds[1].fd = adhoc_get_fd(adhoc);
      fds[1].events = POLLIN;

      if (poll(fds, 2, -1) < 0) {
        perror("poll failed");
        break;
      }
        
      if (fds[0].revents & POLLIN)
        break;
      
      if (fds[1].revents & POLLIN) {
        int r = adhoc_parse(adhoc);
        if (r) {
          fprintf(stderr, "parse failed\n");
          goto error;
        }
      }
    }
  }

 error:
  adhoc_exit(adhoc);
 error_init:
  close(fd);
  return -1;
}
