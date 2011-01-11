/*
 * page.h
 * 
 * author: Piotr Dobrowolski
 * 4.01.2011r.
 *
*/

#ifndef __PAGE_H
#define __PAGE_H

typedef struct page {
   unsigned char properties; 
   /*modificated bit * 2^0 & valid/invalid *2^1 (invalid - nie znajduje się w pamięci)*/
   unsigned long long counter;
   /*licznik odwołań LFU, FIFO: wpisywana kolejna liczba*/
   uint8_t* frameAddr;
} page

#endif
