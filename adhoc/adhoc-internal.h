#ifndef ADHOC_INTERNAL_H
#define ADHOC_INTERNAL_H

#include "adhoc.h"

struct internal_calls {
  int (*init)(struct adhoc *adhoc);
  void (*exit)(struct adhoc *adhoc);
  int (*parse)(struct adhoc *adhoc);
};

struct adhoc {
  struct libevdev *dev;
  struct adhoc_callbacks callbacks;
  struct internal_calls internal_calls;
  void *state;
};

int next_event_wrapper(struct adhoc *adhoc, struct input_event *event);

void report_mouse(const struct adhoc *adhoc, int touch_mod, float x, float y);
void report_finger(const struct adhoc *adhoc);
void report_rubber(const struct adhoc *adhoc);
void report_pen(const struct adhoc *adhoc, uint32_t colour);

extern struct internal_calls internal_calls_sb680;
extern struct internal_calls internal_calls_sb885;

#endif
