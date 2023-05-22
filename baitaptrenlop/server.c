#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <ctype.h>

#define MAX_CLIENTS 10
#define BUFFER_SIZE 4096

void handle_client_message(int client_socket, fd_set* all_fds, int* client_count) {
    char buffer[BUFFER_SIZE];
    int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);

    if (bytes_received < 0) {
        perror("Error receiving data from client");
        exit(1);
    } else if (bytes_received == 0) {
        // Kết nối đã đóng
        printf("Client %d disconnected\n", client_socket);
        close(client_socket);
        FD_CLR(client_socket, all_fds);
        (*client_count)--;
    } else {
        // Xử lý xâu ký tự từ client
        buffer[bytes_received] = '\0';
        printf("Received from client %d: %s\n", client_socket, buffer);

        // Xử lý xâu ký tự và chuẩn hóa
        char* token;
        char* rest = buffer;
        char response[BUFFER_SIZE] = "";
        while ((token = strtok_r(rest, " ", &rest))) {
            // Xóa ký tự dấu cách không hợp lệ
            if (strlen(token) > 0) {
                strcat(response, token);
                strcat(response, " ");
            }
        }

        // Viết hoa chữ cái đầu các từ, các ký tự còn lại viết thường
        int i;
        int len = strlen(response);
        int is_word_start = 1;
        for (i = 0; i < len; i++) {
            if (is_word_start && response[i] != ' ') {
                response[i] = toupper(response[i]);
                is_word_start = 0;
            } else {
                response[i] = tolower(response[i]);
            }

            if (response[i] == ' ') {
                is_word_start = 1;
            }
        }

        // Gửi kết quả cho client
        if (send(client_socket, response, strlen(response), 0) < 0) {
            perror("Error sending data to client");
            exit(1);
        }
    }
}

void chat_server(int port) {
    // Tạo socket cho server
    int server_socket;
    struct sockaddr_in server_address;

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Error creating server socket");
        exit(1);
    }

    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(port);

    if (bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
        perror("Error binding server socket");
        exit(1);
    }

    if (listen(server_socket, MAX_CLIENTS) < 0) {
        perror("Error listening on server socket");
        exit(1);
    }

    printf("Chat server started on port %d\n", port);

    fd_set all_fds, read_fds;
    int max_fd, i;
    int client_sockets[MAX_CLIENTS];
    int client_count = 0;

    FD_ZERO(&all_fds);
    FD_SET(server_socket, &all_fds);
    max_fd = server_socket;

    while (1) {
        read_fds = all_fds;

        if (select(max_fd + 1, &read_fds, NULL, NULL, NULL) < 0) {
            perror("Error in select");
            exit(1);
        }

        // Kiểm tra nếu có kết nối mới
        if (FD_ISSET(server_socket, &read_fds)) {
            int new_client_socket = accept(server_socket, NULL, NULL);
            if (new_client_socket < 0) {
                perror("Error accepting new client");
                exit(1);
            }

            printf("New client connected: %d\n", new_client_socket);

            // Gửi xâu chào và số lượng client đang kết nối
            char welcome_message[BUFFER_SIZE];
            sprintf(welcome_message, "Xin chào. Hiện có %d clients đang kết nối.\n", client_count);
            if (send(new_client_socket, welcome_message, strlen(welcome_message), 0) < 0) {
                perror("Error sending welcome message to client");
                exit(1);
            }

            // Thêm client socket vào tập hợp
            FD_SET(new_client_socket, &all_fds);
            client_sockets[client_count] = new_client_socket;
            client_count++;

            // Cập nhật giá trị max_fd nếu cần thiết
            if (new_client_socket > max_fd) {
                max_fd = new_client_socket;
            }
        }

        // Xử lý các thông điệp từ client
        for (i = 0; i < client_count; i++) {
            int client_socket = client_sockets[i];
            if (FD_ISSET(client_socket, &read_fds)) {
                handle_client_message(client_socket, &all_fds, &client_count);
            }
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    int port = atoi(argv[1]);

    chat_server(port);

    return 0;
}
