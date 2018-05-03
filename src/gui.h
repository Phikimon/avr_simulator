#ifndef GUI_H_INCLUDED
#define GUI_H_INCLUDED

#include <gtk/gtk.h>

// Public
enum { SRAM_TEXT_SIZE = 17 * 17 };

GtkWidget* gui_configure(int argc, char *argv[]);
void gui_dump_memory(void);
GtkWidget* gui_find_widget_child(GtkWidget* parent, const gchar* name);
void printf_gui(GtkWindow* window, const char* format, ...)
__attribute__((format(printf,2,3)));


#endif
