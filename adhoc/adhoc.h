#ifndef ADHOC_H
#define ADHOC_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct adhoc;

struct adhoc_callbacks {
  void (*mouse_down)   (void *data, float x, float y);
  void (*mouse_up)     (void *data, float x, float y);
  void (*mouse_move)   (void *data, float x, float y);
  void (*select_finger)(void *data);
  void (*select_rubber)(void *data);
  void (*select_pen)   (void *data, uint32_t colour);
};

struct adhoc* adhoc_init(int fd);
void adhoc_exit(struct adhoc *adhoc);

int adhoc_get_fd(const struct adhoc *adhoc);
double adhoc_get_aspect_ratio(struct adhoc *adhoc);
void adhoc_set_callbacks(struct adhoc *adhoc, const struct adhoc_callbacks *callbacks);
void adhoc_set_data(struct adhoc *adhoc, void *data);

int adhoc_parse(struct adhoc *adhoc);

#define ADHOC_BUFFER_SIZE 256

int scan_devices(char devices[][ADHOC_BUFFER_SIZE], char names[][ADHOC_BUFFER_SIZE], int max_devices);
  int select_unique_device(char *device, char *name);
int open_unique_device(int num_args, char** args);

#ifdef __cplusplus
}
#endif

#endif
