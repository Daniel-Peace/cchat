#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>

void* poll_sockets(void* args) {
  printf("Polling sockets...\n");
  return NULL;
}

int main() {
  // Host and port
  char* ip_address = "127.0.0.1";
  int port = 9001;
  int backlog = 10;

  // Error code
  int error = 0;

  // Creating listening socket
  int accepting_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (accepting_socket_fd == -1) {
    printf("ERROR - %s\n", strerror(errno));
    exit(1);
  }

  // Create address for socket
  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = inet_addr(ip_address);
  server_addr.sin_port = htons(port);

  // Bind accepting socket
  error = bind(accepting_socket_fd, (struct sockaddr *) &server_addr, sizeof(server_addr));
  if (error == -1) {
    printf("ERROR - %s\n", strerror(errno));
    exit(1);
  }

  // Set socket to listen
  error = listen(accepting_socket_fd, backlog);
  if (error == -1) {
    printf("ERROR - %s\n", strerror(error));
    exit(1);
  }


  // Create polling thread
  pthread_t polling_thread;
  pthread_create(&polling_thread, NULL, poll_sockets, NULL);

  // Joining threads
  pthread_join(polling_thread, NULL);

  exit(0);
}
