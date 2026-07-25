#include "dining.h"

#include <pthread.h>
#include <stdlib.h>

typedef struct dining {
  // TODO: Add your variables here
  int capacity;
  int students;         // number of students currently inside
  int cleaning;         // 1 if cleaning is in progress
  int cleaner_waiting;  // 1 if a cleaner is waiting to enter
  pthread_mutex_t lock;
  pthread_cond_t student_can_enter;  // signaled when a student may enter
  pthread_cond_t cleaner_can_enter;  // signaled when cleaner may enter
} dining_t;

dining_t *dining_init(int capacity) {
  // TODO: Initialize necessary variables
  dining_t *dining = malloc(sizeof(dining_t));
  dining->capacity = capacity;
  dining->students = 0;
  dining->cleaning = 0;
  dining->cleaner_waiting = 0;

  pthread_mutex_init(&dining->lock, NULL);
  pthread_cond_init(&dining->student_can_enter, NULL);
  pthread_cond_init(&dining->cleaner_can_enter, NULL);
  return dining;
}
void dining_destroy(dining_t **dining) {
  // TODO: Free dynamically allocated memory
  pthread_mutex_destroy(&(*dining)->lock);
  pthread_cond_destroy(&(*dining)->student_can_enter);
  pthread_cond_destroy(&(*dining)->cleaner_can_enter);
  free(*dining);
  *dining = NULL;
}

void dining_student_enter(dining_t *dining) {
  // TODO: Your code goes here
  pthread_mutex_lock(&dining->lock);
  while (dining->students >= dining->capacity || dining->cleaning ||
         dining->cleaner_waiting) {
    pthread_cond_wait(&dining->student_can_enter, &dining->lock);
  }

  dining->students++;
  pthread_mutex_unlock(&dining->lock);
}

void dining_student_leave(dining_t *dining) {
  // TODO: Your code goes here
  pthread_mutex_lock(&dining->lock);
  dining->students--;

  if (dining->cleaner_waiting && dining->students == 0) {
    pthread_cond_signal(&dining->cleaner_can_enter);
  } else {
    pthread_cond_signal(&dining->student_can_enter);
  }

  pthread_mutex_unlock(&dining->lock);
}

void dining_cleaning_enter(dining_t *dining) {
  // TODO: Your code goes here
  pthread_mutex_lock(&dining->lock);
  dining->cleaner_waiting = 1;  // cleaner is waiting so new students are
                                // blocked

  while (dining->students > 0) {
    pthread_cond_wait(&dining->cleaner_can_enter, &dining->lock);
  }

  dining->cleaner_waiting = 0;
  dining->cleaning = 1;

  pthread_mutex_unlock(&dining->lock);
}

void dining_cleaning_leave(dining_t *dining) {
  // TODO: Your code goes here
  pthread_mutex_lock(&dining->lock);
  dining->cleaning = 0;
  pthread_cond_broadcast(&dining->student_can_enter);
  pthread_mutex_unlock(&dining->lock);
}
