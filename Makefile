CC=gcc
LD=gcc
CFLAGS=-O0 -Wall -pedantic -Wno-format
LIBS=-pthread
DEPS=

all: tags main

tags:
	ctags -R

OBJDIR=obj
_OBJ = main.o
OBJ = $(patsubst %,$(OBJDIR)/%,$(_OBJ))

$(OBJDIR):
	mkdir $(OBJDIR) || true

$(OBJDIR)/%.o: %.c $(OBJDIR) $(DEPS)
	$(CC) $(CFLAGS) -c $< -o $@

main: $(OBJ)
	$(LD) $(LFLAGS) $(LIBS) $^ -o $@

clean:
	rm -rf $(OBJDIR) || true
	rm tags main || true
