
#ifndef _CONFIG_H
#define _CONFIG_H

#include "actypes.h"

/*typedef struct _confval {
    char *name;
    char *value;
    struct _confval *next;
} confval;*/

typedef metanode confval;

typedef struct _defval {
    char *name;
    char *value;
} defval;

void CONFIG(void);
void INFO(void);
void (*conf)(void);

extern confval *conflist;
void setConfigValue(char *name, char *val);
char *getConfigValue(char *name);

void loadConfig(void);
void printConfig(void);

#endif /* _CONFIG_H */
