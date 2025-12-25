#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define BUFFER_SIZE 1024
#define NAME_SIZE 32

int sockfd;
volatile int running = 1;

void *receive_handler(void *arg) {
    char buffer[BUFFER_SIZE];
    
    while (running) {
        memset(buffer, 0, BUFFER_SIZE);
        int n = recv(sockfd, buffer, BUFFER_SIZE - 1, 0);
        if (n <= 0) {
            if (running) {
                printf("\nDisconnected from server\n");
                running = 0;
            }
            break;
        }
        printf("%s", buffer);
        fflush(stdout);
    }
    
    return NULL;
}

int main(int argc, char *argv[]) {
    struct sockaddr_in server_addr;
    pthread_t recv_thread;
    char buffer[BUFFER_SIZE];
    char name[NAME_SIZE];
    
    if (argc < 3) {
        printf("Usage: %s <server_ip> <port>\n", argv[0]);
        exit(1);
    }
    
    char *server_ip = argv[1];
    int port = atoi(argv[2]);
    
    // Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        exit(1);
    }
    
    // Connecting to server
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        perror("inet_pton");
        exit(1);
    }
    
    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        exit(1);
    }
    
    printf("Connected to server %s:%d\n", server_ip, port);
    
    // registration ??
    printf("Enter your name: ");
    fflush(stdout);
    if (fgets(name, NAME_SIZE, stdin) == NULL) {
        close(sockfd);
        exit(1);
    }
    name[strcspn(name, "\n")] = '\0';
    
    // Send username to server
    send(sockfd, name, strlen(name), 0);
    
    printf("Welcome to the chat, %s!\n", name);
    printf("Commands: /list - show users, /exit - quit\n");
    printf("-------------------------------------------\n");
    
    pthread_create(&recv_thread, NULL, receive_handler, NULL);
    
    while (running) {
        memset(buffer, 0, BUFFER_SIZE);
        if (fgets(buffer, BUFFER_SIZE, stdin) == NULL) {
            break;
        }
        
        char cmd[BUFFER_SIZE];
        strncpy(cmd, buffer, BUFFER_SIZE);
        cmd[strcspn(cmd, "\n")] = '\0';
        
        if (strcmp(cmd, "/exit") == 0) {
            send(sockfd, cmd, strlen(cmd), 0);
            running = 0;
            break;
        } else if (strcmp(cmd, "/list") == 0) {
            send(sockfd, cmd, strlen(cmd), 0);
        } else if (strlen(cmd) > 0) {
            send(sockfd, cmd, strlen(cmd), 0);
        }
    }
    
    running = 0;
    close(sockfd);
    pthread_join(recv_thread, NULL);
    
    printf("Disconnected.\n");
    return 0;
}
