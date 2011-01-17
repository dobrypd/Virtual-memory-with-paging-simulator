##/*
#* Makefile
#* 
#* author: Piotr Dobrowolski
#* 17.01.2011r.
#*
##*/
CC=gcc
CFLAGS=-Wall -Werror -pthread -fPIC -lrt
LDFLAGS=-Wall -Werror -lrt -pthread
LD_LIBRARY_PATH=.
DEBUG=
ifeq ($(debug), 1)
DEBUG=-g
endif
CFLAGS += $(DEBUG)
LDFLAGS += $(DEBUG)

LIBNAME=pagesim
CCOBJS=pagesim.o err.o
COBJS=$(CCOBJS) strategyFIFO.o strategyLFU.o

all: fifo

fifo: $(CCOBJS) strategyFIFO.o
	$(CC) $(LDFLAGS) -shared -Wl,-soname,lib$(LIBNAME).so -o lib$(LIBNAME).so $(CCOBJS) strategyFIFO.o

lfu: $(CCOBJS) strategyLFU.o
	$(CC) $(LDFLAGS) -shared -Wl,-soname,$(LIBNAME) -o lib$(LIBNAME).so $(CCOBJS) strategyLFU.o

$(COBJS): %.o: %.c
	$(CC) $(CFLAGS) -c $<

test: simpleTest.c
	$(CC) $(LDFLAGS) simpleTest.c -o test -L. -l$(LIBNAME)

clean:
	rm -f *.o *~