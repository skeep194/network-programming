// 2019113928 정종운
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

void error_handling(char *message);

#define BUF_SIZE 100
// cmd type
#define FILE_REQ 1
#define FILE_RES 2
#define FILE_END 3
#define FILE_END_ACK 4
#define FILE_NOT_FOUND 5
typedef struct
{
    int cmd;
    int buf_len; // 실제 전송되는 파일의 크기 저장
    char buf[BUF_SIZE];
} PACKET;

int main(int argc, char *argv[])
{
    int sock;
    struct sockaddr_in serv_addr;
    int str_len;

    if (argc != 3)
    {
        printf("Usage : %s <IP> <port>\n", argv[0]);
        exit(1);
    }

    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock == -1)
        error_handling("socket() error");

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
        error_handling("connect() error!");
    printf("Input file name: ");

    PACKET packet;
    packet.cmd = FILE_REQ;
    scanf("%s", packet.buf);
    packet.buf_len = strlen(packet.buf);
    write(sock, &packet, sizeof(packet));
    printf("[Tx] cmd: 1, file_name: %s\n", packet.buf);

    int total_rx_count = 0;
    int total_bytes = 0;

    while (1)
    {
        str_len = read(sock, &packet, sizeof(packet));
        if (str_len == -1)
            error_handling("read() error!");

        if (packet.cmd == FILE_RES || packet.cmd == FILE_END)
        {
            total_rx_count++;
            total_bytes += packet.buf_len;
            for (int i = 0; i < packet.buf_len; ++i)
            {
                putchar(packet.buf[i]);
            }
            if (packet.cmd == FILE_END)
            {
                printf("\n---------------------------\n");
                printf("[Rx] cmd: %d, FILE_END\n", packet.cmd);
                packet.cmd = FILE_END_ACK;
                write(sock, &packet, sizeof(packet));
                printf("[Tx] cmd: %d, FILE_END_ACK\n", packet.cmd);
                break;
            }
        }
        else if (packet.cmd == FILE_NOT_FOUND)
        {
            printf("[Rx] cmd: %d, %s: File Not Found\n", packet.cmd, packet.buf);
            break;
        }
        else
        {
            printf("[Client] invalid packet!\n");
            break;
        }
    }
    printf("------------------------------------\n");
    printf("Total Rx count: %d, bytes: %d\n", total_rx_count, total_bytes);
    printf("TCP Client Socket Close!\n");
    printf("------------------------------------\n");
    close(sock);
    return 0;
}

void error_handling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
