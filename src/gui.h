#ifndef GUI_H_INCLUDED
#define GUI_H_INCLUDED

#include <gtk/gtk.h>

GtkWidget* create_window(void);

GtkWidget* find_child   (GtkWidget* parent, const gchar* name);

void configure_gui(int argc, char *argv[]);

#endif
