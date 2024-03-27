
UDP를 사용하여 파일을 전송하는 송신자 프로그램입니다.

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>

#define BUF_SIZE 1024

void error_handling(char* message);

int main(int argc, char* argv[]) {
    if (argc != 4) {
        printf("Usage: %s <file> <receiver_ip> <port>\n", argv[0]);
        exit(1);
    }
    
### 프로그램은 명령행에서 4개의 인수를 받습니다.
### 첫 번째 인수는 프로그램 이름이며, 두 번째는 전송할 파일의 이름, 세 번째는 수신자의 IP 주소, 네 번째는 포트 번호입니다.

    int sock;
    struct sockaddr_in serv_addr;
    char buf[BUF_SIZE];
    FILE* fp;
    ssize_t read_len;

    sock = socket(PF_INET, SOCK_DGRAM, 0);
### socket() 함수를 사용하여 UDP 소켓을 생성합니다.

    if (sock == -1)
        error_handling("socket() error");
### 만약 소켓 생성에 실패했을 떄 오류 처리합니다.

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[2]);
    serv_addr.sin_port = htons(atoi(argv[3]));
### 수신자의 IP 주소와 포트 번호를 설정합니다.

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
        error_handling("connect() error");
### 소켓을 생성한 후에는 connect() 함수를 사용하여 소켓을 수신자의 주소에 연결합니다.

    sendto(sock, "Greeting", strlen("Greeting"), 0, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    recvfrom(sock, buf, BUF_SIZE, 0, NULL, NULL);
    printf("%s\n", buf);
### Greeting" 메시지를 수신자에게 보내고 recvfrom() 함수를 사용하여 "OK" 응답을 기다립니다.

    char file_name[BUF_SIZE];
    snprintf(file_name, sizeof(file_name), "R_%s", argv[1]);
    sendto(sock, file_name, strlen(file_name), 0, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    recvfrom(sock, buf, BUF_SIZE, 0, NULL, NULL);
    printf("%s\n", buf);
### 전송할 파일의 이름을 원본 파일과 헷갈리지 않게 하기 위해 "R_<파일이름>" 형식으로 변경하고 수신자에게 보냅니다. 
### "OK" 응답을 기다립니다.

    fp = fopen(argv[1], "rb");
    if (fp == NULL)
        error_handling("fopen() error");

    while ((read_len = fread(buf, 1, BUF_SIZE, fp)) > 0) {
        sendto(sock, buf, read_len, 0, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    }
### 전송할 파일을 읽어서 파일 데이터를 버퍼에 담고, 이를 sendto() 함수를 사용하여 수신자로 전송합니다.

    sendto(sock, "Finish", strlen("Finish"), 0, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    recvfrom(sock, buf, BUF_SIZE, 0, NULL, NULL);
    printf("%s\n", buf);
### 파일 전송이 완료되면 "Finish" 메시지를 수신자에게 보냅니다. 그리고 "WellDone" 응답을 기다립니다.

    fclose(fp);
    close(sock);
### 파일 전송이 완료되면 파일을 닫고, 소켓을 닫아 프로그램이 종료됩니다.

    return 0;
}




UDP를 사용하여 파일을 전송받는 수신자 프로그램입니다.

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
### 헤더파일입니다.

#define BUF_SIZE 1024
### 소켓 통신에 버퍼의 크기입니다.

void error_handling(char* message);

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: %s <port>\n", argv[0]);
        exit(1);
    }
### 인수의 개수를 확인하고 배열의 저장합니다. 개수가 맞지 않으면 오류가 출력됩니다.

    int serv_sock;
    char buf[BUF_SIZE];
    struct sockaddr_in serv_addr, clnt_addr;
    socklen_t clnt_addr_size;
    FILE* fp;

    serv_sock = socket(PF_INET, SOCK_DGRAM, 0);
    if (serv_sock == -1)
        error_handling("socket() error");
### UDP 소켓을 생성합니다. 소켓이 생성되지 못하면 error_handling() 함수를 호출하여 오류 메시지를 출력하고 프로그램을 종료합니다.

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[1]));
### serv_addr 구조체를 초기화하고 서버 소켓 주소 정보를 설정하고. INADDR_ANY를 사용하여 서버가 모든 네트워크 인터페이스에서 들어오는 패킷을 수신합니다.

    if (bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
        error_handling("bind() error");
### 바인딩을 못하면 error_handling() 함수를 호출하여 오류 메시지를 출력하고 프로그램을 종료합니다.

    while (1) {
        clnt_addr_size = sizeof(clnt_addr);
        ssize_t recv_len = recvfrom(serv_sock, buf, BUF_SIZE, 0, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
        buf[recv_len] = '\0';
        printf("%s\n", buf);
### 송신사로부터 데이터를 수신합니다. 수신한 데이터를 버퍼에 저장하고 출력합니다.

        sendto(serv_sock, "OK", strlen("OK"), 0, (struct sockaddr*)&clnt_addr, clnt_addr_size);
### 송신자에게 "OK" 메시지를 전송하여 수신 확인을 응답합니다.

        recv_len = recvfrom(serv_sock, buf, BUF_SIZE, 0, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
        buf[recv_len] = '\0';
        printf("%s\n", buf);
### recvfrom() 함수를 사용해 송신자로부터 데이터를 수신합니다. 함수는 소켓으로부터 데이터를 읽어 buf 배열에 저장합니다.

        sendto(serv_sock, "OK", strlen("OK"), 0, (struct sockaddr*)&clnt_addr, clnt_addr_size);

        fp = fopen(buf, "wb");
        if (fp == NULL) {
            error_handling("fopen() error");
            sendto(serv_sock, "Error: Cannot open file", strlen("Error: Cannot open file"), 0, (struct sockaddr*)&clnt_addr, clnt_addr_size);
            continue;
        }
### 송신자에게 받은 파일의 이름을 통해 파일을 열고 이진 데이터로 파일을 씁니다. 실패하면 오류 처리합니다.

        while (1) {
            ssize_t recv_len = recvfrom(serv_sock, buf, BUF_SIZE, 0, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
            if (recv_len <= 0)
                break;
            fwrite(buf, 1, recv_len, fp);
### fwrite() 함수를 사용하여 수신된 데이터를 파일에 씁니다.

            if (strcmp(buf, "Finish") == 0) {
                sendto(serv_sock, "WellDone", strlen("WellDone"), 0, (struct sockaddr*)&clnt_addr, clnt_addr_size);
                printf("%s\n", buf);
                fclose(fp);
                break;
            }
        }
### 수신된 데이터가 "Finish"라는 문자열과 일치하면 "WellDone" 메시지를 송신자에게 보내고, 파일을 닫고 반복문을 종료합니다.
    }

    close(serv_sock);
### 소켓을 닫습니다.

    return 0;
}

void error_handling(char* message) {
    perror(message);
    exit(1);
}
### 시스템 오류 메시지를 출력합니다.




![스크린샷 2024-03-27 230341](https://github.com/torosa1/udp/assets/165176275/0461e283-24a5-4d35-8f36-76c0f25a14f5)

![스크린샷 2024-03-27 233409](https://github.com/torosa1/udp/assets/165176275/af8166ab-140c-43ce-b3a3-ddb90f88b571)

![스크린샷 2024-03-28 001210](https://github.com/torosa1/udp/assets/165176275/74fb060c-a99b-457b-9e1b-a89a1fb63e60)

![스크린샷 2024-03-27 230325](https://github.com/torosa1/udp/assets/165176275/06fa137e-106b-4213-990d-c8c676d70832)
