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

enum pin_state {
    STATE_HI_Z = 0,

    STATE_WEAK_LOW,
    STATE_WEAK_HIGH,

    STATE_STRONG_HIGH,
    STATE_STRONG_LOW,
};

enum {
    STRENGTH_HI_Z = 0,
    STRENGTH_WEAK,
    STRENGTH_STRONG,
};

#define IS_STRONG(state)                  \
        ( (state == STATE_STRONG_HIGH) || \
          (state == STATE_STRONG_LOW)  )

#define IS_WEAK(state)                  \
        ( (state == STATE_WEAK_HIGH) || \
          (state == STATE_WEAK_LOW)  )

#define IS_HI_Z(state)          \
          (state == STATE_HI_Z)

#define STRENGTH(state) \
        ( IS_STRONG(state) ? STRENGTH_STRONG :                \
         (IS_WEAK  (state) ? STRENGTH_WEAK   : STRENGTH_HI_Z) )

#define IS_HIGH(state)                    \
        ( (state == STATE_STRONG_HIGH) || \
          (state == STATE_WEAK_HIGH)    )

#define IS_LOW(state)                     \
        ( (state == STATE_STRONG_LOW) || \
          (state == STATE_WEAK_LOW)    )

static enum pin_state get_attiny_pin_state(int pin)
{
    enum pin_state state = STATE_HI_Z;

    int chip_num = pin / ONE_CHIP_PINS_NUM;
    int pin_num  = pin % ONE_CHIP_PINS_NUM;
    int PORTBn  = !!(simulator.chips[chip_num]->PORTB & _BV(pin_num));
    int DDRBn   = !!(simulator.chips[chip_num]->DDRB  & _BV(pin_num));
    int PUD_val = !!(simulator.chips[chip_num]->MCUCR & _BV(PUD));
    if ((DDRBn == 0) && (PORTBn == 0)) {
        state = STATE_HI_Z;
    } else
    if ((DDRBn == 0) && (PORTBn == 1) && (PUD_val == 0)) {
        state = STATE_WEAK_HIGH;
    } else
    if ((DDRBn == 0) && (PORTBn == 1) && (PUD_val == 1)) {
        state = STATE_HI_Z;
    } else
    if ((DDRBn == 1) && (PORTBn == 0)) {
        state = STATE_STRONG_LOW;
    } else
    if ((DDRBn == 1) && (PORTBn == 1)) {
        state = STATE_STRONG_HIGH;
    }

    return state;
}

static enum pin_state get_pin_state(int pin)
{
    enum pin_state state = STATE_HI_Z;

    if (IS_ATTINY_PIN(pin)) {
        get_attiny_pin_state(pin);
    } else
    if (pin == PIN_VCC) {
        state = STATE_STRONG_HIGH;
    } else
    if (pin == PIN_GND) {
        state = STATE_STRONG_LOW;
    }

    return state;
}

int gui_refresh_pins_connections(void)
{
    for (int dock_pin = PIN_DOCK_0;
         dock_pin < PIN_DOCK_0 + DOCK_PINS_NUM;
         dock_pin++) {

        //First pass - find dominating state on wire
        enum pin_state global_state = STATE_HI_Z;
        enum pin strongest_pin_num  = PIN_DEFAULT;

        for (int pin = 0; pin < PINS_NUM; pin++) {
            if (!(simulator.pins_conn_mask[dock_pin] & _BV(pin)))
                continue;

            enum pin_state state = get_pin_state(pin);

            if (STRENGTH(state) > STRENGTH(global_state)) {
                global_state = state;
                strongest_pin_num = pin;
            } else
            if ((STRENGTH(state) == STRENGTH(global_state)) &&
                (STRENGTH(state) >= STRENGTH_WEAK         )  ) {
                printf_gui(simulator.window,
                           "Contradiction: pins '%1$s' and '%2$s' both "
                           "have strength '%3$s', but '%1$s' has state "
                           "'%4$s' and '%2$s' has state '%5$s'. Amend "
                           "mistake and try again.",
                           get_pin_name_by_num(pin),
                           get_pin_name_by_num(strongest_pin_num),
                           (STRENGTH(state) == STRENGTH_STRONG) ? "STRONG" : "WEAK",
                           IS_HIGH(state)        ? "HIGH" : "LOW",
                           IS_HIGH(global_state) ? "HIGH" : "LOW");
                return -1;
            }
        }

        // Which value is finally dominant?
        // Is it determined?
#ifndef NON_DETERMINED_HI_Z
        int is_high = 0;
#else
        int is_high;
#endif
        if (IS_HI_Z(global_state)) {
            // Leave garbage
            is_high = !!is_high;
        } else {
            is_high = IS_HIGH(global_state);
        }

        //Second pass - set PINBn values
        for (int pin = 0; pin < ATTINY_PINS_NUM; pin++) {
            if (!(simulator.pins_conn_mask[dock_pin] & _BV(pin)))
                continue;

            int chip_num = pin / ONE_CHIP_PINS_NUM;
            int pin_num  = pin % ONE_CHIP_PINS_NUM;
            if (is_high) {
                simulator.chips[chip_num]->PINB |= _BV(pin_num);
            } else {
                simulator.chips[chip_num]->PINB &= ~_BV(pin_num);
            }
        }
    }
    return 0;
}

#undef IS_STRONG
#undef IS_WEAK
#undef IS_HI_Z
#undef STRENGTH
#undef IS_HIGH
#undef IS_LOW
