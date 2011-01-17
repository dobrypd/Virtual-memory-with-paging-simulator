#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <errno.h>

#include "page.h"
#include "pagesim.h"

#define TEST1   0
#define TEST2   0
#define TEST3a  0
#define TEST3b  0
#define TEST4   0
#define TEST5   0
#define TEST6   0
#define TEST7   0
#define TEST8   1

/* globals */
int thread_count;
int err_th, mod_th;
int counter_th;

void callback(int op, int arg1, int arg2)
{
/*   printf("op=%d\targ1=%d\targ2=%d\n", op, arg1, arg2);*/
  return;
}

/* thread fun 1 */
void* thread_fun_set(void *arg)
{
  int i, n;

  for (n = 0; n < counter_th; ++n)
    for (i = 0; i < 512; ++i)
      if (page_sim_set(i, i%mod_th) != 0)
        printf("ERROR: page_sim_set err=%d\n", errno);
       
  --thread_count;
       
  return 0;
}

/* thread fun 2 */
void* thread_fun_get(void *arg)
{
  int i;
  uint8_t v;
  
  for (i = 0; i < counter_th; ++i) {
    int no = (rand()%512);
    if (page_sim_get(no, &v) != 0)
      printf("ERROR: page_sim_get err=%d\n", errno);
    if (v != no%mod_th) {
       printf("ERROR: Debug i=%d no=%d v=%d nomodmod_th=%d\n", i, no, v, no%mod_th);
      err_th = 1;
      //break;
    }
  }  
         
  --thread_count;
       
  return 0;
}

int main()
{
//  page_sim_init(64/*rozmiar ramki*/, 2/*ilość pamięci (ilość ramek)*/,
//                16/*przestrzeń adresowa*/, 4/*max ilość jednoczesnych operacji I/O*/,
//                &callback);
  page_sim_init(64/*rozmiar ramki*/, 2/*ilość pamięci (ilość ramek)*/,
                8/*przestrzeń adresowa*/, 4/*max ilość jednoczesnych operacji I/O*/,
                &callback);

  printf("\nTESTS:\n");
  uint8_t v;
  int i;
  
  if (TEST1) {
    printf("TEST1\n");
    page_sim_set(1, 5);
    page_sim_set(2, 12);
    page_sim_set(2, 12);
    page_sim_set(3, 234);
    printf("END\n\n");
  }

  if (TEST2) { /* swaping simple test */
    printf("TEST2\n"); /* enable PRINTALL in fifo */
    page_sim_set(1, 5);
    page_sim_set(65, 5);
    page_sim_set(129, 5);
    page_sim_set(199, 5);
    page_sim_set(257, 5);
    page_sim_set(327, 5);
    page_sim_set(397, 5);
    printf("END\n\n");
  }

  if (TEST3a) {
    printf("TEST3a\n");
    page_sim_set(101, 5);
    page_sim_set(102, 7);
    page_sim_set(103, 9);
    page_sim_set(104, 11);
    page_sim_get(101, &v);
    printf("[101]v=%d\n", v);
    page_sim_get(102, &v);
    printf("[102]v=%d\n", v);
    page_sim_get(103, &v);
    printf("[103]v=%d\n", v);
    page_sim_get(104, &v);
    printf("[104]v=%d\n", v);
    printf("END\n\n");
  }

  if (TEST3b) {
    printf("TEST3b\n");
    page_sim_set(1, 5);
    page_sim_set(65, 7);
    page_sim_set(129, 9);
    page_sim_get(1, &v);
    printf("[1]v=%d\n", v);
    page_sim_get(65, &v);
    printf("[65]v=%d\n", v);
    page_sim_get(129, &v);
    printf("[129]v=%d\n", v);
    printf("END\n\n");
  }

  if (TEST4) {
    printf("TEST4\n");
    printf("page_sim_end()\n");
    page_sim_end();
    printf("page_sim_init()\n");
    page_sim_init(64/*rozmiar ramki*/, 2/*ilość pamięci (ilość ramek)*/,
                  8/*przestrzeń adresowa*/, 4/*max ilość jednoczesnych operacji I/O*/,
                  &callback);
  }

  if (TEST5) {
    printf("TEST5\n");
    printf("page_sim_end()\n");
    page_sim_end();
    printf("page_sim_init()\n");
    page_sim_init(4/*rozmiar ramki*/, 1/*ilość pamięci (ilość ramek)*/,
                  2/*przestrzeń adresowa*/, 4/*max ilość jednoczesnych operacji I/O*/,
                  &callback);
    for (i = 0; i < 10; ++i) {
      unsigned a =0;
      if ( (a = page_sim_set(0, 4)) != 0)
         printf("DUPA%d\n", a);
      if ( (a= page_sim_set(7, 6)) != 0)printf("DUPA%d\n", a);
    }
    page_sim_set(121217, 6);
    
    printf("2nd page_sim_end()\n");
    page_sim_end();
    printf("2nd page_sim_init()\n");
    page_sim_init(64/*rozmiar ramki*/, 2/*ilość pamięci (ilość ramek)*/,
                  8/*przestrzeń adresowa*/, 4/*max ilość jednoczesnych operacji I/O*/,
                  &callback);
  }

  if (TEST6) {
    printf("TEST6\n");
    page_sim_end();
    page_sim_init(16/*rozmiar ramki*/, 4/*ilość pamięci (ilość ramek)*/,
                  32/*przestrzeń adresowa*/, 4/*max ilość jednoczesnych operacji I/O*/,
                  &callback);

    int err = 0;
    for (i = 0; i < 512; ++i) {
      page_sim_set(i, i%256);
      page_sim_get(i, &v);
      if (i%256 != v) {
        printf("ERROR: Debug i=%d v=%d\n", i, v);
        err = 1;
        //break;
      }
    }
    
    if (err == 0) printf("OK!\n");
  }

  if (TEST7) {
    printf("TEST7\n");
    page_sim_end();
    page_sim_init(16/*rozmiar ramki*/, 4/*ilość pamięci (ilość ramek)*/,
                  32/*przestrzeń adresowa*/, 4/*max ilość jednoczesnych operacji I/O*/,
                  &callback);

    srand(time(0));
    int err = 0;
    int mod;
    
    for (mod = 256; mod > 0; mod -= 10) {
      printf("test loop mod=%d err=%d\n", mod, err);
      
      int n;
      for (n = 0; n < 1000; ++n)
      for (i = 0; i < 512; ++i)
        page_sim_set(i, i%mod);
        
      for (i = 0; i < 50; ++i) {
        int no = (rand()%512);
        page_sim_get(no, &v);
        if (v != no%mod) {
          printf("ERROR: Debug i=%d no=%d v=%d\n", i, no, v);
          err = 1;
          //break;
        }
      }
      
      if (err == 1) break;
    }
    if (err == 0) printf("OK!\n");
  }
  
  if (TEST8) {
    printf("TEST8 - threads\n");
    if (page_sim_end() != 0) {
      printf("ERROR: lib end\n");             
    }
    if (page_sim_init(16/*rozmiar ramki*/, 4/*ilość pamięci (ilość ramek)*/,
                  32/*przestrzeń adresowa*/, 4/*max ilość jednoczesnych operacji I/O*/,
                  &callback) != 0) {
      printf("ERROR: lib init\n");             
    }

    /* threads attributes */
    pthread_t thread;
    pthread_attr_t attr;
    void* (*th_set)(void*) = thread_fun_set;
    void* (*th_get)(void*) = thread_fun_get;
    if (pthread_attr_init(&attr) != 0 )
      printf("attrinit: th creation\n");
    if (pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED) != 0)
      printf("setdetach: th creation\n");

    counter_th = 100;
    err_th = 0;
    
    for (i = 256; i > 1; i-=1) {
      mod_th = i;
      printf("test loop mod_th=%d err_th=%d\n", mod_th, err_th);

      /* make sure all valueas are set */    
      thread_count = 3;
      if (pthread_create(&thread, &attr, th_set, NULL) != 0) 
        printf("pthread_create: no1\n");
      if (pthread_create(&thread, &attr, th_set, NULL) != 0) 
        printf("pthread_create: no2\n");
      if (pthread_create(&thread, &attr, th_set, NULL) != 0) 
        printf("pthread_create: no3\n");
      while (thread_count != 0) {}
      
      if (err_th == 1) break;
  
      /* gets & sets */    
      thread_count = 6;
      if (pthread_create(&thread, &attr, th_set, NULL) != 0) 
        printf("pthread_create: no4\n");
      if (pthread_create(&thread, &attr, th_set, NULL) != 0) 
        printf("pthread_create: no5\n");
      if (pthread_create(&thread, &attr, th_set, NULL) != 0) 
        printf("pthread_create: no6\n");
      if (pthread_create(&thread, &attr, th_get, NULL) != 0) 
        printf("pthread_create: no7\n");
      if (pthread_create(&thread, &attr, th_get, NULL) != 0) 
        printf("pthread_create: no8\n");
      if (pthread_create(&thread, &attr, th_get, NULL) != 0) 
        printf("pthread_create: no9\n");
      while (thread_count != 0) {}
  
      if (err_th == 1) break;
/*      break;*/
    }
    
    if (err_th == 0) printf("OK!\n");
  }
 
  printf("TESTS END\n\n");
  page_sim_end();
  
return 0;
}
