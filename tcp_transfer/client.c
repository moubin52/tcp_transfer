#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int client_socket;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    char *filename = argv[1];

    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error connecting to server");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    if (write(client_socket, filename, strlen(filename)) < 0) {
        perror("Error sending filename to server");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    FILE *file = fopen(filename, "wb");
    if (!file) {
        perror("Error opening file for writing");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    int bytes_read;
    while ((bytes_read = read(client_socket, buffer, BUFFER_SIZE)) > 0) {
        if (fwrite(buffer, 1, bytes_read, file) != bytes_read) {
            perror("Error writing to file");
            break;
        }
    }

    if (bytes_read < 0) {
        perror("Error reading from server");
    }

    fclose(file);
    close(client_socket);
    return 0;
}
