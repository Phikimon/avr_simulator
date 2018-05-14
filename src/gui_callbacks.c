#include "gui.h"
#include "common.h"
#include "threads.h"
#include "gui_pins.h"

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

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

void on_step_pressed(GtkWidget* button, GtkSpinButton* step_num_widget)
{
    gint step_num = gtk_spin_button_get_value_as_int(step_num_widget);
    if (simulator.is_inited == 0) {
        simulator_init();
        simulator.is_inited = 1;
    }
    simulator_step(step_num);
}

void disasm_file_set(GtkFileChooserButton* button, GtkTextBuffer* text_buf)
{

    GtkWidget *dialog;
    dialog = gtk_file_chooser_dialog_new (gtk_widget_get_name(GTK_WIDGET(button)),
                                          simulator.window,
                                          GTK_FILE_CHOOSER_ACTION_OPEN,
                                          "Cancel",
                                          GTK_RESPONSE_CANCEL,
                                          "Open",
                                          GTK_RESPONSE_ACCEPT,
                                          NULL);

    gint res = gtk_dialog_run (GTK_DIALOG (dialog));
    if (res != GTK_RESPONSE_ACCEPT)
        return;
    GtkFileChooser *chooser = GTK_FILE_CHOOSER (dialog);
    char* filename = gtk_file_chooser_get_filename (chooser);
    assert(filename);
    gtk_widget_destroy (dialog);

    struct stat buf = {0};
    int stat_ret, close_ret, fd, bytes_read;
    char* text;

    stat_ret = stat(filename, &buf);
    if ((stat_ret == -1) || (buf.st_size < 1)) {
        perror("stat");
        goto critical;
    }
    fd = open(filename, O_RDONLY);
    if (stat_ret == -1) {
        perror("open");
        goto critical;
    }
    text = (char*)calloc(buf.st_size + sizeof((char)'\0'),
                         sizeof(char));
    if (!text) {
        perror("calloc");
        goto close_fd;
    }
    bytes_read = read(fd, text, buf.st_size);
    if (bytes_read != buf.st_size) {
        perror("read");
        goto free_text;
    }
    close_ret = close(fd);
    if (close_ret == -1) {
        perror("close");
        free(text);
        goto critical;
    }
    (void)gtk_text_buffer_set_text(text_buf, text, buf.st_size);

    free(text);
    g_free (filename);
    return;

free_text:
    free(text);
close_fd:
    close(fd);
critical:
    g_free (filename);
    exit(1);
}

void on_window_main_destroy(void)
{
    gtk_main_quit();
    exit(0);
}
