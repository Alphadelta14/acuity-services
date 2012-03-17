#ifndef _MODULE_H
#define _MODULE_H

#define MOD_IDLE    0x0
#define MOD_LOAD    0x1
#define MOD_UNLOAD  0x2
#define MOD_PRERELOAD 0x3
#define MOD_RELOAD  0x4

typedef struct modnode_s {
    char *name;
    void *handle;
    struct modnode_s *next;
} modnode_t;

void *load_module(const char *modname);
void unload_modules(void);
void unload_module(char *name);
void *reload_module(char *modname);
char *module_error(void);
void load_dependencies(int count, ...);
bool is_module_loaded(char *modname);
extern int MOD_STATE;

#endif /* _MODULE_H */
