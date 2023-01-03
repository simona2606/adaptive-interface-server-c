#include "sql.h"

MYSQL *start_connection(MYSQL *conn) {
    char *server = "localhost";
    char *user = "root";
    char *password = "my-secret-password";
    char *database = "Accessibility";

    conn = mysql_init(NULL);

    /* Connect to database */
    if (!mysql_real_connect(conn, server, user, password, database, 0, NULL, 0)) {
        fprintf(stderr, "%s\n", mysql_error(conn));
        exit(1);
    }
    return conn;

}

void close_connection(MYSQL *conn, MYSQL_RES *res) {
    mysql_free_result(res);
    mysql_close(conn);
}

MYSQL_RES *send_query_insert(MYSQL *conn, MYSQL_RES *res, MYSQL_ROW row, User *u1) {
    char query_insert[200];
    
    sprintf(query_insert, "insert into user(username, password, access) values ('%s', '%s', '%s')", u1->username, u1->password, u1->accessibility);
    
    if (mysql_query(conn, query_insert)) {
	fprintf(stderr, "%s\n", mysql_error(conn));
	exit(1);
    }

    res = mysql_use_result(conn);
    return res;
}

User *send_query_select(MYSQL *conn, MYSQL_RES *res, MYSQL_ROW row, User *u1) {
	char query_select[200];
	
	sprintf(query_select, "select access from user where username='%s'", u1->username);
	
	if (mysql_query(conn, query_select)) {
		fprintf(stderr, "%s\n", mysql_error(conn));
		exit(1);
	}

	res = mysql_use_result(conn);
	
	while ((row = mysql_fetch_row(res)) != NULL) {
		strcpy(u1->accessibility, row[0]);
	}
	
	return u1;
}

bool login(User *u1, MYSQL *conn, MYSQL_RES *res, MYSQL_ROW row) {
	conn = start_connection(conn);
	u1 = send_query_select(conn, res, row, u1);
	close_connection(conn, res);
	if (res != NULL) {
		return true;
	} else {
		return false;
	}
}

bool registration(User *u1, MYSQL *conn, MYSQL_RES *res, MYSQL_ROW row) {
      conn = start_connection(conn);
      res = send_query_insert(conn, res, row, u1);
      close_connection(conn, res);
      
      return true;
}
