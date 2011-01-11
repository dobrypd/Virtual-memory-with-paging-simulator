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
#define MAXPAGESIZE 612
#define MINMEMSIZE 1
#define MAXMEMSIZE 64
#define MINADDRSPACESIZE 1
#define MAXADDRSPACESIZE 512

/*init library*/
typedef void (*pagesim_callback)(int op, int arg1, int arg2);

typedef struct {
   /*page simulator parameters*/
   unsigned page_size;
   unsigned mem_size;
   unsigned addr_space_size;
   unsigned max_concurrent_operations;
   pagesim_callback callback;
   
   unsigned offsetMask;
   unsigned pagenrMask;
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
