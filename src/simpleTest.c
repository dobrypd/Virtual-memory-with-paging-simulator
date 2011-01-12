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
   page_sim_init(4/*rozmiar ramki*/, 8/*ilość pamięci (ilość ramek)*/,
                 16/*przestrzeń adresowa*/, 4/*max ilość jednoczesnych operacji I/O*/,
                 &callback);
   uint8_t v;
   /*page_sim_set(1, 5);
   page_sim_set(2, 12);
   page_sim_set(3, 234);
   page_sim_set(4, 1234);
   page_sim_set(5, 23553);
   page_sim_set(6, 223421);
   page_sim_get(1, &v);
   printf("[1]v=%d\n", v);
   page_sim_get(7, &v);
   printf("[7]v=%d\n", v);
   page_sim_get(112, &v);
   printf("[112]v=%d\n", &v);
   */
#define MAX 8*4
   printf("Pętle : set od 1 do MAX wartościami od 1 do MAX i później get na nich\n");
   uint8_t i  =0;
   for(i = 1; i <= MAX; ++i){
      printf("zapisuję %d\n", i-1);
      page_sim_set(i-1, i);
   }
   printf("zapisałem\nodczytuję od końca...");
   for(i = MAX; i > 0; i--){
      page_sim_get(i-1, &v);
      printf("Odczytano [%d] = %u\n", i-1, v);
   }
   printf("koniec\n");
   page_sim_end();
   return 0;
}
