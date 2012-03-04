#include <services.h>
#include <permissions.h>
#include <string.h>
#include <stdlib.h>

permclass defaultperms = {"DEFAULT", 0, NULL, NULL, NULL};
permclass *permclasslist = &defaultperms;
permclass *defaultpermclass = &defaultperms;
static unsigned int permclassid = 0;

static permtxlist *createMTX(char *name){
    permtxlist *mtx;
    int len;
    char *index;
    safemalloc(mtx, permtxlist, NULL);
    mtx->subperm = NULL;
    mtx->perm = 0;
    mtx->next = NULL;
    index = strstr(name, ".");
    if(index){
        len = (int)(index-(name));
    } else {
        len = strlen(name);
    }
    safenmalloc(mtx->name, char, len+1, NULL)/* not always static */
    strncpy(mtx->name, name, len);
    return mtx;
}

static void addPermissionToMTX(permtxlist **mtx, char *name){
    permtxlist *list;
    char *tail;
    list = *mtx;
    int len;
    tail = strstr(name, ".");
    if(tail){
        len = (int)(tail-(name));
        tail++;
    } else {
        len = strlen(name);
    }
    if(!list){
        list = *mtx = createMTX(name);
        if(tail)
            addPermissionToMTX(&list->subperm, tail);
        return;
    }
    while(list){
        if(!strncasecmp(list->name, name, len)){
            if(tail)
                addPermissionToMTX(&list->subperm, tail);
            return;
        }
        list = list->next;
    }
    list = createMTX(name);
    list->next = *mtx;
    *mtx = list;
    if(tail)
        addPermissionToMTX(&list->subperm, tail);
    return;
}

static permtxlist *getPermissionMTX(permtxlist *mtx, char *name){
    permtxlist *list;
    char *tail;
    list = mtx;
    int len;
    tail = strstr(name, ".");
    if(tail){
        len = (int)(tail-(name));
        tail++;
    } else {
        len = strlen(name);
    }
    while(list){
        if(!strncasecmp(list->name, name, len)){
            if(tail)
                return getPermissionMTX(list->subperm, tail);
            return list;
        }
        list = list->next;
    }
    return NULL;
}

void addPermission(char *name){
    addPermissionToMTX(&(defaultperms.mtx), name);
}

permclass *addPermissionClass(char *name){
    permclass *node;
    if((node = getPermissionClass(name)))
        return node;
    safemalloc(node, permclass, NULL);
    safenmalloc(node->name, char, strlen(name)+1, NULL);
    strcpy(node->name, name);
    node->mtx = NULL;
    node->parent = &defaultperms;
    node->permid = ++permclassid;
    node->next = defaultperms.next;/* front insertion */
    defaultperms.next = node;
    return node;
}

permclass *getPermissionClass(char *name){
    permclass *node;
    node = permclasslist;
    while(node){
        if(!strcasecmp(name, node->name)){
            return node;
        }
        node = node->next;
    }
    return NULL;
}

void setPermclassParent(char *name, char *parentname){
    permclass *child, *parent;
    if(!(child = getPermissionClass(name)))
        return;
    if(!(parent = getPermissionClass(parentname)))
        return;
    child->parent = parent;
}

void setPermission(char *classname, char *permname, int perm){
    permtxlist *mtx;
    permclass *class;
    if(!(class = getPermissionClass(classname)))
        return;
    addPermissionToMTX(&(class->mtx), permname);
    mtx = getPermissionMTX(class->mtx, permname);
    if(!mtx)
        return;/* er, wha? */
    mtx->perm = perm;
}

int getPermission(char *classname, char *permname){
    permtxlist *mtx;
    permclass *class;
    if(!(class = getPermissionClass(classname)))
        return 0;
    mtx = getPermissionMTX(class->mtx, permname);
    if(!mtx){
        if(class->parent)
            return getPermission(class->parent->name, permname);
        else
            return 0;
    }
    return mtx->perm;
}

void loadPermissions(){
    void *result;
    int classid, parent, perm;
    char *classname, *permname;
    permclass *class = &defaultperms, *parentclass;
    permclassid = 0;
    db_query("CREATE TABLE IF NOT EXISTS `permclass` ("
    "`classid` int unsigned NOT NULL UNIQUE PRIMARY KEY,"
    "`classname` text (255) NOT NULL,"
    "`parent` int unsigned);", NULL, NULL);
    db_query("CREATE TABLE IF NOT EXISTS `perms` ("
    "`classid` int unsigned,"
    "`permname` text (255) NOT NULL,"
    "`perm` tinyint);", NULL, NULL);
    db_query("CREATE TABLE IF NOT EXISTS `userperms` ("
    "`nickid` int unsigned,"
    "`permname` text (255) NOT NULL,"
    "`perm` tinyint);", NULL, NULL);
    db_query("SELECT `perms`.`classid`, `classname`, `parent`, `permname`, `perm`"
    "FROM `permclass` LEFT JOIN `perms` ON"
    "`perms`.`classid` = `permclass`.`classid` ORDER BY `perms`.`classid`;", &result, NULL);
    while(db_fetch_row(result, "isisi", &classid, &classname, &parent, &permname, &perm)==5){
        aclog(LOG_DEBUG, "%s %s %i\n", classname, permname, perm);
        if(classid!=permclassid){
            safemallocvoid(class, permclass);
            safenmallocvoid(class->name, char, strlen(classname)+1);
            strcpy(class->name, classname);
            class->mtx = NULL;
            class->parent = &defaultperms;
            class->permid = permclassid;
            if(parent!=0){
                parentclass = defaultperms.next;
                while(parentclass){
                    if(parentclass->permid==parent){
                        class->parent = parentclass;
                        break;
                    }
                    parentclass = parentclass->next;
                }
            }
            class->next = defaultperms.next;
            defaultperms.next = class;
        }
        setPermission(class->name, permname, perm);
    }
}
