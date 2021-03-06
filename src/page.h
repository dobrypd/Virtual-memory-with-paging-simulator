/*
 * page.h
 * 
 * author: Piotr Dobrowolski
 * 4.01.2011r.
 *
*/

#ifndef __PAGE_H
#define __PAGE_H

#include <stdint.h>

#define MBIT (1)
#define VBIT (2)
#define UBIT (4)

#define MPAGE(X) (((X).properties) & MBIT)
#define VPAGE(X) (((X).properties) & VBIT)


typedef struct page {
   unsigned char properties; 
   /*modificated bit * 2^0 & valid/invalid *2^1 (invalid - nie znajduje się w pamięci)*/
   /*in use bit 2^2 - gdy strona jest w traknie uzycia*/
   unsigned long long counter;
      /*zamiast tego można zastosować wskaźnik i utworzyć listę
         * w przypadku FIFO kolejka
         * w przypadku LFU będzie przepisywany dany adres 'na górę'
      */
   /*page* next;*/
   uint8_t* frame;
} page;

#endif
