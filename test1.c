
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

void * producer (void * arg) {
  put_alarm(q, 42);
  put_alarm(q, 43);
  put_normal(q, 1);
  return 0;
}

void * producer2 (void * arg) {
  msleep(200);
  put_alarm(q, 45);
  put_normal(q, 2);
  return 0;
}

void * consumer(void * arg) {
  msleep(1000);
  get(q);
  msleep(500);
  get(q);
  msleep(500);
  get(q);
  msleep(500);
  get(q);
  get(q);
  return 0;
}


int main(int argc, char ** argv) {
    int ret;

  q = aq_create();

  if (q == NULL) {
    printf("Alarm queue could not be created\n");
    exit(1);
  }
  
  pthread_t t1, t2, t3 ;


  void *res1, *res2, *res3;

  printf("----------------\n");

  /* Fork threads */
  pthread_create(&t1, NULL, producer, NULL);
  pthread_create(&t2, NULL, producer2, NULL);
  pthread_create(&t3, NULL, consumer, NULL);
  
  /* Join with all threads */
  pthread_join(t1, &res1);
  pthread_join(t2, &res2);
  pthread_join(t3, &res3);

  printf("----------------\n");
  //printf("Threads terminated with %ld, %ld\n", (uintptr_t) res1, (uintptr_t) res2);
  printf("Threads terminated with %ld, %ld, %ld\n", (uintptr_t) res1, (uintptr_t) res2, (uintptr_t) res3);
  print_sizes(q);
  
  return 0;
}

