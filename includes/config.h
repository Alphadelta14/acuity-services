#ifndef _CONFIG_H
#define _CONFIG_H

#include "services.h"

typedef struct confval_s {
    char *name;
    char *value;
    char *old;/* TODO: implement deltas */
    struct confval_s *next;
} confval_t;

void set_config_value(char *name, char *val);
char *get_config_value(char *name);

void load_config(int argc, char *argv[]);
void clear_config(void);

#endif /* _CONFIG_H */
