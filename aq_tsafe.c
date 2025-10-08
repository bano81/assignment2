/**
 * @file   aq.c
 * @Author 02335 team
 * @date   October, 2024
 * @brief  Alarm queue skeleton implementation
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "aq.h"


//Queue implementation here
 typedef struct msg{
    void* ID;
    MsgKind type; //'n' for normal, 'a' for alarm
    struct msg * next;
} Msg;

typedef struct alarmQueue{
    Msg * head;
    Msg * tail;
    int size;
    int alarmCount;
    pthread_mutex_t lock;     // Per-queue mutex lock
    pthread_cond_t not_empty; // Condition variable to signal non-empty queue
    pthread_cond_t alarm_available; // Condition variable to signal alarm availability
} AlarmQueueImpl;

AlarmQueue aq_create( ) {
  AlarmQueueImpl * newQueue = malloc(sizeof(AlarmQueueImpl));
  if (newQueue == NULL) return NULL;
  newQueue->head = NULL;
  newQueue->tail = NULL;
  newQueue->size = 0;
  newQueue->alarmCount = 0;
  pthread_mutex_init(&newQueue->lock, NULL); // Initialize the mutex
  pthread_cond_init(&newQueue->not_empty, NULL); // Initialize the condition variable
  pthread_cond_init(&newQueue->alarm_available, NULL); // Initialize the alarm condition variable
  return (AlarmQueue) newQueue;
}

int aq_send(AlarmQueue aq, void * msg, MsgKind k) {
  if (aq == NULL) return AQ_UNINIT;
  if (msg == NULL) return AQ_NULL_MSG;
  AlarmQueueImpl * queue = (AlarmQueueImpl *) aq;
  Msg * newMsg = malloc(sizeof(Msg));
  if (newMsg == NULL) return AQ_NO_ROOM; // malloc failed
  newMsg->ID = msg;  
  newMsg->type = k; 
  newMsg->next = NULL;
  pthread_mutex_lock(&queue->lock);
  if(k == AQ_ALARM){ // Wait if there's already an alarm
    while(queue->alarmCount >= 1) {
        pthread_cond_wait(&queue->alarm_available, &queue->lock); 
    }
  }
  if(queue->head == NULL){ // empty queue
      queue->head = newMsg;
      queue->tail = newMsg;
  } else if(k == AQ_ALARM) { // insert alarm at the front
      newMsg->next = queue->head;
      queue->head = newMsg;
  } else { // normal message, add to the end
      queue->tail->next = newMsg;
      queue->tail = newMsg; 
  }
  queue->size++; // Increment size
  if (k == AQ_ALARM) queue->alarmCount++; // Increment alarm count if
  pthread_cond_signal(&queue->not_empty); // Signal that the queue is not empty
  pthread_mutex_unlock(&queue->lock); // unlock the critical section
  return 0;
}

int aq_recv(AlarmQueue aq, void * * msg) {
  if (aq == NULL) return AQ_UNINIT;
  if (msg == NULL) return AQ_NULL_MSG; 
  AlarmQueueImpl * queue = (AlarmQueueImpl *) aq;
  Msg * toReceive;
  MsgKind msgType;
  pthread_mutex_lock(&queue->lock);   // lock the critical section
  while (queue->size == 0) { // Wait until there's a message to receive
      pthread_cond_wait(&queue->not_empty, &queue->lock); // Wait for a message to be available
  }
  // Remove alarm from the front if exists
  toReceive = queue->head;
  *msg = toReceive->ID; // Assign the ID to the output parameter
  queue->head = toReceive->next;
  if (queue->head == NULL) queue->tail = NULL; //queue is now empty
  queue->size--;
  msgType = toReceive->type;
  if(msgType == AQ_ALARM) {
      queue->alarmCount--;
      pthread_cond_signal(&queue->alarm_available); // Signal that an alarm slot is available
  }
  free(toReceive);
  pthread_mutex_unlock(&queue->lock); // unlock the critical section  
  return msgType;
}

int aq_size( AlarmQueue aq) {
  if (aq == NULL) return AQ_UNINIT; //queue not initialized
  AlarmQueueImpl * queue = (AlarmQueueImpl *) aq;
  int size;
  pthread_mutex_lock(&queue->lock);   // lock the critical section
  size = queue->size;
  pthread_mutex_unlock(&queue->lock); // unlock the critical section
  return size;
}

int aq_alarms( AlarmQueue aq) {
  if( aq == NULL) return AQ_UNINIT; //queue not initialized
  AlarmQueueImpl * queue = (AlarmQueueImpl *) aq;
  int count;
  pthread_mutex_lock(&queue->lock);   // lock the critical section
  count = queue->alarmCount;
  pthread_mutex_unlock(&queue->lock); // unlock the critical section
  return count;
}

