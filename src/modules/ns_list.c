#include <services.h>
#include "nickserv.h"

void ns_list(char *uid, char *msg){
    /* XXX: how can this possibly get alphabetized */
    nicklist *nicks;
    int count = 1;
    nicks = registerednicks;
    while(nicks){
        ns_message(uid, "%d. \x02%s\x02", count++, nicks->acc->nick);
        nicks = nicks->next;
    }
}
void ns_listgroups(char *uid, char *msg){
    /* XXX: how can this possibly get alphabetized */
    nickgrouplist *groups;
    int count = 1;
    groups = registerednickgroups;
    while(groups){
        ns_message(uid, "%d. \x02%s\x02", count++, groups->group->main->nick);
        groups = groups->next;
    }
}

void INIT_MOD(){
    registerNickServCommand("list", ns_list);
    registerNickServCommand("listgroups", ns_listgroups);
}
