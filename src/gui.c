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
                                   "font-size: 10;"
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
        GtkTextView* text_view = (GtkTextView*)gui_find_widget_child(GTK_WIDGET(simulator.window),
                                                                     text_view_name);
        assert(text_view);
        GtkTextBuffer* text_buf = gtk_text_view_get_buffer(text_view);
        assert(text_buf);
        // Get memory content

        const int LINE_LENGTH = sizeof("FFFFFFFFFFFFFFFF\n") - sizeof((char)'\0');
        const int TEXT_SIZE = LINE_LENGTH * 17 + sizeof((char)'\0'); // 8 for each representation + 1 for separation line
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
        GtkTextView* text_view = (GtkTextView*)gui_find_widget_child(GTK_WIDGET(simulator.window),
                                                                     text_view_name);
        assert(text_view);
        GtkTextBuffer* text_buf = gtk_text_view_get_buffer(text_view);
        assert(text_buf);

        // Get registers content
        const int LINE_LENGTH = sizeof("RRRRRR = FF;   RRRRRR = FF;\n") - sizeof((char)'\0');
        const int LINES_NUM = (REGISTERS_NUM + NOT_RESERVED_IO_REGS_NUM + 1) / 2 + 1;   // 2 registers in one line + 1 separation line
        const int TEXT_SIZE = LINE_LENGTH * LINES_NUM;
        char registers_text[TEXT_SIZE];
        int k;  // Lines counter
        for (k = 0; k < REGISTERS_NUM / 2; k++)
            sprintf(registers_text + k * LINE_LENGTH, "R%-5d = %02hX;   R%-5d = %02hX;\n",
                    k,                     (uint8_t)simulator.chips[i]->registers[k],
                    k + REGISTERS_NUM / 2, (uint8_t)simulator.chips[i]->registers[k + REGISTERS_NUM / 2]);

        // Separation line
        memset(registers_text + k * LINE_LENGTH, '_', LINE_LENGTH - 1);
        k++;
        registers_text[k * LINE_LENGTH - 1] = '\n';

        // Get io_registers content
        int len = 0;

#define RESERVED_REGISTER(OFFSET)
#define IO_REGISTER(NAME, OFFSET)                                                         \
        if (len == 0)                                                                     \
            len = sprintf(registers_text + k * LINE_LENGTH, "%-6s = %02hX;",              \
                           #NAME, simulator.chips[i]->io_registers[OFFSET]);              \
        else {                                                                            \
            sprintf(registers_text + k * LINE_LENGTH + len, "   %-6s = %02hX;\n",         \
                                #NAME, simulator.chips[i]->io_registers[OFFSET]);         \
            k++;                                                                          \
            len = 0;                                                                      \
        }
#include "registers_list.h"
#undef RESERVED_REGISTER
#undef IO_REGISTER

        sprintf(registers_text + k * LINE_LENGTH + len, "   %-6s = %02hX;",
                            "PC", simulator.chips[i]->PC);
        k++;

        // Set text
        (void)gtk_text_buffer_set_text(text_buf, registers_text, TEXT_SIZE - 1);
    }
}

void gui_obj_dump(void)
{
    for (int i = 0; i < CHIPS_NUM; i++) {
        // Get text buffer
        char text_view_name[MAX_STR_LEN] = "Disasm_X";
        text_view_name[7] = '0' + i + 1;
        GtkTextView* text_view = (GtkTextView*)gui_find_widget_child(GTK_WIDGET(simulator.window),
                                                                     text_view_name);
        assert(text_view);
        GtkTextBuffer* text_buf = gtk_text_view_get_buffer(text_view);
        assert(text_buf);

        // Get text
        char obj_dump_file_name[MAX_STR_LEN] = "tests/objdump_X";
        obj_dump_file_name[14] =  '0' + i + 1;
        FILE* obj_dump_file = fopen(obj_dump_file_name, "r");
        assert(obj_dump_file);
        fseek(obj_dump_file, 0, SEEK_END);
        int file_size = ftell(obj_dump_file);
        rewind(obj_dump_file);
        char* text = (char*)calloc(file_size, sizeof(char));
        assert(text);
        fread(text, sizeof(char), file_size, obj_dump_file);
        fclose(obj_dump_file);

        // Set text
        (void)gtk_text_buffer_set_text(text_buf, text, file_size);
    }
}
