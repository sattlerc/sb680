#include <errno.h>
#include <error.h>
#include <stdio.h>
#include <stdlib.h>

#include <libevdev/libevdev.h>

#include "adhoc-internal.h"

struct sb880_state {
  int x, y;
  int buttons[4];
  int pen;
  uint32_t colour;
};

void sb880_update_colour(struct sb880_state *state) {
  uint32_t mask = state->buttons[0] ? 0x7f : 0xff;
  uint32_t r = state->buttons[1] ? mask : 0;
  uint32_t g = state->buttons[2] ? mask : 0;
  uint32_t b = state->buttons[3] ? mask : 0;
  state->colour = (r << 16) | (g << 8) | b;
}

int sb880_init(struct adhoc *adhoc) {
  struct sb880_state *state = calloc(1, sizeof(struct sb880_state));
  if (state == NULL) {
    perror("sb880_init");
    return errno;
  }

  adhoc->state = state;
  return 0;
}

void sb880_exit(struct adhoc *adhoc) {
  free(adhoc->state);
}

int sb880_parse(struct adhoc *adhoc) {
  struct sb880_state *state = adhoc->state;

  int x = -1, y = -1, touch = -1;
  int buttons[4] = {-1, -1, -1, -1};
  int pens[3];
  int finger = -1, rubber = -1, pen = -1;
  
  for (;;) {
    struct input_event event;
    int r = next_event_wrapper(adhoc, &event);
    if (r != LIBEVDEV_READ_STATUS_SUCCESS) {
      perror("libevdev_next_event failed");
      return r;
    }
    
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
        pens[0] = event.value;
        break;

      case BTN_1:
        pens[1] = event.value;
        break;

      case BTN_2:
        pens[2] = event.value;
        break;

      case BTN_3:
        buttons[0] = event.value;
        break;

      case BTN_4:
        buttons[1] = event.value;
        break;

      case BTN_5:
        buttons[2] = event.value;
        break;

      case BTN_6:
        buttons[3] = event.value;
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

    report_mouse(adhoc, touch, (state->x + .5) / 4096, (state->y + .5) / 4096);
  }

  if (buttons[0] != -1 || buttons[1] != -1 || buttons[2] != -1 || buttons[3] != -1) {
    for (int i = 0; i != 4; ++i)
      if (buttons[i] != -1)
        state->buttons[i] = buttons[i];

    if (buttons[0] == 1 || buttons[1] == 1 || buttons[2] == 1 || buttons[3] == 1) {
      sb880_update_colour(state);
      if (state->pen)
        report_pen(adhoc, state->colour);
    }
  }

  if (pens[0] != -1 || pens[1] != -1 || pens[2] != -1) {
    if (pen != -1)
      state->pen = pen;

    if (finger == 1)
      report_finger(adhoc);

    if (rubber == 1)
      report_rubber(adhoc);

    if (pen == 1)
      report_pen(adhoc, state->colour);
  }
  
  return 0;
}

struct internal_calls internal_calls_sb880 = {
  .init  = sb880_init,
  .exit  = sb880_exit,
  .parse = sb880_parse
};
