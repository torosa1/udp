#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUF_SIZE 1024

void error_handling(char* message);

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    int serv_sock;
    char buf[BUF_SIZE];
    struct sockaddr_in serv_addr, clnt_addr;
    socklen_t clnt_addr_size;
    FILE* fp;

    serv_sock = socket(PF_INET, SOCK_DGRAM, 0);
    if (serv_sock == -1)
        error_handling("socket() error");

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[1]));

    if (bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
        error_handling("bind() error");

    while (1) {
        clnt_addr_size = sizeof(clnt_addr);
        ssize_t recv_len = recvfrom(serv_sock, buf, BUF_SIZE, 0, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
        buf[recv_len] = '\0';
        printf("%s\n", buf);

        sendto(serv_sock, "OK", strlen("OK"), 0, (struct sockaddr*)&clnt_addr, clnt_addr_size);

        recv_len = recvfrom(serv_sock, buf, BUF_SIZE, 0, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
        buf[recv_len] = '\0';
        printf("%s\n", buf);

        sendto(serv_sock, "OK", strlen("OK"), 0, (struct sockaddr*)&clnt_addr, clnt_addr_size);

        fp = fopen(buf, "wb");
        if (fp == NULL) {
            error_handling("fopen() error");
            sendto(serv_sock, "Error: Cannot open file", strlen("Error: Cannot open file"), 0, (struct sockaddr*)&clnt_addr, clnt_addr_size);
            continue;
        }

        while (1) {
            ssize_t recv_len = recvfrom(serv_sock, buf, BUF_SIZE, 0, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
            if (recv_len <= 0)
                break;
            fwrite(buf, 1, recv_len, fp);
            if (strcmp(buf, "Finish") == 0) {
                sendto(serv_sock, "WellDone", strlen("WellDone"), 0, (struct sockaddr*)&clnt_addr, clnt_addr_size);
                printf("Sender: %s\n", buf);
                fclose(fp);
                break;
            }
        }
    }

    close(serv_sock);
    return 0;
}

void error_handling(char* message) {
    perror(message);
    exit(1);
}
