CC := clang++
CFLAGS := -Wall -std=gnu++11 -stdlib=libc++ -g -DDEBUG
PORT := 4200

all: server client

server: server.cpp
	$(CC) $(CFLAGS) -o server.out server.cpp

client: client.cpp
	$(CC) $(CFLAGS) -o client.out client.cpp

clean:
	@echo "\nCleaning..."; 
	rm server.out
	rm client.out

run_client:
	./client.out $(PORT)

run_server:
	./server.out $(PORT)