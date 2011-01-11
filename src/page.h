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
   /*modyficated bit * 2^0 & valid/invalid *2^1 (invalid - nie znajduje się w pamięci)*/
   long long counter;
   /*licznik odwołań (FIFO)/ */
   uint8_t** frameAddr;
   /*tablica 2wymiarowa (ramka * komórka w ramce)*/
} page

#endif
