#include "gui.h"
#include "common.h"
#include "threads.h"
#include "gui_pins.h"

#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include <gtk/gtk.h>
#include <cairo.h>
#include <glib.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#pragma GCC diagnostic ignored "-Wunused-function"

void on_pin_clicked(GtkWidget* pin)
{
    enum pin pin_num = get_pin_num_by_name(gtk_widget_get_name(pin));

    static enum pin old_pin_num = PIN_DEFAULT;

    int same_pin = (old_pin_num == pin_num);
    int one_pin_is_dock = IS_DOCK_PIN(old_pin_num) ^ IS_DOCK_PIN(pin_num);
    int ok = (!same_pin) && (one_pin_is_dock) && (old_pin_num != PIN_DEFAULT);

    if (!ok) {
        old_pin_num = pin_num;
        return;
    }

    enum pin src_pin, dock_pin;
    if (IS_DOCK_PIN(old_pin_num)) {
        dock_pin = old_pin_num;
        src_pin  = pin_num;
    } else {
        dock_pin = pin_num;
        src_pin  = old_pin_num;
    }

    if (simulator.pins_conn_mask[dock_pin] & _BV(src_pin)) {
        simulator.pins_conn_mask[dock_pin] &= ~_BV(src_pin);
    } else {
        for (int i = PIN_DOCK_0; i < PIN_DOCK_0 + DOCK_PINS_NUM; i++) {
            simulator.pins_conn_mask[i] &= ~_BV(src_pin);
        }
        simulator.pins_conn_mask[dock_pin] |= _BV(src_pin);
    }
    gui_refresh_pins_states();

    old_pin_num = PIN_DEFAULT;
    //Force redraw
    GtkWidget* draw_area = gui_find_widget_child(GTK_WIDGET(simulator.window),
                                                 "DRAW_AREA");
    assert(draw_area);
    gtk_widget_queue_draw(GTK_WIDGET(draw_area));
}

void on_draw_area_draw(GtkWidget *widget, cairo_t *cr, gpointer data)
{
    GdkPixbuf* pix = gdk_pixbuf_new_from_file("./resources/attinys.png", NULL);
    cairo_surface_t* s = gdk_cairo_surface_create_from_pixbuf(pix, 0, NULL);
    cairo_set_source_surface (cr, s, 0, 0);
    cairo_paint (cr);

    cairo_set_source_rgba (cr, 0, 0, 0, 1);
    for (int i = PIN_DOCK_0; i < PIN_DOCK_0 + DOCK_PINS_NUM; i++) {
        for (int j = 0; j < PINS_NUM; j++) {
            if (!(simulator.pins_conn_mask[i] & _BV(j)))
                continue;

            int src_num = i, dest_num = j;
            cairo_move_to (cr, pins_pos[src_num].x , pins_pos[src_num].y);
            cairo_line_to (cr, pins_pos[dest_num].x, pins_pos[dest_num].y);
        }
    }

    cairo_stroke(cr);
    cairo_surface_destroy(s);
}

void on_step_pressed(GtkWidget* button, GtkEntry* step_num_widget)
{
    const gchar* text = gtk_entry_get_text(step_num_widget);
    long int step_num = -1;
    int bytes_read = -1;
    sscanf(text, "%ld%n", &step_num, &bytes_read);
    step_num = 1;  ///
    if (bytes_read != strlen((const char*)text)) {
#ifdef STRICT
        printf_gui(simulator.window, "Invalid step value \"%s\"", text);
        return;
#else
        step_num = 1;
#endif
    }
    if (simulator.is_inited == 0) {
        simulator_init();
        simulator.is_inited = 1;
    }
    simulator_step(step_num);
}

void on_window_main_destroy(void)
{
    gtk_main_quit();
    exit(0);
}
