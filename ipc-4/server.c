#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 8888
#define MAX_CLIENTS 100
#define BUFFER_SIZE 1024
#define NAME_SIZE 32

typedef struct {
    int sockfd;
    char name[NAME_SIZE];
    int active;
} client_t;

client_t clients[MAX_CLIENTS];
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

void broadcast_message(const char *msg, int sender_fd) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].active && clients[i].sockfd != sender_fd) {
            send(clients[i].sockfd, msg, strlen(msg), 0);
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

void send_user_list(int sockfd) {
    char list[BUFFER_SIZE] = "Connected users:\n";
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].active) {
            strcat(list, "  - ");
            strcat(list, clients[i].name);
            strcat(list, "\n");
        }
    }
    pthread_mutex_unlock(&clients_mutex);
    send(sockfd, list, strlen(list), 0);
}

int add_client(int sockfd, const char *name) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (!clients[i].active) {
            clients[i].sockfd = sockfd;
            strncpy(clients[i].name, name, NAME_SIZE - 1);
            clients[i].name[NAME_SIZE - 1] = '\0';
            clients[i].active = 1;
            pthread_mutex_unlock(&clients_mutex);
            return i;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
    return -1;
}

void remove_client(int sockfd) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].active && clients[i].sockfd == sockfd) {
            clients[i].active = 0;
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

char* get_client_name(int sockfd) {
    static char name[NAME_SIZE];
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].active && clients[i].sockfd == sockfd) {
            strncpy(name, clients[i].name, NAME_SIZE);
            pthread_mutex_unlock(&clients_mutex);
            return name;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
    return "Unknown";
}

void *handle_client(void *arg) {
    int client_fd = *(int*)arg;
    free(arg);
    char buffer[BUFFER_SIZE];
    char message[BUFFER_SIZE + NAME_SIZE + 10];
    
    memset(buffer, 0, BUFFER_SIZE);
    int n = recv(client_fd, buffer, NAME_SIZE - 1, 0);
    if (n <= 0) {
        close(client_fd);
        return NULL;
    }
    buffer[n] = '\0';
    
    char *newline = strchr(buffer, '\n');
    if (newline) *newline = '\0';
    
    if (add_client(client_fd, buffer) < 0) {
        const char *err = "Server is full\n";
        send(client_fd, err, strlen(err), 0);
        close(client_fd);
        return NULL;
    }
    
    printf("%s joined the chat\n", buffer);
    snprintf(message, sizeof(message), "*** %s joined the chat ***\n", buffer);
    broadcast_message(message, client_fd);
    
    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        n = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);
        if (n <= 0) {
            break;
        }
        buffer[n] = '\0';
        
        newline = strchr(buffer, '\n');
        if (newline) *newline = '\0';
        
        if (strcmp(buffer, "/exit") == 0) {
            break;
        } else if (strcmp(buffer, "/list") == 0) {
            send_user_list(client_fd);
        } else if (strlen(buffer) > 0) {
            char *name = get_client_name(client_fd);
            snprintf(message, sizeof(message), "[%s]: %s\n", name, buffer);
            printf("%s", message);
            broadcast_message(message, -1);
        }
    }
    
    char *name = get_client_name(client_fd);
    printf("%s left the chat\n", name);
    snprintf(message, sizeof(message), "*** %s left the chat ***\n", name);
    remove_client(client_fd);
    broadcast_message(message, client_fd);
    close(client_fd);
    
    return NULL;
}

int main(int argc, char *argv[]) {
    int server_fd, *client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    pthread_t tid;
    
    int port = PORT;
    if (argc > 1) {
        port = atoi(argv[1]);
    }
    
    memset(clients, 0, sizeof(clients));
    
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        exit(1);
    }
    
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
    
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        exit(1);
    }
    
    if (listen(server_fd, 10) < 0) {
        perror("listen");
        exit(1);
    }
    
    printf("Chat server started on port %d\n", port);
    
    while (1) {
        client_fd = malloc(sizeof(int));
        *client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
        if (*client_fd < 0) {
            perror("accept");
            free(client_fd);
            continue;
        }
        
        printf("New connection from %s:%d\n", 
               inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        
        pthread_create(&tid, NULL, handle_client, client_fd);
        pthread_detach(tid);
    }
    
    close(server_fd);
    return 0;
}
