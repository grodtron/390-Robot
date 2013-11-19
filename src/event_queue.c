#ifdef __AVR__
#include <avr/interrupt.h>
#endif

#include <stddef.h>

typedef enum {
   LINE_DETECTED = 255,
   CONTACT_DETECTED_FRONT = 190,
   CONTACT_DETECTED_REAR  = 150,
   NEW_PROXIMITY_READINGS = 127,
   MOVEMENT_COMPLETE = 63,

   NULL_EVENT = 0
} event_t;

#define HEAP_SIZE 31
#define HEAP_MAX_INDEX (HEAP_SIZE - 1)

static size_t  event_q_next;
static event_t event_queue[HEAP_SIZE];

static inline void swap(event_t * a, event_t * b){
   event_t t = *a;
   *a = *b;
   *b = t;
}

// This should ONLY be called from ISRs! No main loop!!
void event_q_add_event(event_t event){

   size_t i;

   //----------------------------//
   //    Find starting index     //
   //----------------------------//
   if(event_q_next < HEAP_SIZE){
      // There is an empty slot, so we just stick it there
      i = event_q_next;
      ++event_q_next;
   }else{
      // There's no empty slot, so we check the bottom row and
      // boot out the first thing with lower priority
      size_t j;
      for(j = HEAP_MAX_INDEX; j > (HEAP_MAX_INDEX - 1)/2; --j){
         if(event_queue[j] < event){
            i = j;
            break;
         }
      }

      // Here we didn't find anything with lower priority
      if(j == (HEAP_MAX_INDEX - 1)/2){
         // Can't be added, silently ignore
         // TODO - should we give feedback?
         return;
      }
   }

   //----------------------------//
   //   Insert and bubble up     //
   //----------------------------//
   // insert:
   event_queue[i] = event;
   // bubble up
   while(i){
      // Here we just keep swapping with the parent while the
      // child is greater
      size_t parent = (i - 1)/2;
      if(event_queue[i] > event_queue[parent]){
         swap(event_queue + i, event_queue + parent);
         i = parent;
      }else{
         break;
      }
   }
}

// NB This function must ONLY be called from main loop code!!!
event_t event_q_get_next_event(){

   #ifdef __AVR__
   cli();
   #endif

   if(event_q_next == 0){
      return NULL_EVENT;
   }

   // Save highest priority event to return at the end
   event_t return_val = event_queue[0];

   // Move bottom event to top
   --event_q_next;

   event_queue[0]            = event_queue[event_q_next];
   event_queue[event_q_next] = NULL_EVENT;


   size_t left=1, right=2, i=0, next;
   // NB - we assume unused slots are always NULL_EVENT
   while(left < event_q_next || right < event_q_next){
      if(event_queue[left] > event_queue[i] || event_queue[right] > event_queue[i]){

         next = event_queue[left] > event_queue[right] ? left : right;

         swap(event_queue + i, event_queue + next);

         i     = next;
         left  = (2*i) + 1;
         right = (2*i) + 2;

      }else{
         break;
      }
   }

   #ifdef __AVR__
   sei();
   #endif

   return return_val;

}


#ifndef __AVR__

// These are stress tests to make sure that our little priority queue
// works correctly

#include <stdio.h>

#include <stdlib.h>
#include <time.h>

#include <assert.h>

int main()
{

   srand(time(0));


   int i;
   for(i = 0; i < 100; ++i){

      int j;
      for(j = 0; j < HEAP_SIZE + 5; ++j){
         add_event((event_t) rand() & 0xFF);
      }

      event_t curr, prev;
      prev = get_next_event();
      printf("%d ", prev);
      while(curr = get_next_event()){
         printf("%d ", curr);
         assert(curr <= prev);
         prev = curr;
      }
      printf("\n");

   }

   for(i = 0; i < 100; ++i){

      int j;
      for(j = 0; j < HEAP_SIZE + 5; ++j){
         add_event((event_t) rand() & 0xFF);
      }

      event_t curr, prev;
      prev = get_next_event();
      printf("%d ", prev);
      for(j = 0; j < HEAP_SIZE/2; ++j){
         curr = get_next_event();
         printf("%d ", curr);
         assert(curr <= prev);
         prev = curr;
      }
      printf("\n");

      for(j = 0; j < HEAP_SIZE + 5; ++j){
         add_event((event_t) rand() & 0xFF);
      }

      prev = get_next_event();
      printf("%d ", prev);
      while(curr = get_next_event()){
         printf("%d ", curr);
         assert(curr <= prev);
         prev = curr;
      }
      printf("\n");

   }
   printf("\n\nAll tests passed!!\n\n");

   return 0;
}

#endif
