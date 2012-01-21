#ifndef _ACTYPES_H
#define _ACTYPES_H

#include <sys/types.h>

#define safemalloc(variable,type,failreturn) do{ variable = ( type *)malloc(sizeof( type ));\
if(! variable ){\
aclog(LOG_ERROR,"Could not allocate memory for %s.\n", #variable );\
return failreturn ;\
}\
} while(0);


#define safenmalloc(variable,type,size,failreturn) do{ variable = ( type *)malloc( size );\
if(! variable ){\
aclog(LOG_ERROR,"Could not allocate memory for %s.\n", #variable );\
return failreturn ;\
}\
} while(0);

/* Defining mode macros. These will be used to build/store modes:
 * +a = MODE_A
 * -a = (MODE_A|MODE_REMOVE)
 * +V = (MODE_V|MODE_MAJOR)
 */
#define MODE_NONE     0x0
#define MODE_A 0x00000001
#define MODE_B 0x00000002
#define MODE_C 0x00000004
#define MODE_D 0x00000008
#define MODE_E 0x00000010
#define MODE_F 0x00000020
#define MODE_G 0x00000040
#define MODE_H 0x00000080
#define MODE_I 0x00000100
#define MODE_J 0x00000200
#define MODE_K 0x00000400
#define MODE_L 0x00000800
#define MODE_M 0x00001000
#define MODE_N 0x00002000
#define MODE_O 0x00004000
#define MODE_P 0x00008000
#define MODE_Q 0x00010000
#define MODE_R 0x00020000
#define MODE_S 0x00040000
#define MODE_T 0x00080000
#define MODE_U 0x00100000
#define MODE_V 0x00200000
#define MODE_W 0x00400000
#define MODE_X 0x00800000
#define MODE_Y 0x01000000
#define MODE_Z 0x02000000
/* Used to get the actual mode in buildModes in network.c */
#define MODE_CHAR   0x03FFFFFF
/* skip 0x0400000: "Special" bits after this gap */
#define MODE_MAJOR  0x08000000 /* is uppercase mode character? */
#define MODE_REMOVE 0x10000000 /* is mode to be removed? */

typedef struct _line {
    char *id; /* UID/SID who sent the message */
    char *command; /* The actual command */
    char **params; /* Array of paramters */
    int paramCount; /* Amount of parameters for the command */
    char *text; /* The "final" parameter after a colon, if provided */
} line;

typedef struct _ptrnode {
    void *data;
    char canFree;
    struct _ptrnode *next;
} ptrnode;

/* metanode = key/value pair node. Used for configuration entries
 * right now
 */
typedef struct _metanode {
    char *name;
    char *value;
    struct _metanode *next;
} metanode;

line (*parseLine)(char *data);
void (*handleLine)(line *l);

char *getMetaValue(metanode *metadata, char *key);
metanode *setMetaValue(metanode *metadata, char *key, char *value);
void clearMetadata(metanode *metadata);

#endif /* _ACTYPES_H */
