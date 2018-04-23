CC=gcc
LD=gcc
GTKFLAGS=-export-dynamic `pkg-config --cflags --libs gtk+-3.0`
CFLAGS=-DPATH_TO_GLADE="\"./resources/gui_layout.glade\"" -I$(DEPDIR) -O0 -Wall -pedantic -Wno-format $(GTKFLAGS)
LFLAGS=$(GTKFLAGS)
LIBS=-pthread
SRCDIR=src

all: tags main

DEPDIR = src
_SRCS  = gui_callbacks.c gui.c attiny13.c main.c
_DEPS = gui_callbacks.h gui.h attiny13.h common.h decode.h
SRCS = $(patsubst %,$(SRCDIR)/%,$(_SRCS))
DEPS = $(patsubst %,$(DEPDIR)/%,$(_DEPS))

OBJDIR=obj
_OBJ = main.o gui.o gui_callbacks.o attiny13.o
OBJ = $(patsubst %,$(OBJDIR)/%,$(_OBJ))

$(OBJDIR):
	mkdir $(OBJDIR) || true

obj/%.o: src/%.c $(OBJDIR) $(DEPS)
	$(CC) $(CFLAGS) -c $< -o $@

main: $(OBJ)
	$(LD) $^ $(LFLAGS) $(LIBS) -o $@

clean:
	rm -rf $(OBJDIR) || true
	rm tags main || true

tags: $(SRCS) $(DEPS)
	ctags -R
