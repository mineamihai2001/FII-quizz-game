all:
	gcc -o server server.c -lsqlite3
	gcc -o client client.c

