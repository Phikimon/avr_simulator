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
    ATTINY_PINS_NUM = PINS_NUM - NOT_ATTINY_PINS_NUM
};

#define IS_DOCK_PIN(pin)     \
    ( (pin == PIN_DOCK_0) || \
      (pin == PIN_DOCK_1) || \
      (pin == PIN_DOCK_2) || \
      (pin == PIN_DOCK_3) || \
      (pin == PIN_DOCK_4) || \
      (pin == PIN_DOCK_5) )

struct gui_pin {
    GtkPoint point;
    int num;
};

extern const GtkPoint pins_pos[PINS_NUM];

void gui_refresh_pins_states(void);

int get_pin_num_by_name(const char* pin_name);

const char* get_pin_name_by_num(int num);


#endif
