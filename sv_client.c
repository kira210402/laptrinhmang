#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define MAX_BUF_SIZE 1024

struct SinhVien {
    char mssv[11];
    char hoTen[50];
    char ngaySinh[20];
    float diemTB;
};

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Sai cú pháp. Sử dụng: %s <địa chỉ server> <cổng server>\n", argv[0]);
        return 1;
    }

    // Tạo socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Lỗi tạo socket");
        return 1;
    }

    // Kết nối tới sv_server
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2]));
    inet_pton(AF_INET, argv[1], &(server_addr.sin_addr));
    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Lỗi kết nối tới server");
        return 1;
    }

    // Nhập thông tin sinh viên
    struct SinhVien sv;
    printf("MSSV: ");
    fgets(sv.mssv, sizeof(sv.mssv), stdin);
    sv.mssv[strcspn(sv.mssv, "\n")] = '\0';
    printf("Họ tên: ");
    fgets(sv.hoTen, sizeof(sv.hoTen), stdin);
    sv.hoTen[strcspn(sv.hoTen, "\n")] = '\0';
    printf("Ngày sinh: ");
    fgets(sv.ngaySinh, sizeof(sv.ngaySinh), stdin);
    sv.ngaySinh[strcspn(sv.ngaySinh, "\n")] = '\0';
    printf("Điểm trung bình: ");
    scanf("%f", &sv.diemTB);

    // Gửi thông tin đến sv_server
    char buf[MAX_BUF_SIZE];
    snprintf(buf, sizeof(buf), "%s %s %s %.2f", sv.mssv, sv.hoTen, sv.ngaySinh, sv.diemTB);
    if (send(sockfd, buf, strlen(buf), 0) < 0) {
        perror("Lỗi gửi dữ liệu");
        return 1;
    }

    // Đóng kết nối và kết thúc chương trình
    close(sockfd);
    return 0;
}