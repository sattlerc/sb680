#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "adhoc.h"


void print_mouse_down(float x, float y) {
  printf("mouse_down [x: %f1.4, y: %f1.4]\n", x, y);
}

void print_mouse_up(float x, float y) {
  printf("mouse_up [x: %f1.4, y: %f1.4]\n", x, y);
}

void print_mouse_move(float x, float y) {
  printf("mouse_move [x: %f1.4, y: %f1.4]\n", x, y);
}

void print_select_finger(void) {
  printf("select_finger\n");
}

void print_select_rubber(void) {
  printf("select_rubber\n");
}

void print_select_pen(uint32_t colour) {
  printf("select_pen [colour: %06x]\n", colour);
}

struct adhoc_callbacks print_callbacks = {
  .mouse_down    = print_mouse_down,
  .mouse_up      = print_mouse_up,
  .mouse_move    = print_mouse_move,
  .select_finger = print_select_finger, 
  .select_rubber = print_select_rubber,
  .select_pen    = print_select_pen
};


int main(int num_args, char **args) {
  int fd;
  struct adhoc *adhoc;

  fd = open_unique_device(num_args, args);
  adhoc = adhoc_init(fd);
  if (adhoc == NULL) {
    fprintf(stderr, "could not initialize adhoc library\n");
    goto error_init;
  }

  adhoc_set_callbacks(adhoc, &print_callbacks);
  
  for (;;) {
    int r = adhoc_parse(adhoc);
    if (r) {
      fprintf(stderr, "parse failed\n");
      goto error;
    }
  }

 error:
  adhoc_exit(adhoc);
 error_init:
  close(fd);
  return -1;
}
