#include "gui.h"
#include "common.h"
#include "threads.h"
#include <errno.h>
#include <assert.h>

static GtkWidget* gui_create_window(void);

GtkWidget* gui_configure(int argc, char *argv[])
{
    gtk_init(&argc, &argv);

    GtkWidget* window = gui_create_window();
    gtk_widget_show (window);

    return window;
}

GtkWidget* gui_find_widget_child(GtkWidget* parent, const gchar* name)
{
    assert(parent);
    assert(name);
    const gchar* parent_name = gtk_widget_get_name((GtkWidget*)parent);
    if (!g_ascii_strcasecmp(parent_name, name))
    {
        return parent;
    }

    if (GTK_IS_BIN(parent)) {
        GtkWidget *child = gtk_bin_get_child(GTK_BIN(parent));
        return gui_find_widget_child(child, name);
    }

    if (GTK_IS_CONTAINER(parent)) {
        GList *children = gtk_container_get_children(GTK_CONTAINER(parent));
        do {
            GtkWidget* widget = gui_find_widget_child(children->data, name);
            if (widget != NULL) {
                return widget;
            }
        } while ((children = g_list_next(children)) != NULL);
    }

    return NULL;
}

void
__attribute__((format(printf,2,3)))
printf_gui(GtkWindow* window, const char* format, ...)
{
    char buf[MAX_STR_LEN] = {'\0'};

    va_list args;
    va_start(args,format);
    vsprintf(buf, format, args);
    va_end(args);

    GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;
    GtkWidget* dialog = gtk_message_dialog_new (GTK_WINDOW(window),
                                     flags,
                                     GTK_MESSAGE_ERROR,
                                     GTK_BUTTONS_CLOSE,
                                     buf);
    gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy (dialog);
}

static GtkWidget* gui_create_window(void)
{
    GError* error = NULL;

    GtkBuilder* builder = gtk_builder_new();
    if (!gtk_builder_add_from_file(builder, "./resources/gui_layout.glade", &error))
    {
        g_critical("Cannot open file: %s", error->message);
        g_error_free(error);
    }

    gtk_builder_connect_signals(builder, NULL);

    GtkWidget* window = GTK_WIDGET(gtk_builder_get_object(builder, "window_main"));
    if (!window)
    {
        g_critical("Cannot get window widget");
    }
    g_object_unref(builder);

    return window;
}

void gui_dump_memory(void)
{
    for (int i = 0; i < CHIPS_NUM; i++) {
        // Get text buffer
        char text_buf_name[MAX_STR_LEN] = "Memory_X";
        /*
            0 1 2 3 4 5 6 7
            V V V V V V V V
           "M e m o r y _ X"
         */
        text_buf_name[7] = i + 1;
        GtkTextBuffer* text_buf = (GtkTextBuffer*)gui_find_widget_child(GTK_WIDGET(simulator.window),
                                                                        text_buf_name);
        // Get memory content

        // Set text
        ///(void)gtk_text_buffer_set_text(text_buf, data_memory, DATA_MEMORY_SIZE);


    }
}
