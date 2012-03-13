#ifndef _UTIL_H
#define _UTIL_H

#define ITER(n) ( n = n ->next)
#define EMPTY(n) (!n)

#define STR(arg) #arg
#define XSTR(name) STR(name)

/* metanode = key/value pair node. Used for configuration entries
 * right now
 */
typedef struct metanode_s {
    char *name;
    char *value;
    struct metanode_s *next;
} metanode_t;

#endif /* _UTIL_ */
