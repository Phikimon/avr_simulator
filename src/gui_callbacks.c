#include "gui.h"
#include "common.h"
#include "threads.h"
#include "gui_pins.h"

#include <assert.h>
#include <string.h>

#include <gtk/gtk.h>
#include <cairo.h>
#include <glib.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#pragma GCC diagnostic ignored "-Wunused-function"

void on_combo_box_changed(GtkWidget* box, GtkImage* attinys)
{
    const gchar* src_name  = gtk_widget_get_name(box);
    const gchar* dest_name = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(box));

    (void)pins_connect(src_name, dest_name);

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
    for (int i = 0; i < ATTINY_PINS_NUM; i++) {
        int src_num = i, dest_num = simulator.pins_connections[i];
        if (dest_num == PIN_NC) {
            continue;
        }
        cairo_move_to (cr, pins_pos[src_num].x , pins_pos[src_num].y);
        cairo_line_to (cr, pins_pos[dest_num].x, pins_pos[dest_num].y);
    }

    cairo_stroke(cr);
    cairo_surface_destroy(s);
}

void on_step_pressed(GtkWidget* button, GtkEntry* step_num_widget)
{
    printf_gui(simulator.window, "lala");
    return;

    const gchar* text = gtk_entry_get_text(step_num_widget);
    long int step_num = -1;
    int bytes_read = 0;
    sscanf(text, "%ld%n", &step_num, &bytes_read);
    if (bytes_read != strlen((const char*)text)) {
        printf_gui(simulator.window, "Invalid step value \"%s\"", text);
    } else {
        simulator_step(step_num);
    }
}

void on_window_main_destroy(void)
{
    gtk_main_quit();
}
