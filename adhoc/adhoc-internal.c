#define _GNU_SOURCE

#include <dirent.h>
#include <error.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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
  else if (strcmp(name, "SMART SB885 Interactive Whiteboard") == 0)
    adhoc->internal_calls = internal_calls_sb885;
  else {
    fprintf(stderr, "unknown device: %s\n", name);
    goto device_error;
  }

#ifdef DEBUG
  fprintf(stderr, "device: %s\n", name);
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
  fprintf(stderr,
          "event [type: %s, code: %s, value: %d]\n",
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


#define DEV_INPUT "/dev/input"
#define EVENT "event"

const char *RECOGNIZED_DEVICES[] = {
  "SMART SB680 Interactive Whiteboard",
  "SMART SB685 Interactive Whiteboard",
  "SMART SB885 Interactive Whiteboard"
};

int scan_devices(char devices[][ADHOC_BUFFER_SIZE], char names[][ADHOC_BUFFER_SIZE], int max_devices) {
	struct dirent **entries;
	int num_entries = scandir(DEV_INPUT, &entries, NULL, versionsort);
  if (num_entries < 0)
		return -1;

  int j = 0;
	for (int i = 0; i < num_entries && j < max_devices; i++) {
		int r, fd;

    if (strncmp(EVENT, entries[i]->d_name, strlen(EVENT)))
      continue;

		r = snprintf(devices[j], ADHOC_BUFFER_SIZE, "%s/%s", DEV_INPUT, entries[i]->d_name);
    if (r < 0 || r >= ADHOC_BUFFER_SIZE)
      continue;

    r = 0;
    for (int k = 0; k != sizeof(RECOGNIZED_DEVICES) / sizeof(RECOGNIZED_DEVICES[0]); ++k)
      if (strcmp(devices[j], RECOGNIZED_DEVICES[k]) == 0)
        r = 1;

    if (!r)
      continue;

    fd = open(devices[j], O_RDONLY);
		if (fd < 0)
      continue;

    if (ioctl(fd, EVIOCGNAME(ADHOC_BUFFER_SIZE), names[j]) == -1)
      goto error;

    ++j;
    
  error:
    close(fd);
	}
  
  for (int i = 0; i != num_entries; ++i)
    free(entries[i]);
  
	return j;
}

int select_unique_device(char *device, char *name) {
  char devices[8][ADHOC_BUFFER_SIZE], names[8][ADHOC_BUFFER_SIZE];
  int r = scan_devices(devices, names, 8);
  if (r < 0) {
    perror("select_devices failed");
    return r;
  }

  if (r == 0) {
    fprintf(stderr, "No supported device found! Do you have access right?\n");
    return -1;
  }

  if (r > 1) {
    fprintf(stderr, "Multiple supported devices found! Which one should we open?\n");
    for (int i = 0; i != r; ++i)
      fprintf(stderr, "%s: %s", devices[i], names[i]);

    return -1;
  }

  strcpy(device, devices[0]);
  if (name != NULL)
    strcpy(name, names[0]);
  return 0;
}

int open_unique_device(int num_args, char** args) {
  char device[ADHOC_BUFFER_SIZE];
  char *path;
  int fd;
  
  if (num_args < 2) {
    if (select_unique_device(device, NULL) < 0) {
      fprintf(stderr, "Usage: <program> [/dev/input/event<X>]\n");
      exit(-1);
    }

    path = device;
  } else {
    path = args[1];
  }
  
  fd = open(path, O_RDONLY);
  if (fd == -1) {
    fprintf(stderr, "could not open path %s\n", path);
    exit(-1);
  }

  return fd;
}
