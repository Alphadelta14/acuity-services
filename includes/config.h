#ifndef _CONFIG_H
#define _CONFIG_H

#include "services.h"

typedef metanode_t confval_t;

void setConfigValue(char *name, char *val);
char *getConfigValue(char *name);

void loadConfig(int argc, char *argv[]);

#endif /* _CONFIG_H */
