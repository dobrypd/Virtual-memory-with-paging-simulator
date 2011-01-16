/*
 * pagesim.h
 * 
 * author: Piotr Dobrowolski
 * 4.01.2011r.
 *
*/

#ifndef __PAGESIM_H
#define __PAGESIM_H

#include <stdint.h>

#define MINPAGESIZE 4
#define MAXPAGESIZE 512
#define MINMEMSIZE 1
#define MAXMEMSIZE 64
#define MINADDRSPACESIZE 1
#define MAXADDRSPACESIZE 512

#define SECURE(X, TEXT)    if ( (X) == -1) syserr(#TEXT"\n")

/*init library*/
typedef void (*pagesim_callback)(int op, int arg1, int arg2);

typedef struct {
   /*page simulator parameters*/
   unsigned char init;
   
   unsigned page_size;
   unsigned mem_size;
   unsigned addr_space_size;
   unsigned max_concurrent_operations;
   pagesim_callback callback;
   
   unsigned char shift;
   unsigned offsetMask;
   
   unsigned fff; 
   /*first free frame - metoda alokowania pamięci - od początku do końca*/
   /*nigdy nie zwalniam pamięci więc tak może być - nie utworzą się dziury*/
   /*pierwsza wolna ramka w pamięci operacyjnej jeżeli alokuje na samym początku*/
} pageSimParam_t;

extern int page_sim_init(unsigned page_size, 
      unsigned mem_size,
      unsigned addr_space_size,
      unsigned max_concurrent_operations,
      pagesim_callback callback);

extern int page_sim_end();

extern int page_sim_get(unsigned a, uint8_t *v);

extern int page_sim_set(unsigned a, uint8_t v);

#endif
