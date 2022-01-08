all_done: server client clean

server: server.c game.c Message.c
	gcc -Wall server.c game.c Message.c -lzmq -o server -pthread

client: client.c game.c Message.c
	gcc -Wall client.c game.c Message.c -lzmq -o client
clean: 
	rm /tmp/bull*
