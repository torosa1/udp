#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUF_SIZE 1024

void error_handling(char* message);

int main(int argc, char* argv[]) {
    if (argc != 4) {
        printf("Usage: %s <file> <receiver_ip> <port>\n", argv[0]);
        exit(1);
    }

    int sock;
    struct sockaddr_in serv_addr;
    char buf[BUF_SIZE];
    FILE* fp;
    ssize_t read_len;

    sock = socket(PF_INET, SOCK_DGRAM, 0);
    if (sock == -1)
        error_handling("socket() error");

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[2]);
    serv_addr.sin_port = htons(atoi(argv[3]));

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
        error_handling("connect() error");

    sendto(sock, "Greeting", strlen("Greeting"), 0, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    recvfrom(sock, buf, BUF_SIZE, 0, NULL, NULL);
    printf("%s\n", buf);

    char file_name[BUF_SIZE];
    snprintf(file_name, sizeof(file_name), "R_%s", argv[1]);
    sendto(sock, file_name, strlen(file_name), 0, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    recvfrom(sock, buf, BUF_SIZE, 0, NULL, NULL);
    printf("%s\n", buf);

    fp = fopen(argv[1], "rb");
    if (fp == NULL)
        error_handling("fopen() error");

    while ((read_len = fread(buf, 1, BUF_SIZE, fp)) > 0) {
        sendto(sock, buf, read_len, 0, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    }

    sendto(sock, "Finish", strlen("Finish"), 0, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    recvfrom(sock, buf, BUF_SIZE, 0, NULL, NULL);
    printf("%s\n", buf);

    fclose(fp);
    close(sock);
    return 0;
}

void error_handling(char* message) {
    perror(message);
    exit(1);
}
