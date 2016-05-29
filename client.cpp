#include <iostream>
#include <string>
#include <unistd.h>
#include <arpa/inet.h>
#include <thread>
#include <vector>
#include <sys/time.h>
#include "helper.cpp"

int socket_fd;
int received_messages_count;

#define BUFFER_SIZE 1024
#define MESSAGES_COUNT 4000

struct Message {
  std::string message;
  struct timeval time_sent;
  struct timeval time_received;
  int time_taken;
};

std::vector<Message*> sent_messages;

Message* find_correct_message(std::string message_string) {
  for (std::vector<Message*>::iterator i = sent_messages.begin(); i != sent_messages.end(); ++i) {
    Message *message = *i;
    if (message_string == message->message) {
      return message;
    }
  }
  return NULL;
}

void read_responses_thread() {
  int status;
  char buffer[BUFFER_SIZE];
  std::string incomplete_message = ""; // used to keep incomplete messages between two buffers

  while(1) {
    memset(buffer, 0 , sizeof(buffer));
    status = recv(socket_fd, buffer, sizeof(buffer),0);
    if (status <= 0) {
      printf("Either Connection Closed or Error\n");
      exit(0);
    }
    buffer[status] = '\0';

    // Read each message and calculate the round trip time
    std::string combined_buffer = incomplete_message + buffer;
    std::vector<std::string> messages = parse_messages(combined_buffer);
    for (int i = 0; i < messages.size()-1; ++i) {
      Message* message = find_correct_message(messages[i]);

      if (message == NULL) {
        std::cout << "Received message does not exist: " << messages[i] << std::endl;
        exit(0);
      } else {
        gettimeofday(&message->time_received, NULL);
        message->time_taken = (message->time_received.tv_sec - message->time_sent.tv_sec) * 1000000 + (message->time_received.tv_usec - message->time_sent.tv_usec);
        std::cout << "Received " << message->message << " - with round trip time: " << message->time_taken << " microseconds" << std::endl;
        received_messages_count++;
      }

      incomplete_message = messages.back();
    }

    if (received_messages_count >= MESSAGES_COUNT) {
      return;
    }
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

  // Sending messages
  for (int i = 0; i < MESSAGES_COUNT; ++i) {
    Message *message = new Message();
    message->message = "message " + std::to_string(i);
    gettimeofday(&message->time_sent, NULL);
    sent_messages.push_back(message);
    send_message(message->message, socket_fd);
  }

  // Waiting for all messages to get back
  responses_t.join();

  // Calculate the average round trip time
  double total_time = 0;
  for (std::vector<Message*>::iterator i = sent_messages.begin(); i != sent_messages.end(); ++i) {
    Message *message = *i;
    total_time += message->time_taken;
  }
  std::cout << "Average round trip time: " << total_time / MESSAGES_COUNT << " microseconds" << std::endl;

  // Calculate throughput rate
  Message *first_message = sent_messages.front();
  Message *last_message = sent_messages.back();
  float running_time = (last_message->time_received.tv_sec - first_message->time_sent.tv_sec) * 1000000 + (last_message->time_received.tv_usec - first_message->time_sent.tv_usec);
  float throughput_rate = (MESSAGES_COUNT / running_time) * 1000000;
  std::cout << "Running throughput rate: " << throughput_rate << " messages per second" << std::endl;
  
  return 0;
}
