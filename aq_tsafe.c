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
  return (AlarmQueue) newQueue;
}

void aq_destroy( AlarmQueue aq) {
  if (aq == NULL) return; //queue not initialized
  AlarmQueueImpl * queue = (AlarmQueueImpl *) aq;
  Msg * current = queue->head;
  while (current != NULL) {
      Msg * next = current->next; // Store next message
      free(current->ID); // Free the message ID
      free(current); // Free the message struct
      current = next;
  }
  pthread_mutex_destroy(&queue->lock); // Destroy the mutex
  pthread_cond_destroy(&queue->not_empty); // Destroy the condition variable
  free(queue);
}

int aq_send_unsafe(AlarmQueue aq, void * msg, MsgKind k) {
  if (aq == NULL) return AQ_UNINIT;
  if (msg == NULL) return AQ_NULL_MSG;
  AlarmQueueImpl * queue = (AlarmQueueImpl *) aq;
  
  // Check if trying to add alarm when one already exists
  if (k == AQ_ALARM && queue->alarmCount >= 1) {
    return AQ_NO_ROOM; // max 1 alarm message
  }
  
  Msg * newMsg = malloc(sizeof(Msg));
  if (newMsg == NULL) return AQ_NO_ROOM; // malloc failed
  
  newMsg->ID = msg;  // This might also be problematic - see below
  newMsg->type = k;
  newMsg->next = NULL;
  
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
  queue->size++;
  if (k == AQ_ALARM) queue->alarmCount++;
  return 0;
}

int aq_send(AlarmQueue aq, void * msg, MsgKind k){
  if (aq == NULL) return AQ_UNINIT; //queue not initialized
  AlarmQueueImpl * queue = (AlarmQueueImpl *) aq;  
  pthread_mutex_lock(&queue->lock);   // lock the critical section
  int result = aq_send_unsafe(aq, msg, k);
  if(result == 0) {
      pthread_cond_signal(&queue->not_empty); // Signal that the queue is not empty
  }
  pthread_mutex_unlock(&queue->lock); // unlock the critical section
  return result;
}


int aq_recv_unsafe(AlarmQueue aq, void * * msg) {
  if (aq == NULL) return AQ_UNINIT;
  AlarmQueueImpl * queue = (AlarmQueueImpl *) aq;
  if (queue->size == 0) return AQ_NO_MSG; //empty queue
  Msg * toReceive = queue->head;
  *msg = toReceive->ID; // Assign the ID to the output parameter
  queue->head = toReceive->next;
  if (queue->head == NULL) queue->tail = NULL; //queue is now empty
  queue->size--;
  if (toReceive->type == AQ_ALARM) queue->alarmCount--;
  MsgKind msgType = toReceive->type;
  free(toReceive);
  return msgType;
}


int aq_recv(AlarmQueue aq, void * * msg){
  if (aq == NULL) return AQ_UNINIT; //queue not initialized
  AlarmQueueImpl * queue = (AlarmQueueImpl *) aq;
  pthread_mutex_lock(&queue->lock);   // lock the critical section
  while(queue->size == 0) {
      pthread_cond_wait(&queue->not_empty, &queue->lock); // Wait for a message to be available
  }
  int result = aq_recv_unsafe(aq, msg);
  pthread_mutex_unlock(&queue->lock); // unlock the critical section
  return result;
}

int aq_size( AlarmQueue aq) {
  if (aq == NULL) return AQ_UNINIT; //queue not initialized
  AlarmQueueImpl * queue = (AlarmQueueImpl *) aq;
  pthread_mutex_lock(&queue->lock);   // lock the critical section
  int size = queue->size;
  pthread_mutex_unlock(&queue->lock); // unlock the critical section
  return size;
}

int aq_alarms( AlarmQueue aq) {
  if( aq == NULL) return AQ_UNINIT; //queue not initialized
  AlarmQueueImpl * queue = (AlarmQueueImpl *) aq;
  pthread_mutex_lock(&queue->lock);   // lock the critical section
  int count = queue->alarmCount;
  pthread_mutex_unlock(&queue->lock); // unlock the critical section
  return count;
}
