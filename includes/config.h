#ifndef _CONFIG_H
#define _CONFIG_H

#include "services.h"

typedef metanode_t confval_t;

void set_config_value(char *name, char *val);
char *get_config_value(char *name);

void load_config(int argc, char *argv[]);

#endif /* _CONFIG_H */
