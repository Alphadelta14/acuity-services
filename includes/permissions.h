#ifndef PERMISSIONS_H
#define PERMISSIONS_H

#define PERM_LEVEL          0x0000FFFF
#define PERM_REQUIREOPER    0x00010000
#define PERM_REQUIREOSAUTH  0x00020000
#define PERM_EQUAL          0x00100000
/* note: PERMTX is easier to read than PERMMTX */
typedef struct _permtxlist permtxlist;

struct _permtxlist {
    char *name;
    permtxlist *subperm;
    int perm;
    permtxlist *next;
};

typedef struct _permclass {
    char *name;
    struct _permclass *parent;
    permtxlist *mtx;
    struct _permclass *next;
} permclass;

permclass *getPermissionClass(char *name);
permclass *addPermissionClass(char *name);
void setPermissionParent(char *name, char *parentname);
void addPermission(char *name);
void setPermission(char *classname, char *permname, int perm);
int getPermission(char *classname, char *permname);

extern permclass *defaultpermclass;

#endif /* PERMISSIONS_H */
