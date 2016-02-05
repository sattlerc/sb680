#include <errno.h>
#include <error.h>
#include <stdio.h>
#include <stdlib.h>

#include <libevdev/libevdev.h>

#include "adhoc-internal.h"

struct sb680_state {
  int x, y;
  int buttons[4];
  int pen;
};

uint32_t sb680_get_colour(const struct sb680_state *state) {
  uint32_t mask = state->buttons[0] ? 0x7f : 0xff;
  uint32_t r = state->buttons[1] ? mask : 0;
  uint32_t g = state->buttons[2] ? mask : 0;
  uint32_t b = state->buttons[3] ? mask : 0;
  return (r << 16) | (g << 8) | b;
}

int sb680_init(struct adhoc *adhoc) {
  struct sb680_state *state = calloc(1, sizeof(struct sb680_state));
  if (state == NULL) {
    perror("sb680_init");
    return errno;
  }

  adhoc->state = state;
  return 0;
}

void sb680_exit(struct adhoc *adhoc) {
  free(adhoc->state);
}

int sb680_parse(struct adhoc *adhoc) {
  struct sb680_state *state = adhoc->state;

  int x = -1, y = -1, touch = -1;
  int buttons[5] = {-1, -1, -1, -1, -1};
  int finger = -1, rubber = -1, pen = -1;
  
  for (;;) {
    struct input_event event;
    int r = next_event_wrapper(adhoc, &event);
    if (r)
      return r;
    
    if (event.type == EV_SYN)
      break;

    if (event.type == EV_ABS)
      switch (event.code) {
      case ABS_X:
        x = event.value;
        break;

      case ABS_Y:
        y = event.value;
        break;
      }
    else if (event.type == EV_KEY)
      switch (event.code) {
      case BTN_TOUCH:
        touch = event.value;
        break;
        
      case BTN_0:
        buttons[0] = event.value;
        break;

      case BTN_1:
        buttons[1] = event.value;
        break;

      case BTN_2:
        buttons[2] = event.value;
        break;

      case BTN_3:
        buttons[3] = event.value;
        break;

      case BTN_4:
        buttons[4] = event.value;
        break;

      case BTN_TOOL_FINGER:
        finger = event.value;
        break;

      case BTN_TOOL_RUBBER:
        rubber = event.value;
        break;

      case BTN_TOOL_PEN:
        pen = event.value;
        break;
      }
  }

  if (x != -1 || y != -1 || touch != -1) {
    if (x != -1)
      state->x = x;
    if (y != -1)
      state->y = y;

    report_mouse(adhoc, touch, (state->x + .5) / 2048, (state->y + .5) / 2048);
  }

  if (buttons[0] != -1 || buttons[1] != -1 || buttons[2] != -1 || buttons[3] != -1 || buttons[4] != -1) {
    for (int i = 0; i != 4; ++i)
      if (buttons[i] != -1)
        state->buttons[i] = buttons[i];

    if (pen != -1)
      state->pen = pen;

    if (finger == 1)
      report_finger(adhoc);

    if (rubber == 1)
      report_rubber(adhoc);

    if (state->pen)
      report_pen(adhoc, sb680_get_colour(state));
  }

  return 0;
}

struct internal_calls internal_calls_sb680 = {
  .init  = sb680_init,
  .exit  = sb680_exit,
  .parse = sb680_parse
};
