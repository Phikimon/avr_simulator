#ifndef GUI_H_INCLUDED
#define GUI_H_INCLUDED

#include <gtk/gtk.h>

// Public
GtkWidget* gui_configure(int argc, char *argv[]);
void gui_dump_memory(void);
void gui_dump_registers(void);
void gui_obj_dump(void);
GtkWidget* gui_find_widget_child(GtkWidget* parent, const gchar* name);
void printf_gui(GtkWindow* window, const char* format, ...)
__attribute__((format(printf,2,3)));
void gui_obj_dump_update_line(void);
void gui_increase_global_tick(void);

#endif
