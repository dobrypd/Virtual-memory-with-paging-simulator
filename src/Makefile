##/*
 #* Makefile
 #* 
 #* author: Piotr Dobrowolski
 #* 17.01.2011r.
 #*
##*/
CC=gcc
CFLAGS=-Wall -Werror -c -pthread -fPIC
LDFLAGS=-Wall -Werror -lrt -pthread
DEBUG=
ifeq ($(debug), 1)
	DEBUG=-g
endif
CFLAGS += $(DEBUG)
LDFLAGS += $(DEBUG)

LIBNAME=pagesim.so
CCOBJS=pagesim.o err.o
COBJS=$(CCOBJS) strategyFIFO.o strategyLFU.o

all: fifo

fifo: $(CCOBJS) strategyFIFO.o
	$(CC) $(LDFLAGS) -shared -Wl,-soname,$(LIBNAME) -o $(LIBNAME) $(CCOBJS) strategyFIFO.o

lfu: $(CCOBJS) strategyLFU.o
	$(CC) $(LDFLAGS) -shared -Wl,-soname,$(LIBNAME) -o $(LIBNAME) $(CCOBJS) strategyLFU.o

$(COBJS): %.o: %.c
	$(CC) $(CFLAGS) -c $<

test: simpleTest.c
	$(CC) $(CFLAGS) -o test -L. -lpagesim.so simpleTest.c

clean:
	rm -f *.o *~
