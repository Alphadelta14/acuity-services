#ifndef MODULE_H
#define MODULE_H

#define MOD_IDLE    0x0
#define MOD_LOAD    0x1
#define MOD_UNLOAD  0x2
#define MOD_RELOAD  0x3

typedef struct _modnode {
    char *name;
    void *handle;
    struct _modnode *next;
} modnode;

extern modnode *modlist;
void *load_module(const char *modname);
void unload_modules(void);
void unload_module(char *name);
void reload_module(char *modname);
char *module_error(void);
void load_dependencies(int count, ...);
bool is_module_loaded(char *modname);
extern int MOD_STATE;

#endif /* MODULE_H */
