#include <config.h>
#include <sqlite3.h>
#include <stdlib.h>
#include <stdarg.h>
#include <acuity.h>
#include <db.h>

sqlite3 *db_conn = NULL;

int open_database(){
    char *dbFileName, *defaultDbFile = "acuity.db";
    if(!(dbFileName = getConfigValue("dbFile")))
        dbFileName = defaultDbFile;
    return sqlite3_open(dbFileName, &db_conn);
}

int close_database(){
    int status;
    status = sqlite3_close(db_conn);
    if(status==SQLITE_BUSY)
        aclog(LOG_ERROR, "Tried to close database while still busy!\n");
    if(status==SQLITE_OK)
        db_conn = NULL;
    return status;
}


int db_query(char *query, char *fmt, ...){
    int status, param = 1, i;
    const char *tail = NULL;
    char type;
    va_list args;
    sqlite3_stmt *statement;
    va_start(args, fmt);
    do{
        status = sqlite3_prepare_v2(db_conn, query, -1, &statement, &tail);
        if(status!=SQLITE_OK)
            return status;/* oh noes, error */
        if(fmt){
            while((type = fmt[param-1])){
                switch(type){
                case 'i':
                    i = va_arg(args, int);
                    status = sqlite3_bind_int(statement, param, i);
                    if(status!=SQLITE_OK) return status;
                    break;
                case 's':
                    status = sqlite3_bind_text(statement, param, va_arg(args, char*), -1, SQLITE_STATIC);
                    if(status!=SQLITE_OK) return status;
                    break;
                default:

                    break;
                }
                param++;
            }
        }
        status = sqlite3_step(statement);
        if(status!=SQLITE_DONE) return status;
        status = sqlite3_finalize(statement);
        if(status!=SQLITE_OK) return status;
    }while(tail[0]);
    va_end(args);
    return status;
}

