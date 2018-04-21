CC=gcc
LD=gcc
GTKFLAGS=-export-dynamic `pkg-config --cflags --libs gtk+-3.0`
CFLAGS=-DPATH_TO_GLADE="\"./resources/gui_layout.glade\"" -O0 -Wall -pedantic -Wno-format $(GTKFLAGS)
LFLAGS=$(GTKFLAGS)
LIBS=-pthread
DEPS=
SRCDIR=src

all: tags main

tags:
	ctags -R

OBJDIR=obj
_OBJ = main.o
OBJ = $(patsubst %,$(OBJDIR)/%,$(_OBJ))

$(OBJDIR):
	mkdir $(OBJDIR) || true

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(OBJDIR) $(DEPS)
	$(CC) $(CFLAGS) -c $< -o $@

main: $(OBJ)
	$(LD) $(LFLAGS) $(LIBS) $^ -o $@

clean:
	rm -rf $(OBJDIR) || true
	rm tags main || true
