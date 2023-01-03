#ifndef __H_SQL__
#define __H_SQL__

#include "client.h"

MYSQL *start_connection(MYSQL *conn);
void close_connection(MYSQL *conn, MYSQL_RES *res);

MYSQL_RES *send_query_insert(MYSQL *conn, MYSQL_RES *res, MYSQL_ROW row, User *u1);
User *send_query_select(MYSQL *conn, MYSQL_RES *res, MYSQL_ROW row, User *u1);

bool registration(User *u1, MYSQL *conn, MYSQL_RES *res, MYSQL_ROW row);
bool login(User *u1, MYSQL *conn, MYSQL_RES *res, MYSQL_ROW row);

#endif
