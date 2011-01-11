##/*
 #* page.h
 #* 
 #* author: Piotr Dobrowolski
 #* 4.01.2011r.
 #*
##*/
##MAKEFILE DO ZMIANY, TEN JEST DO TESTOWANIA
CC = gcc
CFLAGS = -Wall -c
LDFLAGS = -Wall
DEBUG=
ifeq ($(debug), 1)
	DEBUG=-g
endif
CFLAGF += $(DEBUG)
LDFLAGS += $(DEBUG)

ALLTESTS = simpleTest
OBJECTS = pagesim.o strategy.o

all: $(ALLTESTS)

fifo: strategyFIFO.c strategy.h
	$(CC) $(CFLAGS) strategyFIFO.c -o strategy.o
lfu: strategyLFU.c strategy.h
	$(CC) $(CFLAGS) strategyLFU.c -o strategy.o

pagesim.o: pagesim.c pagesim.h
	$(CC) $(CFLAGS) pagesim.c -o pagesim.o

$(ALLTESTS)FIFO : $(ALLTESTS).c page.h pagesim.o fifo
	$(CC) $(LDFLAGS) pagesim.o strategy.o $(ALLTESTS).c -o %$(ALLTESTS) : %FIFO
$(ALLTESTS)LFU : $(ALLTESTS).c page.h pagesim.o lfu
	$(CC) $(LDFLAGS) pagesim.o strategy.o $(ALLTESTS).c -o %$(ALLTESTS) : %LFU
$(ALLTESTS) : % : %FIFO % : %LFU
clean:
	rm -f *.o $(ALLTESTS) *~
