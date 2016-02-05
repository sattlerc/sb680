#ifndef ADHOC_H
#define ADHOC_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct adhoc_callbacks {
  void (*mouse_down)(float x, float y);
  void (*mouse_up)(float x, float y);
  void (*mouse_move)(float x, float y);
  void (*select_finger)(void);
  void (*select_rubber)(void);
  void (*select_pen)(uint32_t colour);
};

struct adhoc;

struct adhoc* adhoc_init(int fd);
void adhoc_exit(struct adhoc *adhoc);

int adhoc_get_fd(const struct adhoc *adhoc);
void adhoc_set_callbacks(struct adhoc *adhoc, const struct adhoc_callbacks *callbacks);

int adhoc_parse(struct adhoc *adhoc);

#ifdef __cplusplus
}
#endif

#endif
