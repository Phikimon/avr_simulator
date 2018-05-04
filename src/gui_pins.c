#include "gui_pins.h"
#include "threads.h"
#include "common.h"
#include <string.h>
#include <stdint.h>
#include <assert.h>

const GtkPoint pins_pos[PINS_NUM] =
{
#define PIN(NAME, X_COORD, Y_COORD) \
    { .x = X_COORD, .y = Y_COORD },
#include "gui_pins_positions.h"
#undef PIN
};

int get_pin_num_by_name(const char* pin_name)
{
#define PIN(NAME, X_COORD, Y_COORD)          \
    if (strstr(pin_name, #NAME) == pin_name) \
        return PIN_##NAME;
#include "gui_pins_positions.h"
#undef PIN
    if (strstr(pin_name, "NC") == pin_name)
        return PIN_NC;
    printf_gui(simulator.window, "Unknown pin");
    return PIN_DEFAULT;
}

const char* get_pin_name_by_num(int num)
{
    switch (num) {
#define PIN(NAME, X_COORD, Y_COORD)          \
        case PIN_##NAME:                     \
            return #NAME;
#include "gui_pins_positions.h"
#undef PIN
    }
    printf_gui(simulator.window, "Unknown pin");
    return NULL;
}

void gui_refresh_pins_states(void)
{
    GtkWidget* layout = gui_find_widget_child(GTK_WIDGET(simulator.window),
                                              "Main_Field");
    char led_name[MAX_STR_LEN] = {0};
    for (int i = 0; i < ATTINY_PINS_NUM; i++) {
        // Find LED
        const char* pin_name = get_pin_name_by_num(i);
        assert(pin_name);
        (void)strncpy(led_name, pin_name, MAX_STR_LEN);
        (void)strcat(led_name, "_LED");
        GtkWidget* led = gui_find_widget_child(layout, led_name);

        // Change its state accordingly
        int chip_num = i / ONE_CHIP_PINS_NUM;
        uint8_t PINB = simulator.chips[chip_num]->PINB;

        int pin_num  = i % ONE_CHIP_PINS_NUM;

        const char* led_picture = (PINB & _BV(pin_num)) ?
                                  "go-top" : "go-bottom";
        gtk_image_set_from_icon_name(GTK_IMAGE(led),
                                     led_picture,
                                     GTK_ICON_SIZE_BUTTON);
    }

}
