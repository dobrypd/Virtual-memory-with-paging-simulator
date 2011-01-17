##/*
 #* page.h
 #* 
 #* author: Piotr Dobrowolski
 #* 4.01.2011r.
 #*
##*/
##MAKEFILE DO ZMIANY, TEN JEST DO TESTOWANIA
CC = gcc
CFLAGS = -Wall -c -pthread
LDFLAGS = -Wall -lrt -pthread
DEBUG=
ifeq ($(debug), 1)
	DEBUG=-g
endif
CFLAGS += $(DEBUG)
LDFLAGS += $(DEBUG)

ALLTESTS = simpleTest
OBJECTS = pagesim.o strategy.o
OTHEROBJECTS = 

fifo: strategyFIFO.c strategy.h
	$(CC) $(CFLAGS) strategyFIFO.c -o strategy.o
lfu: strategyLFU.c strategy.h
	$(CC) $(CFLAGS) strategyLFU.c -o strategy.o

pagesim.o: pagesim.c pagesim.h
	$(CC) $(CFLAGS) pagesim.c -o pagesim.o
err.o: err.c err.h
	$(CC) $(CFLAGS) err.c -o err.o

fifotest: fifo pagesim.o err.o
	$(CC) $(LDFLAGS) simpleTest.c pagesim.o strategy.o err.o -o simpleTest

lfutest: lfu pagesim.o err.o
	$(CC) $(LDFLAGS) simpleTest.c pagesim.o strategy.o err.o -o simpleTest

clean:
	rm -f *.o $(ALLTESTS) *~
