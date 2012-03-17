#ifndef _UTIL_H
#define _UTIL_H

#define ITER(n) ( n = n ->next)
#define EMPTY(n) (!n)

#define PREPEND(var, list) do{\
var->next = list;\
list = var;\
} while(0);

#define STR(arg) #arg
#define XSTR(name) STR(name)

typedef struct metanode_s {
    char *name;
    char *value;
    struct metanode_s *next;
} metanode_t;

typedef signed char byte;
typedef enum bool_e {
    FALSE = 0,
    TRUE = 1
} bool;

#endif /* _UTIL_ */
