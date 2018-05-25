#include "gui.h"
#include "common.h"
#include "threads.h"

#include <errno.h>
#include <assert.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

static GtkWidget* gui_create_window(void);

GtkWidget* gui_configure(int argc, char *argv[])
{
    gtk_init(&argc, &argv);

    GtkCssProvider* css = gtk_css_provider_new();
    (void)gtk_css_provider_load_from_data(css,
                          "GtkTextView, textview {"
                                   "font-family: 'MonoSpace', monospace;"
                                   "font-size: 12;"
                          "}", -1, NULL);
    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
                                              GTK_STYLE_PROVIDER(css), 1);

    GtkWidget* window = gui_create_window();
    gtk_widget_show (window);

    return window;
}

GtkWidget* gui_find_widget_child(GtkWidget* parent, const gchar* name)
{
    if ((!name) || (!parent)) return NULL;
    const gchar* parent_name = gtk_widget_get_name((GtkWidget*)parent);
    if (!parent_name) return NULL;
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
        if (!children) return NULL;
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
    char buf[MAX_STR_LEN << 2] = {'\0'};

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
        char text_view_name[MAX_STR_LEN] = "Memory_X";
        /*
            0 1 2 3 4 5 6 7
            V V V V V V V V
           "M e m o r y _ X"
         */
        text_view_name[7] = '0' + i + 1;
        GtkTextView* text_view = (GtkTextView*)gui_find_widget_child(
                                               GTK_WIDGET(simulator.window),
                                               text_view_name);
        assert(text_view);
        GtkTextBuffer* text_buf = gtk_text_view_get_buffer(text_view);
        assert(text_buf);
        // Get memory content

        const int LINE_LENGTH = sizeof("FFFFFFFFFFFFFFFF\n") - sizeof((char)'\0');
        // 8 for each representation + 1 for separation line:
        const int TEXT_SIZE = LINE_LENGTH * 17 + sizeof((char)'\0');
        char sram_text[TEXT_SIZE];
        int k = 0;  // sram_text index
        int cur_sram_byte = 0;
        for (; k < LINE_LENGTH * 8; k++) {   // String representation (first 8 lines)
            if (k % LINE_LENGTH == LINE_LENGTH - 1)
                sram_text[k] = '\n';
            else if (isprint(simulator.chips[i]->sram[cur_sram_byte]))
                sram_text[k] = simulator.chips[i]->sram[cur_sram_byte++];
            else
                sram_text[k] = '.';
        }
        for (; k < 17 * 9 - 1; k++)     // Separation line
            sram_text[k] = '_';
        sram_text[k++] = '\n';
        while (k < TEXT_SIZE - 1) {    // Hex representation (last 8 lines)
            if (k % LINE_LENGTH == LINE_LENGTH - 1)
                sram_text[k++] = '\n';
            else {
                sprintf(sram_text + k, "%02hX", (uint8_t)simulator.chips[i]->sram[cur_sram_byte++]);
                k += 2;
            }
        }
        sram_text[TEXT_SIZE - 1] = ' ';

        // Set text
        (void)gtk_text_buffer_set_text(text_buf, sram_text, TEXT_SIZE);
    }
}

void gui_dump_registers(void)
{
    for (int i = 0; i < CHIPS_NUM; i++) {
        // Get text buffer
        char text_view_name[MAX_STR_LEN] = "Registers_X";
        text_view_name[10] = '0' + i + 1;
        GtkTextView* text_view = (GtkTextView*)gui_find_widget_child(
                                                GTK_WIDGET(simulator.window),
                                                text_view_name);
        assert(text_view);
        GtkScrolledWindow* sw = (GtkScrolledWindow*)gtk_widget_get_parent(
                                                    GTK_WIDGET(text_view));
        GtkTextBuffer* text_buf = gtk_text_view_get_buffer(text_view);
        assert(text_buf);

        // Get registers content
        const int LINE_LENGTH = sizeof("RRRRRR = FF;   RRRRRR = FF;\n") -
                                sizeof((char)'\0');
         // 2 registers in one line + 1 separation line:
        const int LINES_NUM = (REGISTERS_NUM + NOT_RESERVED_IO_REGS_NUM + 1) / 2 + 1;
        const int TEXT_SIZE = LINE_LENGTH * LINES_NUM;
        char registers_text[TEXT_SIZE];
        int k;  // Lines counter
        for (k = 0; k < REGISTERS_NUM / 2; k++)
            sprintf(registers_text + k * LINE_LENGTH,
                    "R%-5d = %02hX;   R%-5d = %02hX;\n",
                    k,
                    (uint8_t)simulator.chips[i]->registers[k],
                    k + REGISTERS_NUM / 2,
                    (uint8_t)simulator.chips[i]->registers[k + REGISTERS_NUM / 2]);

        // Separation line
        memset(registers_text + k * LINE_LENGTH, '_', LINE_LENGTH - 1);
        k++;
        registers_text[k * LINE_LENGTH - 1] = '\n';

        // Get io_registers content
        int len = 0;

#define RESERVED_REGISTER(OFFSET)
#define IO_REGISTER(NAME, OFFSET)                                      \
        if (len == 0)                                                  \
            len = sprintf(registers_text + k * LINE_LENGTH,            \
                    "%-6s = %02hX;",                                   \
                    #NAME, simulator.chips[i]->io_registers[OFFSET]);  \
        else {                                                         \
            sprintf(registers_text + k * LINE_LENGTH + len,            \
                    "   %-6s = %02hX;\n",                              \
                    #NAME, simulator.chips[i]->io_registers[OFFSET]);  \
            k++;                                                       \
            len = 0;                                                   \
        }
#include "registers_list.h"
#undef RESERVED_REGISTER
#undef IO_REGISTER

        sprintf(registers_text + k * LINE_LENGTH + len, "   %-6s = %02hX;",
                            "PC", simulator.chips[i]->PC * 2);
        k++;

        // Set text
        GtkAdjustment* adj = gtk_scrolled_window_get_vadjustment(sw);
        gdouble value = gtk_adjustment_get_value(adj);
        (void)gtk_text_buffer_set_text(text_buf, registers_text, TEXT_SIZE - 1);
        gdouble ceiling = gtk_adjustment_get_upper(adj) -
                          gtk_adjustment_get_page_size(adj);
        gint new_value = (value >= ceiling) ? ceiling - 1: value;
        (void)gtk_adjustment_set_value(adj, new_value);
    }
}

void gui_obj_dump_update_line(void)
{
    static GtkTextBuffer* text_bufs[CHIPS_NUM] = { NULL, NULL };

    if (text_bufs[0] == NULL) {
        GtkTextView* text_view = NULL;
        //                                         01234567
        //                                         VVVVVVVV
        char text_view_name[sizeof("Disasm_X")] = "Disasm_X";
        for (int i = 0; i < CHIPS_NUM; i++) {
            text_view_name[7] = '1' + i;
            text_view = (GtkTextView*)gui_find_widget_child(GTK_WIDGET(simulator.window),
                                                            text_view_name);
            assert(text_view);
            text_bufs[i] = gtk_text_view_get_buffer(text_view);
            assert(text_bufs[i]);

        }
    }

    for (int i = 0; i < CHIPS_NUM; i++) {
        int PC = simulator.chips[i]->PC;
        char needle[MAX_STR_LEN] = {0};
        sprintf(needle, "  %x:", PC * 2);
        GtkTextIter start, end;
        (void)gtk_text_buffer_get_start_iter(text_bufs[i], &start);
        (void)gtk_text_buffer_get_end_iter  (text_bufs[i], &end);
        gchar* haystack = NULL;
        haystack = gtk_text_buffer_get_text(text_bufs[i],
                                            &start,
                                            &end,
                                            FALSE);
        char* needle_in_haystack = strstr(haystack, needle);
        if (!needle_in_haystack) {
            free(haystack);
            continue;
        }
        int index = (long long int)needle_in_haystack -
                    (long long int)haystack;
        int slash_n_sum = 0;
        for (int i = 0; i < index; i++) {
            slash_n_sum += (haystack[i] == '\n');
        }
        index++;
        while (haystack[index] != '\n') {
            index++;
        }
        GtkTextIter line_begin_iter, line_end_iter;
        (void)gtk_text_buffer_get_iter_at_line(text_bufs[i],
                                               &line_begin_iter,
                                               slash_n_sum);
        (void)gtk_text_buffer_get_iter_at_line_offset(text_bufs[i],
                                                      &line_end_iter,
                                                      slash_n_sum,
                                                      index);
        GtkTextTag* line_tag[2] = {NULL};
        if (line_tag[i] == NULL) {
            line_tag[i] = gtk_text_buffer_create_tag(text_bufs[i],
                                                     NULL, //< Anonymous
                                                     "foreground",
                                                     "white",
                                                     "background",
                                                     "black",
                                                     NULL);
        }
        (void)gtk_text_buffer_remove_all_tags(text_bufs[i], &start, &end);
        (void)gtk_text_buffer_apply_tag(text_bufs[i],
                                        line_tag[i],
                                        &line_begin_iter,
                                        &line_end_iter);
        free(haystack);
    }
}
