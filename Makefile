CC=gcc
LD=gcc
GTKFLAGS=-export-dynamic `pkg-config --cflags --libs gtk+-3.0`
CFLAGS=-I$(DEPDIR) -O0 -Wall -Wno-format $(GTKFLAGS)
LFLAGS=$(GTKFLAGS)
LIBS=-pthread
SRCDIR=src

DEPDIR = src
_DEPS = attiny13.h gui.h gui_pins.h gui_pins_positions.h \
        common.h decode.h registers_list.h threads.h
_SRCS = attiny13.c gui.c gui_pins.c gui_callbacks.c main.c do_functions.c threads.c
SRCS = $(patsubst %,$(SRCDIR)/%,$(_SRCS))
DEPS = $(patsubst %,$(DEPDIR)/%,$(_DEPS))

OBJDIR=obj
_OBJ = $(_SRCS:.c=.o)
OBJ = $(patsubst %,$(OBJDIR)/%,$(_OBJ))

all: $(OBJDIR) tags main

$(OBJDIR):
	mkdir $(OBJDIR) || true

obj/%.o: src/%.c $(DEPS)
	$(CC) $(CFLAGS) -c $< -o $@

main: $(OBJ)
	$(LD) $^ $(LFLAGS) $(LIBS) -o $@

clean:
	rm -rf $(OBJDIR) || true
	rm tags main || true

tags: $(SRCS) $(DEPS)
	ctags -R
