#include "gui.h"
#include "common.h"
#include "threads.h"
#include "gui_pins.h"

#include <assert.h>

#include <gtk/gtk.h>
#include <cairo.h>
#include <glib.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#pragma GCC diagnostic ignored "-Wunused-function"

void on_combo_box_changed(GtkWidget* box, GtkImage* attinys)
{
    const gchar* src_name  = gtk_widget_get_name(box);
    const gchar* dest_name = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(box));

    int src_pin_num  = get_pin_num_by_name(src_name,  simulator.window);
    int dest_pin_num = get_pin_num_by_name(dest_name, simulator.window);
    if ((src_pin_num  == PIN_DEFAULT) ||
        (dest_pin_num == PIN_DEFAULT)) {
        return;
    }

    simulator.pins_connections[src_pin_num] = dest_pin_num;
    GtkWidget* draw_area;
    draw_area = gui_find_widget_child(GTK_WIDGET(simulator.window), "DRAW_AREA");
    assert(draw_area);
    gtk_widget_queue_draw(GTK_WIDGET(draw_area));
}

gboolean draw(GtkWidget *widget, cairo_t *cr, gpointer data)
{
    static cairo_t* cr_saved = NULL;
    if ((cr_saved != cr) && (cr)) {
        cr_saved = cr;
    }

    GdkPixbuf* pix = gdk_pixbuf_new_from_file("./resources/attinys.png", NULL);
    cairo_surface_t* s = gdk_cairo_surface_create_from_pixbuf(pix, 0, NULL);
    cairo_set_source_surface (cr_saved, s, 0, 0);
    cairo_paint (cr_saved);

    cairo_set_source_rgba (cr, 0, 0, 0, 1);
    for (int i = 0; i < ATTINY_PINS_NUM; i++) {
        int src_num = i, dest_num = simulator.pins_connections[i];
        if (dest_num == PIN_NC) {
            continue;
        }
        cairo_move_to (cr_saved, pins_pos[src_num].x , pins_pos[src_num].y);
        cairo_line_to (cr_saved, pins_pos[dest_num].x, pins_pos[dest_num].y);
    }

    cairo_stroke(cr_saved);
    cairo_surface_destroy(s);

    return FALSE;
}

void on_window_main_destroy(void)
{
    gtk_main_quit();
}

