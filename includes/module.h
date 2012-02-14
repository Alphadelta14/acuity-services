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
void unloadModules(void);
void loadDependencies(int count, ...);
void (*initModule)(void);
void (*termModule)(void);

#endif /* MODULE_H */
