#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>

#define MAX_BUF_SIZE 1024

void write_log(char* ip_addr, char* data, char* log_file) {
    FILE* fp = fopen(log_file, "a");
    if (fp == NULL) {
        perror("Lỗi mở file log");
        return;
    }

    time_t raw_time;
    struct tm* time_info;
    char time_str[20];
    time(&raw_time);
    time_info = localtime(&raw_time);
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", time_info);

    fprintf(fp, "%s %s %s\n", ip_addr, time_str, data);
    fclose(fp);
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Sai cú pháp. Sử dụng: %s <cổng> <tên file log>\n", argv[0]);
        return 1;
    }

    // Tạo socket
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0) {
        perror("Lỗi tạo socket");
        return 1;
    }

    // Thiết lập địa chỉ và cổng cho socket
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(atoi(argv[1]));
    if (bind(listenfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Lỗi bind socket");
        return 1;
    }

    // Chờ kết nối từ sv_client
    if (listen(listenfd, 10) < 0) {
        perror("Lỗi listen socket");
        return 1;
    }

    // Vòng lặp chờ và xử lý kết nối
    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        int connfd = accept(listenfd, (struct sockaddr*)&client_addr, &client_addr_len);
        if (connfd < 0) {
            perror("Lỗi accept socket");
            continue;
        }

        // in ra dia chi client
        printf("Client IP: %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        
        char ip_addr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(client_addr.sin_addr), ip_addr, sizeof(ip_addr));

        // Nhận dữ liệu từ sv_client
        char buf[MAX_BUF_SIZE];
        memset(buf, 0, sizeof(buf));
        if (recv(connfd, buf, sizeof(buf), 0) <= 0) {
            close(connfd);
            continue;
        }

        // In dữ liệu ra màn hình và ghi vào file log
        printf("%s - %s\n", buf, argv[2]);
        write_log(ip_addr, buf, argv[2]);

        // Đóng kết nối và tiếp tục chờ kết nối mới
        close(connfd);
    }
    close(listenfd);
    return 0;
}