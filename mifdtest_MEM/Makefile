# Makefile for utilities

CC      = cc
CFLAGS  = -g -w
#INCLUDE = -I/usr/local/include
INCLUDE = -I/usr/include/PCSC
LIBS    = -L/usr/local/lib -lpcsclite -lpthread
OBJS = mifdtest.o


all: mifdtest

clean:
	rm -f  mifdtest  *.o
mifdtest : $(OBJS)
	$(CC) $(CFLAGS) -o mifdtest ${OBJS} $(INCLUDE) ${LIBS}
%.o : %.c
	$(CC) $(CFLAGS) -c $< $(INCLUDE)
