chat_client: chat_client.c
	gcc -o chat_client -std=c99 -Wall -O1 chat_client.c

chat_server: chat_server.c
	gcc -pthread -o chat_server -std=c99 -Wall -O1 chat_server.c

clean:
	rm -f chat_client chat_server
