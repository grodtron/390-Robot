#ifndef CONTACTS_H_
#define CONTACTS_H_

typedef enum {
   CONTACT_NONE        = 0,
   CONTACT_FRONT_LEFT  = 1,
   CONTACT_FRONT_RIGHT = 2,
   CONTACT_REAR        = 4
} contact_position_t;

void contacts_init();

contact_position_t contacts_get_position();

#endif
