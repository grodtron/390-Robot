#ifndef EVENT_QUEUE_H_
#define EVENT_QUEUE_H_

// This type defines the different types of events that
// our system recognizes. The priorities of the different
// events are encoded as their integer values, with higher
// values representing higher priority
typedef enum {
   LINE_DETECTED = 255,
   CONTACT_DETECTED_FRONT = 190,
   CONTACT_DETECTED_REAR  = 150,
   NEW_PROXIMITY_READINGS = 127,
   MOVEMENT_COMPLETE = 63,

   NULL_EVENT = 0
} event_t;

// This should ONLY be called from ISRs! No main loop!!
void add_event(event_t event);

// NB This function must ONLY be called from main loop code!!!
event_t get_next_event();

#endif
