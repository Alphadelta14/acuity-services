#ifndef MODULE_H
#define MODULE_H

#define MOD_IDLE    0x0
#define MOD_LOAD    0x1
#define MOD_UNLOAD  0x2
#define MOD_RELOAD  0x3

typedef struct _modnode {
    char *name;
    void *handle;
    /* int flags */
    struct _modnode *next;
} modnode;

extern modnode *modlist;
void *loadModule(const char *modname);
void unloadModules(void);
void unloadModule(char *name);
void loadDependencies(int count, ...);
void (*initModule)(void);
void (*termModule)(void);
extern int MOD_STATE;

#endif /* MODULE_H */
