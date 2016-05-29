CC := clang++
CFLAGS := -Wall -std=gnu++11 -stdlib=libc++ -g -DDEBUG

all: server client

server: server.cpp
	$(CC) $(CFLAGS) -o server.out server.cpp

client: client.cpp
	$(CC) $(CFLAGS) -o client.out client.cpp

clean:
	@echo "\nCleaning..."; 
	$(RM) -r $(client.out)/* $(TARGET)
	$(RM) -r $(DEPDIR)/*.d

run_client:
	./client.out 4200

run_server:
	./server.out 4200