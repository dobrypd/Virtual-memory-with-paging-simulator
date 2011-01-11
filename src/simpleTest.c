/*
 * simpleTest.c
 * 
 * author: Piotr Dobrowolski
 * 4.01.2011r.
 *
*/
#include<stdio.h>

#include "page.h"
#include "pagesim.h"
void callback(int op, int arg1, int arg2)
{
   printf("op=%d\targ1=%d\targ2=%d\n", op, arg1, arg2);
}
int main(){
   page_sim_init(64/*rozmiar ramki*/, 8/*ilość pamięci (ilość ramek)*/,
                 16/*przestrzeń adresowa*/, 4/*max ilość jednoczesnych operacji I/O*/,
                 &callback);
   page_sim_set(1, 5);
   page_sim_set(2, 12);
   page_sim_set(3, 234);
   page_sim_set(4, 1234);
   page_sim_set(5, 23553);
   page_sim_set(6, 223421);
   uint8_t v;
   page_sim_get(1, &v);
   printf("[1]v=%d\n", v);
   page_sim_get(7, &v);
   printf("[7]v=%d\n", v);
   page_sim_get(112, &v);
   printf("[112]v=%d\n", &v);
   
   page_sim_end();
   return 0;
}
