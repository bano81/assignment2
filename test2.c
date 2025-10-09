
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <pthread.h>

#include "aq.h"

#include "aux.h"

/** 
 * Concurrent program that sends and receives a few integer messages 
 * in order to demonstrate the basic use of the thread-safe Alarm Queue Library
 *
 * By using sleeps we (try to) control the scheduling of the threads in
 * order to obtain specific execution scenarios.  But there is no guarantee.
 *
 */

static AlarmQueue q;
int delay = 300;

void * producer1 (void * arg) {
  for(int i=0; i < 3; i++) {
      msleep(delay);
      put_normal(q, i);
  }
  return 0;
}

void * producer2 (void * arg) {
  for(int i=3; i < 6; i++) {
      msleep(delay);
      put_normal(q, i);
  }
  return 0;
}

void * producer3 (void * arg) {
  for(int i=6; i < 9; i++) {
      msleep(delay);
      put_normal(q, i);
  }
  return 0;
}

void * consumer(void * arg) {
  
  for (int i = 0; i < 9; i++) {
      msleep(100);
      get(q);
  }
  return 0;
}


int main(int argc, char ** argv) {
    int ret;

  q = aq_create();

  if (q == NULL) {
    printf("Alarm queue could not be created\n");
    exit(1);
  }
  
  pthread_t t1, t2, t3, t4 ;


  void *res1, *res2, *res3, *res4;

  printf("----------------\n");

  /* Fork threads */
  pthread_create(&t1, NULL, producer1, NULL);
  pthread_create(&t2, NULL, producer2, NULL);
  pthread_create(&t3, NULL, producer3, NULL);
 
  /* Join with all threads */
  pthread_join(t1, &res1);
  pthread_join(t2, &res2);
  pthread_join(t3, &res3);
   pthread_create(&t4, NULL, consumer, NULL);
  pthread_join(t4, &res4);

  printf("----------------\n");
  //printf("Threads terminated with %ld, %ld\n", (uintptr_t) res1, (uintptr_t) res2);
  printf("Threads terminated with %ld, %ld, %ld, %ld\n", (uintptr_t) res1, (uintptr_t) res2, (uintptr_t) res3, (uintptr_t) res4);
  print_sizes(q);
  
  return 0;
}

