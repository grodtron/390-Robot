#ifndef LINE_SENSORS_H_
#define LINE_SENSORS_H_

#define LINESENS_FL (1<<0)
#define LINESENS_FR (1<<1)
#define LINESENS_BL (1<<2)
#define LINESENS_BR (1<<3)

typedef enum {
   LINE_NONE         = 0,
   LINE_FRONT        = LINESENS_FL | LINESENS_FR,
   LINE_BACK         = LINESENS_BL | LINESENS_BR,
   LINE_FRONT_LEFT   = LINESENS_FL,
   LINE_FRONT_RIGHT  = LINESENS_FR,
   LINE_BACK_LEFT    = LINESENS_BL,
   LINE_BACK_RIGHT   = LINESENS_BR
} line_position_t;

#undef LINESENS_FL
#undef LINESENS_FR
#undef LINESENS_BL
#undef LINESENS_BR

void line_sensors_init();

line_position_t line_sensors_get_position();

#endif
