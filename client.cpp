#include <iostream>
#include <string>
#include <unistd.h>
#include <arpa/inet.h>
#include <thread>

int socket_fd;

void read_responses_thread() {
  int status;
  char buffer[1024];
  
  while(1) {
    memset(buffer, 0 , sizeof(buffer));
    status = recv(socket_fd, buffer, sizeof(buffer),0);
    if (status <= 0) {
      printf("Either Connection Closed or Error\n");
      exit(0);
    }

    printf("Received: %s\n",buffer);
  }
}

void send_message(const char message[]) {
  if ((send(socket_fd, message, strlen(message), 0)) == -1) {
      fprintf(stderr, "Failure Sending Message");
      close(socket_fd);
      exit(1);
  }
}

int main(int argc, char const *argv[]) {
  // Getting the port number from args
  if (argc != 2) {
      perror("Needs to specifiy port number");
      exit(1);
  }
  int port_number = atoi(argv[1]);

  // Creating a socked and storing the file descriptor
  if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
      perror("Could not create a socket");
      exit(1);
  }

  // Connecting to the server socket
  struct sockaddr_in server_info;
  memset(&server_info, 0, sizeof(server_info));
  server_info.sin_family = AF_INET;
  server_info.sin_port = htons(port_number);
  server_info.sin_addr.s_addr = inet_addr("127.0.0.1");
  if (connect(socket_fd, (struct sockaddr *)&server_info, sizeof(struct sockaddr))<0) {
      perror("Could not connect to server");
      exit(1);
  }

  // Reading responses in thread
  std::thread responses_t (read_responses_thread);
  std::cout << "Now reading reponses in thread...\n";

  // Sending 20 messages
  for (int i = 0; i < 20; ++i) {
    std::string m = "message " + std::to_string(i);
    send_message(m.c_str());
  }

  // Waiting for all messages to get back
  responses_t.join();
  
  return 0;
}
