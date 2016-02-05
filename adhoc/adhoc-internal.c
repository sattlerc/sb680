#include <error.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libevdev/libevdev.h>

#include "adhoc-internal.h"

struct adhoc* adhoc_init(int fd) {
  struct adhoc *adhoc = calloc(1, sizeof(struct adhoc));
  int r = libevdev_new_from_fd(fd, &adhoc->dev);
  if (r) {
    error(0, -r, "libevdev_new_from_fd failed");
    goto creation_error;
  }

  const char *name = libevdev_get_name(adhoc->dev);
  if (strcmp(name, "SMART SB680 Interactive Whiteboard") == 0)
    adhoc->internal_calls = internal_calls_sb680;
  else if (strcmp(name, "SMART SB880 Interactive Whiteboard") == 0)
    adhoc->internal_calls = internal_calls_sb880;
  else {
    fprintf(stderr, "unknown device: %s\n", name);
    goto device_error;
  }

#ifdef DEBUG
  printf("device: %s\n", name);
#endif
  
  if (adhoc->internal_calls.init(adhoc))
    goto device_error;

  return adhoc;

 device_error:
  libevdev_free(adhoc->dev);
  
 creation_error:
  free(adhoc);
  return NULL;
}

void adhoc_exit(struct adhoc *adhoc) {
  adhoc->internal_calls.exit(adhoc);
  libevdev_free(adhoc->dev);
  free(adhoc);
}

int adhoc_get_fd(const struct adhoc *adhoc) {
  return libevdev_get_fd(adhoc->dev);
}

void adhoc_set_callbacks(struct adhoc *adhoc, const struct adhoc_callbacks *callbacks) {
  adhoc->callbacks = *callbacks;
}

int adhoc_parse(struct adhoc *adhoc) {
  return adhoc->internal_calls.parse(adhoc);
}


int next_event_wrapper(struct adhoc *adhoc, struct input_event *event) {
  int r = libevdev_next_event(adhoc->dev, LIBEVDEV_READ_FLAG_NORMAL | LIBEVDEV_READ_FLAG_BLOCKING, event);
  if (r < 0) {
    error(0, -r, "libevdev_next_event failed");
    return r;
  }

  if (r == LIBEVDEV_READ_STATUS_SYNC) {
    fprintf(stderr, "libevdev_next_event returned LIBEVDEV_READ_STATUS_SYNC, handling not implemented!\n");
    return 0;
  }

#ifdef DEBUG
  printf("event [type: %s, code: %s, value: %d]\n",
         libevdev_event_type_get_name(event->type),
         libevdev_event_code_get_name(event->type, event->code),
         event->value);
#endif

  return 0;
}

void report_mouse(const struct adhoc *adhoc, int touch_mod, float x, float y) {
  void (*callback)(float x, float y) = NULL;
  
  switch (touch_mod) {
  case -1:
    callback = adhoc->callbacks.mouse_move;
    break;

  case 0:
    callback = adhoc->callbacks.mouse_up;
    break;

  case 1:
    callback = adhoc->callbacks.mouse_down;
    break;
  }

  if (callback != NULL)
    callback(x, y);
}

void report_finger(const struct adhoc *adhoc) {
  if (adhoc->callbacks.select_finger != NULL)
    adhoc->callbacks.select_finger();
}

void report_rubber(const struct adhoc *adhoc) {
  if (adhoc->callbacks.select_rubber != NULL)
    adhoc->callbacks.select_rubber();
}

void report_pen(const struct adhoc *adhoc, uint32_t colour) {
  if (adhoc->callbacks.select_pen != NULL)
    adhoc->callbacks.select_pen(colour);
}
