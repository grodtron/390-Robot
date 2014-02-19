#ifndef CONTACTS_H_
#define CONTACTS_H_

typedef enum {
   CONTACT_NONE  = 0,
   CONTACT_FRONT = 1<<0,
   CONTACT_REAR  = 1<<1
} contact_position_t;

void contacts_init();

contact_position_t contacts_get_position();

#endif
