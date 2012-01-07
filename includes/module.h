#ifndef MODULE_H
#define MODULE_H

typedef struct _modnode {
    char *name;
    void *handle;
    /* int flags */
    struct _modnode *next;
} modnode;

extern modnode *modlist;
void *loadModule(const char *modname);
void loadDependencies(int count, ...);
void (*initModule)(void);

#endif /* MODULE_H */
