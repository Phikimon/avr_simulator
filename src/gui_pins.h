#ifndef GUI_PINS_H
#define GUI_PINS_H
#include "gui.h"

typedef struct {
    gint x, y;
} GtkPoint;

enum pin {
    PIN_DEFAULT = -1,

#define PIN(NAME, X_COORD, Y_COORD) \
    PIN_##NAME,
#include "gui_pins_positions.h"
#undef PIN
    PINS_NUM,
    PIN_NC,

    ONE_CHIP_PINS_NUM = 6,

    DOCK_PINS_NUM = ONE_CHIP_PINS_NUM,
    NOT_ATTINY_PINS_NUM = 3 + DOCK_PINS_NUM,
    ATTINY_PINS_NUM = PINS_NUM - NOT_ATTINY_PINS_NUM + 1
};

#define IS_DOCK_PIN(pin)     \
    ( (pin == PIN_DOCK_0) || \
      (pin == PIN_DOCK_1) || \
      (pin == PIN_DOCK_2) || \
      (pin == PIN_DOCK_3) || \
      (pin == PIN_DOCK_4) || \
      (pin == PIN_DOCK_5) )

#define IS_ATTINY_PIN(pin)    \
    ( (pin == PIN_F_PB0)   || \
      (pin == PIN_F_PB1)   || \
      (pin == PIN_F_PB2)   || \
      (pin == PIN_F_PB3)   || \
      (pin == PIN_F_PB4)   || \
      (pin == PIN_F_RESET) || \
                              \
      (pin == PIN_S_PB0)   || \
      (pin == PIN_S_PB1)   || \
      (pin == PIN_S_PB2)   || \
      (pin == PIN_S_PB3)   || \
      (pin == PIN_S_PB4)   || \
      (pin == PIN_S_RESET) )

struct gui_pin {
    GtkPoint point;
    int num;
};

extern const GtkPoint pins_pos[PINS_NUM];

void gui_refresh_pins_states(void);

int get_pin_num_by_name(const char* pin_name);

const char* get_pin_name_by_num(int num);

int gui_refresh_pins_connections(void);

#endif
