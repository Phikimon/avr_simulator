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
    ATTINY_PINS_NUM = PINS_NUM - NOT_ATTINY_PINS_NUM,

    ONE_CHIP_PINS_NUM = 6
};

struct gui_pin {
    GtkPoint point;
    int num;
};

extern const GtkPoint pins_pos[PINS_NUM];

void pins_connect(const gchar* src_name,
                  const gchar* dest_name);

void gui_refresh_pins_states(void);

#endif
