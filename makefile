all:
	gcc -o server server.c -lsqlite3 -pthread
	gcc -o client client.c

