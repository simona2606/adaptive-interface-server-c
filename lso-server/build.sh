#!/bin/bash

gcc -o app.out ./server/main.c ./server/client.c ./server/sql.c ./server/utils.c -lmysqlclient
./app.out 5003
