#include "gui.h"

void configure_gui(int argc, char *argv[])
{
    gtk_init(&argc, &argv);

    GtkWidget* window = create_window();
    gtk_widget_show (window);

    GtkWidget* PB3_LED = find_child(window, "F_PB3_LED");
    if (!PB3_LED)
    {
        g_critical("Cannot get PB3_LED widget");
    }

    gtk_image_set_from_icon_name((GtkImage*)PB3_LED, "go-bottom", GTK_ICON_SIZE_BUTTON);

    gtk_main();
}

GtkWidget* find_child(GtkWidget* parent, const gchar* name)
{
    const gchar* parent_name = gtk_widget_get_name((GtkWidget*)parent);
    if (!g_ascii_strcasecmp(parent_name, name))
    {
        return parent;
    }

    if (GTK_IS_BIN(parent)) {
        GtkWidget *child = gtk_bin_get_child(GTK_BIN(parent));
        return find_child(child, name);
    }

    if (GTK_IS_CONTAINER(parent)) {
        GList *children = gtk_container_get_children(GTK_CONTAINER(parent));
        do {
            GtkWidget* widget = find_child(children->data, name);
            if (widget != NULL) {
                return widget;
            }
        } while ((children = g_list_next(children)) != NULL);
    }

    return NULL;
}

GtkWidget* create_window(void)
{
    GError* error = NULL;

    GtkBuilder* builder = gtk_builder_new();
    if (!gtk_builder_add_from_file(builder, PATH_TO_GLADE, &error))
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
