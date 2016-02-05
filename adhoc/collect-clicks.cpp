#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "adhoc.h"


int main(int num_args, char **args) {
  struct adhoc *adhoc;
  int fd;
  
  if (num_args < 2) {
    fprintf(stderr, "Usage: <program> /dev/input/event<X>\n");
    goto error_out;
  }
  
  fd = open(args[1], O_RDONLY);
  if (fd == -1) {
    perror("could not open path");
    goto error_out;
  }

  adhoc = adhoc_init(fd);
  if (adhoc == NULL) {
    fprintf(stderr, "could not initialize adhoc library\n");
    goto error_init;
  }

  {
    struct adhoc_callbacks print_callbacks = {
      .mouse_down    = [&](float x, float y) {
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
      int r = adhoc_parse(adhoc);
      if (r) {
        fprintf(stderr, "parse failed\n");
        goto error;
      }
    }
  }

 error:
  adhoc_exit(adhoc);
 error_init:
  close(fd);
 error_out:
  return -1;
}
