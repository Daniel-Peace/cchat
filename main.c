#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <poll.h>
#include <string.h>

#define BACKLOG 10
#define MAX_CLIENTS 20
#define THREAD_POOL_SIZE 10
#define READ_BUFFER_SIZE 8192

// A buffer for reading or writing from
typedef struct {
    char* buffer;
    int buffer_size;
    int current_position;
} buffer;

// States of client connections
typedef enum {
    READING,
    IDLE
} Connection_State;

// Holds all relevent info for an active client connection
typedef struct {
    int fd;
    Connection_State state;
    buffer read_buffer;
} Client_Connection;

// Serves clients when there is data to read, requets to handle, or responses to be written
void* serve_client(void* args) {
    
}

// Creates a thread pool of worker threads for handling clients
pthread_t* create_thread_pool(int thread_pool_size) {
    pthread_t* thread_pool = malloc(sizeof(int) * thread_pool_size);
    for (int current_thread = 0; current_thread < thread_pool_size; current_thread++) {
        pthread_t thread;
        pthread_create(&thread, NULL, serve_client, NULL); 
        thread_pool[current_thread] = thread;
    }
    return thread_pool;
}

// Joins a thread pool with a given size
int join_thread_pool(pthread_t* thread_pool, int thread_pool_size) {
    for (int current_thread = 0; current_thread < thread_pool_size; current_thread++) {
        pthread_join(thread_pool[current_thread], NULL);
    }
}

// Initializes the server socket
int setup_server_socket() {
    // For tracking errors
    int error = 0;

    // Creating socket
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        return server_socket;
    }

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(9001);
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Binding socket
    error = bind(server_socket, (struct sockaddr*) &server_address, sizeof(server_address));
    if (error == -1) {
        return error;
    }

    error = listen(server_socket, BACKLOG);
    if (error == -1) {
        return error;
    }


    return server_socket;
}

void* manage_clients() {
    int num_of_clients = 0;
    struct pollfd fds[MAX_CLIENTS];

    int server_socket = setup_server_socket();
    if (server_socket == -1) {
        printf("ERROR - \n");
        exit(1);
    }

    printf("Successfully created server socket with fd of: %d\n", server_socket);

    struct pollfd server;
    server.fd = server_socket;
    server.events = POLLIN;

    fds[num_of_clients++] = server;

    while(1) {
        printf("Polling\n");
        poll(fds, num_of_clients, -1);
        printf("Handling poll events\n");

        // Checking what sockets have data to read
        if (num_of_clients > 1) {

            for (int current_client = 1; current_client < num_of_clients; current_client++) {
                // Checking if there is something to read from the current client
                if (fds[current_client].revents == POLLIN) {
                    printf("There is data to read\n");

                    // Are we at the last item in the list?
                    if (current_client < num_of_clients - 1) {
                        printf("Doing a memory move...\n");
                        // No, so we can do a memmove to shift the items down to fill the gap
                        memmove(fds + (sizeof(struct pollfd) * current_client), fds + (sizeof(struct pollfd) * current_client), num_of_clients - current_client);
                    }

                    num_of_clients--;
                }
            }
        }

        // Checking if a client is trying to connect
        printf("Checking if there is a client who is trying to connect...\n");
        if (fds[0].revents == POLLIN) {
            printf("Accepting client connection...\n");
            struct sockaddr_in client_address;
            socklen_t address_length = sizeof(client_address);
            int client_fd = accept(fds[0].fd, (struct sockaddr*) &client_address, &address_length);
            if (client_fd == -1) {
                printf("ERROR - \n");
                // Maybe shutdown here?
            } else {
                printf("Successfully accepted client\n");
                struct pollfd new_client;
                new_client.fd = client_fd;
                new_client.events = POLLIN;
                fds[num_of_clients++] = new_client;
            }
        }
    }
}

int main() {
    // Creating thread pool
    pthread_t* thread_pool = create_thread_pool(THREAD_POOL_SIZE);

    // Creating polling thread
    pthread_t polling_thread;
    pthread_create(&polling_thread, NULL, manage_clients , NULL);
    pthread_join(polling_thread, NULL);

    // Joining thread pool
    join_thread_pool(thread_pool, THREAD_POOL_SIZE);

    // Freeing thread pool memory
    free(thread_pool);
}






