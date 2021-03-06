#include <iostream>
#include <string>
#include <unistd.h>
#include <arpa/inet.h>
#include <vector>
#include "helper.cpp"

#define BACKLOG 50
#define BUFFER_SIZE 1024

int main(int argc, char const *argv[]) {
  if (argc != 2) {
      fprintf(stderr, "Needs to specifiy port number\n");
      exit(1);
  }
  int port_number = atoi(argv[1]);

  int socket_fd;
  int client_fd;
  struct sockaddr_in server_info;
  struct sockaddr_in client_info;
  memset(&server_info, 0, sizeof(server_info));
  memset(&client_info, 0, sizeof(client_info));

  // Creating a socked and storing the file descriptor
  if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
      perror("Could not create a socket");
      exit(1);
  }

  // Setting socket option values
  int optval = 1;
  if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int)) == -1) {
      perror("Could not set socket option values");
      exit(1);
  }

  // Setting up server info
  server_info.sin_family = AF_INET;
  server_info.sin_port = htons(port_number);
  server_info.sin_addr.s_addr = INADDR_ANY; 

  // Binding socket with server info
  if ((bind(socket_fd, (struct sockaddr *)&server_info, sizeof(struct sockaddr))) == -1)    {
      perror("Could not bind socket with server info");
      exit(1);
  }

  // Listing on socket with backlog amount of messages
  if ((listen(socket_fd, BACKLOG)) == -1){
      perror("Could not listen on socket");
      exit(1);
  }

  std::string incomplete_message = "";
  while(1) {
    socklen_t size = sizeof(struct sockaddr_in);
    if ((client_fd = accept(socket_fd, (struct sockaddr *)&client_info, &size))==-1 ) {
        perror("Could not accept socket from client");
        exit(1);
    }
    std::cout << "Got connection from client" << inet_ntoa(client_info.sin_addr) << std::endl;

    int status;
    char buffer[BUFFER_SIZE];
    while(1) {
      memset(buffer, 0 , sizeof(buffer));
      if ((status = recv(client_fd, buffer, BUFFER_SIZE, 0)) == -1) {
        perror("Could not recieve from client");
        exit(1);
      }
      else if (status == 0) {
        std::cout << "Connection closed from client" << std::endl;
        break;
      }

      // Read each message and bounce it back to client
      std::string combined_buffer = incomplete_message + buffer;
      std::vector<std::string> messages = parse_messages(combined_buffer);
      for (int i = 0; i < messages.size()-1; ++i) {
        std::string message = messages[i];
        std::cout << "Received: " << message << std::endl;
        send_message(message, client_fd);
      }
      incomplete_message = messages.back();
    }

    close(client_fd);
  } 

  return 0;
}
