#ifndef EVENT_QUEUE_H_
#define EVENT_QUEUE_H_

// This type defines the different types of events that
// our system recognizes. The priorities of the different
// events are encoded as their integer values, with higher
// values representing higher priority
typedef enum {
   LINE_DETECTED = 255,
   CONTACT_DETECTED_BOTH  = 180,
   CONTACT_DETECTED_FRONT = 170,
   CONTACT_DETECTED_REAR  = 160,
   NEW_PROXIMITY_READINGS = 127,
   MOVEMENT_COMPLETE = 63,

   NULL_EVENT = 0
} event_t;

void event_q_init();

// This should ONLY be called from ISRs! No main loop!!
void event_q_add_event(event_t event);

// NB This function must ONLY be called from main loop code!!!
event_t event_q_get_next_event();

#endif
