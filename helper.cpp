
std::vector<std::string> parse_messages(std::string buffer) {
  std::vector<std::string> messages;
  std::string message = "";
  for (int i = 0; i < buffer.length(); ++i) {  
    if (buffer[i] == '#') { // Message delimiter
      messages.push_back(message);
      message = "";
    } else {
      message += buffer[i];
    }
  }
  
  messages.push_back(message);
  
  return messages;
}

void send_message(std::string message, int socket_fd) {
  message += "#"; //Adding delimiter
  const char *m = message.c_str();
  if ((send(socket_fd, m, strlen(m), 0)) == -1) {
      fprintf(stderr, "Failure Sending Message");
      close(socket_fd);
      exit(1);
  }
}