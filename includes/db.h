#ifndef DB_H
#define DB_H

/* XXX: DB_MYSQL ? */
#define DB_SQL 0x1

typedef struct _blobdata {
    int size;
    void *data;
} blobdata;

int db_query(const char *query, void **result, char *fmt, ...);
int db_fetch_row(void *result, char *fmt, ...);
char *db_error(void);

int open_database(void);
int close_database(void);

#endif /* DB_H */
