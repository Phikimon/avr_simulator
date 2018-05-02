#include "gui_pins.h"
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

int get_pin_num_by_name(const char* pin_name,
                        GtkWindow* window)
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
    printf_gui(window, "Unknown pin");
    return PIN_DEFAULT;
}
