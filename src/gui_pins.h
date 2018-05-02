#ifndef GUI_PINS_H
#define GUI_PINS_H
#include "gui.h"

typedef struct {
    gint x, y;
} GtkPoint;

enum pins {
    PIN_DEFAULT = -1,

#define PIN(NAME, X_COORD, Y_COORD) \
    PIN_##NAME,
#include "gui_pins_positions.h"
#undef PIN
    PIN_NC,
    PINS_NUM,

    NOT_ATTINY_PINS_NUM = 3,
    ATTINY_PINS_NUM = PINS_NUM - NOT_ATTINY_PINS_NUM
};

struct gui_pin {
    GtkPoint point;
    int num;
};

extern const GtkPoint pins_pos[PINS_NUM];

int get_pin_num_by_name(const char* pin_name,
                        GtkWindow* window);

#endif
