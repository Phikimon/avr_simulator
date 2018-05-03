#include "gui_pins.h"
#include "threads.h"
#include <string.h>

const GtkPoint pins_pos[PINS_NUM] =
{
#define ALL_PINS
#define PIN(NAME, X_COORD, Y_COORD) \
    { .x = X_COORD, .y = Y_COORD },
#include "gui_pins_positions.h"
#undef PIN
#undef ALL_PINS
};

static int get_pin_num_by_name(const char* pin_name)
{
#define ALL_PINS
#define PIN(NAME, X_COORD, Y_COORD)          \
    if (strstr(pin_name, #NAME) == pin_name) \
        return PIN_##NAME;
#include "gui_pins_positions.h"
#undef PIN
#undef ALL_PINS
    if (strstr(pin_name, "NC") == pin_name)
        return PIN_NC;
    printf_gui(simulator.window, "Unknown pin");
    return PIN_DEFAULT;
}

void pins_connect(const gchar* src_name, const gchar* dest_name)
{
    int src_pin_num  = get_pin_num_by_name(src_name);
    int dest_pin_num = get_pin_num_by_name(dest_name);
    simulator.pins_connections[src_pin_num] = dest_pin_num;
}

