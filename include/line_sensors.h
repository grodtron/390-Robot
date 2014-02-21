#ifndef LINE_SENSORS_H_
#define LINE_SENSORS_H_

#define LINESENS_FL (1<<0)
#define LINESENS_FR (1<<1)
#define LINESENS_RL (1<<2)
#define LINESENS_RR (1<<3)

typedef enum {
   LINE_NONE         = 0,
   LINE_FRONT        = LINESENS_FL | LINESENS_FR,
   LINE_REAR         = LINESENS_RL | LINESENS_RR,
   LINE_FRONT_LEFT   = LINESENS_FL,
   LINE_FRONT_RIGHT  = LINESENS_FR,
   LINE_REAR_LEFT    = LINESENS_RL,
   LINE_REAR_RIGHT   = LINESENS_RR
} line_position_t;

#undef LINESENS_FL
#undef LINESENS_FR
#undef LINESENS_RL
#undef LINESENS_RR

void line_sensors_init();

line_position_t line_sensors_get_position();

#endif
