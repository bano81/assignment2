/**
 * @file   aq.c
 * @Author 02335 team
 * @date   October, 2024
 * @brief  Alarm queue skeleton implementation
 */

#include <stdio.h>
#include <stdlib.h>
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
} AlarmQueueImpl;

AlarmQueue aq_create( ) {
  AlarmQueueImpl * newQueue = malloc(sizeof(AlarmQueueImpl));
  if (newQueue == NULL) return NULL;
  newQueue->head = NULL;
  newQueue->tail = NULL;
  newQueue->size = 0;
  newQueue->alarmCount = 0;
  return (AlarmQueue) newQueue;
}

int aq_send(AlarmQueue aq, void * msg, MsgKind k) {
  if (aq == NULL) return AQ_UNINIT;
  if (msg == NULL) return AQ_NULL_MSG;
  AlarmQueueImpl * queue = (AlarmQueueImpl *) aq;
  
  // Check if trying to add alarm when one already exists
  if (k == AQ_ALARM && queue->alarmCount >= 1) {
    return AQ_NO_ROOM; // max 1 alarm message
  }
  
  Msg * newMsg = malloc(sizeof(Msg));
  if (newMsg == NULL) return AQ_NO_ROOM; // malloc failed
  
  newMsg->ID = msg;  
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

int aq_recv( AlarmQueue aq, void * * msg) {
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
  
  free(toReceive);  // free the memmory allocated for the message struct

  return msgType;
}

int aq_size( AlarmQueue aq) {
  if (aq == NULL) return AQ_UNINIT; //queue not initialized
  AlarmQueueImpl * queue = (AlarmQueueImpl *) aq;
  return queue->size;
}

int aq_alarms( AlarmQueue aq) {
  if( aq == NULL) return AQ_UNINIT; //queue not initialized
  AlarmQueueImpl * queue = (AlarmQueueImpl *) aq;
  return queue->alarmCount;
}