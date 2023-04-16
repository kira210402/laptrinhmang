#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

// Server nhan file tu client

int main(int argc, char *argv[])
{
    int port = atoi(argv[1]);
    char *fileChao = argv[2];
    char *fileContent = argv[3];
    
    FILE *f = fopen(fileChao, "r");

    if (f == NULL)
    {
        printf("open file %s failed", fileChao);
        return 1;
    }

    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == -1)
    {
        perror("socket() failed");
        return 1;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);

    if (bind(listener, (struct sockaddr *)&addr, sizeof(addr)))
    {
        perror("bind() failed");
        return 1;
    }
    else 
    {
        printf("ban da tao thanh cong server, port: %d\n", port);
    }

    if (listen(listener, 5))
    {
        perror("listen() failed");
        return 1;
    }

    struct sockaddr_in clientAddr;
    int clientAddrLen = sizeof(addr);

    int client = accept(listener, (struct sockaddr *)&clientAddr, &clientAddrLen);
    printf("Client IP: %s:%d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

    char buf[254]; 
    int ret = 0;

    while (!feof(f))
    {
        ret = fread(buf, 1, sizeof(buf), f);
        if (ret <= 0)
            break;
        send(client, buf, ret, 0);
    }

    fclose(f);

    f = fopen(fileContent, "w");
    
    printf("du lieu nhan tu client se duoc luu trong file: %s\n", fileContent);

    while (1)
    {
        ret = recv(client, buf, sizeof(buf), 0);
        if (ret <= 0)
            break;
        if (ret < sizeof(buf))
            buf[ret] = 0;
        printf("%s",buf);
        fwrite(buf, 1, ret, f);
    }

    fclose(f);
    
    close(client);
    close(listener);
}