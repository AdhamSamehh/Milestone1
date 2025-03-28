#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_ATTEMPTS 3  // Allow 3 authentication attempts

const char *valid_users[2][2] = {
    {"01010764479", "adham1991@"},   // Phone number and password
    {"01010101010", "adham1234"}
};

// Authentication function
int authentication(const char *phone, const char *pass) {
    for (int i = 0; i < 2; i++) {
        if (strcmp(phone, valid_users[i][0]) == 0 && strcmp(pass, valid_users[i][1]) == 0) {
            return 1;  // Valid user
        }
    }
    return 0;  // Authentication failed
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char phone_number[BUFFER_SIZE], password[BUFFER_SIZE], buffer[BUFFER_SIZE];

    // Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        perror("Socket failed!");
        exit(EXIT_FAILURE);
    }

    // Define server address
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind the socket
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("Bind failed!");
        exit(EXIT_FAILURE);
    }

    // Listen for connections
    if (listen(server_fd, 3) < 0) {
        perror("Listen failed!");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    // Accept client connection
    new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
    if (new_socket < 0) {
        perror("Accept failed!");
        exit(EXIT_FAILURE);
    }

    // Receive phone number
    memset(phone_number, 0, BUFFER_SIZE);
    int bytes_received = recv(new_socket, phone_number, BUFFER_SIZE - 1, 0);
    if (bytes_received <= 0) {
        perror("Failed to receive the phone number");
        close(new_socket);
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    phone_number[bytes_received] = '\0';

    int attempt;
    int authenticated = 0;

    for (attempt = 1; attempt <= MAX_ATTEMPTS; attempt++) {
        // Receive password
        memset(password, 0, BUFFER_SIZE);
        bytes_received = recv(new_socket, password, BUFFER_SIZE - 1, 0);
        if (bytes_received <= 0) {
            perror("Failed to receive the password");
            close(new_socket);
            close(server_fd);
            exit(EXIT_FAILURE);
        }
        password[bytes_received] = '\0';

        printf("Attempt %d - Phone: %s, Password: %s\n", attempt, phone_number, password);

        // Authentication check
        if (authentication(phone_number, password)) {
            send(new_socket, "Login successfull", strlen("Login successful"), 0);
            authenticated = 1;
            break;  // Exit loop on success
        } else if (attempt < MAX_ATTEMPTS) {
            send(new_socket, "Wrong password. Try again.", strlen("Wrong password. Try again."), 0);
        } else {
            send(new_socket, "Failed Login!", strlen("Failed Login!"), 0);
            close(new_socket);
            close(server_fd);
            return 0;
        }
    }

    // Receive client message if authenticated
    if (authenticated) {
        memset(buffer, 0, BUFFER_SIZE);
        bytes_received = recv(new_socket, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_received > 0) {
            buffer[bytes_received] = '\0';
            printf("Client message: %s\n", buffer);
            send(new_socket, buffer, strlen(buffer), 0);
        }
    }

    // Close sockets
    close(new_socket);
    close(server_fd);

    return 0;
}


