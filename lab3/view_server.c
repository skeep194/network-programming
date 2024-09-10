// 2019113928 정종운
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
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
    int serv_sock;
    int clnt_sock;

    struct sockaddr_in serv_addr;
    struct sockaddr_in clnt_addr;
    socklen_t clnt_addr_size;

    if (argc != 2)
    {
        printf("Usage : %s <port>\n", argv[0]);
        exit(1);
    }

    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    if (serv_sock == -1)
        error_handling("socket() error");

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[1]));

    if (bind(serv_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
        error_handling("bind() error");

    if (listen(serv_sock, 5) == -1)
        error_handling("listen() error");

    printf("------------------------------\n");
    printf("TCP Remote File View Server\n");
    printf("------------------------------\n");

    int total_tx_count = 0;
    int total_bytes = 0;

    clnt_addr_size = sizeof(clnt_addr);
    clnt_sock = accept(serv_sock, (struct sockaddr *)&clnt_addr, &clnt_addr_size);
    if (clnt_sock == -1)
        error_handling("accept() error");

    while (1)
    {
        PACKET packet;
        read(clnt_sock, &packet, sizeof(packet));
        if (packet.cmd == FILE_REQ)
        {
            printf("[Rx] cmd: %d, file_name: %s\n", packet.cmd, packet.buf);
            FILE *file = fopen(packet.buf, "r");
            if (file == NULL)
            {
                packet.cmd = FILE_NOT_FOUND;
                write(clnt_sock, &packet, sizeof(packet));
                printf("[Tx] cmd: %d, %s: File Not Found\n", packet.cmd, packet.buf);
                break;
            }
            else
            {
                packet.cmd = FILE_RES;
                int is_end = 0;
                while (!is_end)
                {
                    int read_bytes = fread(packet.buf, sizeof(packet.buf[0]), BUF_SIZE, file);
                    if (feof(file))
                    {
                        packet.cmd = FILE_END;
                        is_end = 1;
                    }
                    packet.buf_len = read_bytes;
                    write(clnt_sock, &packet, sizeof(packet));
                    total_tx_count++;
                    total_bytes += packet.buf_len;
                    printf("[Tx] cmd: %d, len: %4d, total_tx_cnt: %4d, total_tx_bytes: %4d\n", packet.cmd, packet.buf_len, total_tx_count, total_bytes);
                    sleep(1);
                }
                fclose(file);
            }
        }
        else if (packet.cmd == FILE_END_ACK)
        {
            printf("[Rx] cmd: %d, FILE_END_ACK\n", packet.cmd);
            break;
        }
        else
        {
            printf("invalid packet!\n");
        }
    }

    printf("----------------------------------------\n");
    printf("Total Tx count: %d, bytes: %d\n", total_tx_count, total_bytes);
    printf("TCP Server Socket Close!\n");
    printf("----------------------------------------\n");

    close(clnt_sock);
    close(serv_sock);
    return 0;
}

void error_handling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
