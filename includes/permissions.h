#ifndef PERMISSIONS_H
#define PERMISSIONS_H

#define MTX_PERMTX 0x1
#define MTX_VALUE  0x2

/* note: PERMTX is easier to read than PERMMTX */
typedef struct _permtxlist permtxlist;

typedef union _permtxnode {/* maybe the only union in acuity */
    permtxlist *mtx;/* MTX_PERMTX */
    int perm;/* MTX_VALUE */
} permtxnode;

struct _permtxlist {
    char *name;
    char type;/* MTX_ */
    permtxnode node;
    permtxlist *next;
};

typedef struct _permclass {
    char *name;
    struct _permclass *parent;
    permtxlist *mtx;
} permclass;
#endif /* PERMISSIONS_H */
