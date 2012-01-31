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


int db_query(const char *query, void **result, char *fmt, ...){
    /* Multiple queries can only be executed if result is NULL */
    int status, param, fparam = 0;
    const char *tail = NULL;
    char type;
    va_list args;
    sqlite3_stmt *statement;
    va_start(args, fmt);
    do{
        param = 1;
        status = sqlite3_prepare_v2(db_conn, query, -1, &statement, &tail);
        if(status!=SQLITE_OK)
            return status;/* oh noes, error */
        if(fmt){
            while((type = fmt[fparam])&&(type!=',')){
                switch(type){
                case 'i':
                    status = sqlite3_bind_int(statement, param, va_arg(args, int));
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
                fparam++;
            }
            if(type==',')
                fparam++;
        }
        if(result){
            *result = (void*)statement;
            return SQLITE_OK;
        }
        status = sqlite3_step(statement);
        if((status!=SQLITE_DONE)&&(status!=SQLITE_ROW)) return status;
        status = sqlite3_finalize(statement);
        if(status!=SQLITE_OK) return status;
        query = tail;
    }while(tail[0]);
    va_end(args);
    return status;
}

int db_fetch_row(void *result, char *fmt, ...){
    /* db_fetch_row(result, "iis", &int1, &int2, &str1); returns number of successful conversions */
    int param = 0, conversions = 0, status;
    char type;
    va_list args;
    sqlite3_stmt *statement;
    va_start(args, fmt);
    if(!result)
        return 0;
    statement = (sqlite3_stmt*)result;
    status = sqlite3_step(statement);
    if(status==SQLITE_ROW){
        if(fmt){
            while((type = fmt[param])){
                switch(type){
                case 'i':
                    *(va_arg(args, int*)) = sqlite3_column_int(statement, param);
                    break;
                case 's':
                    /*columnS = sqlite3_column_text(statement, param);
                    len = sqlite3_column_bytes(statement, param)+1;
                    returnS = va_arg(args, char**);
                    safenmalloc(*returnS, char*, len, NULL);
                    strncpy(*returnS, columnS, len);*/
                    *(va_arg(args, char**)) = (char*)sqlite3_column_text(statement, param);
                    break;
                default:

                    break;
                }
                param++;
                conversions++;
            }
        }
    }else if(status==SQLITE_DONE){
        status = sqlite3_finalize(statement);
        return -1;/* maybe return EOF constant? */
    }
    return conversions;
}
