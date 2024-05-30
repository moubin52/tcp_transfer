#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define FILE_DIR "./files/"

void handle_client(int client_socket) {
    char buffer[BUFFER_SIZE];
    char file_path[BUFFER_SIZE];

    int bytes_read = read(client_socket, buffer, BUFFER_SIZE);
    if (bytes_read <= 0) {
        perror("Error reading filename from client");
        close(client_socket);
        return;
    }
    buffer[bytes_read] = '\0';

    snprintf(file_path, sizeof(file_path), "%s%s", FILE_DIR, buffer);
    printf("Requesting file: %s\n", file_path);

    FILE *file = fopen(file_path, "rb");
    if (!file) {
        perror("Error opening file");
        close(client_socket);
        return;
    }

    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
        if (write(client_socket, buffer, bytes_read) < 0) {
            perror("Error sending file to client");
            break;
        }
    }

    fclose(file);
    close(client_socket);
}

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    pid_t pid;

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error binding socket");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, 5) < 0) {
        perror("Error listening on socket");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Server is listening on port %d\n", PORT);

    while (1) {
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_socket < 0) {
            perror("Error accepting connection");
            continue;
        }

        pid = fork();
        if (pid < 0) {
            perror("Error forking process");
            close(client_socket);
            continue;
        }

        if (pid == 0) {
            close(server_socket);
            handle_client(client_socket);
            exit(0);
        } else {
            close(client_socket);
        }

        while (waitpid(-1, NULL, WNOHANG) > 0);
    }

    close(server_socket);
    return 0;
}
