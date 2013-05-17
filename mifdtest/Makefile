# Makefile for utilities

CC      = cc
CFLAGS  = -g -w
INCLUDE = -I/usr/local/include
LIBS    = -L/lib -lpcsclite -lpthread
OBJS = mifdtest.o


all: mifdtest

clean:
	rm -f  mifdtest  *.o
mifdtest : $(OBJS)
	$(CC) $(CFLAGS) -o mifdtest ${OBJS} $(INCLUDE) ${LIBS}
%.o : %.c
	$(CC) $(CFLAGS) -c $< $(INCLUDE)
