CC=gcc
CFLAGS=-pthread -I$(IDIR) -g -O0
LIBS=-lm
ODIR=obj
SDIR=src
IDIR=include

_DEPS = *.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ1 = main_uart.o terminal.o uart.o queue.o	
OBJ1 = $(patsubst %,$(ODIR)/%,$(_OBJ1))

_OBJ2 = main_server.o terminal.o server.o queue.o
OBJ2 = $(patsubst %,$(ODIR)/%,$(_OBJ2))


$(ODIR)/%.o: $(SDIR)/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: bin/uart bin/server
	echo "Done making"

bin/uart: $(OBJ1)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

bin/server: $(OBJ2)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)


.PHONY: clean all

clean:
	rm -f $(ODIR)/*.o
	rm -f bin/*
