#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

void error_handling(char *message);

#define BUF_SIZE 100
#define TIME_REQ 0
#define TIME_RES 1

typedef struct
{
    int cmd;
    char time_msg[BUF_SIZE];
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
    printf("Connected..........\n");

    while (1)
    {
        PACKET packet;
        printf("Type a message(time or q): ");
        char cmd[50];
        scanf("%s", cmd);
        if (strcmp(cmd, "time") == 0)
        {
            packet.cmd = TIME_REQ;
            strcpy(packet.time_msg, "time");
            write(sock, &packet, sizeof(packet));
        }
        else if (strcmp(cmd, "q") == 0)
        {
            break;
        }
        else
        {
            printf("Wrong message.\n");
            continue;
        }
        str_len = read(sock, &packet, sizeof(packet));
        if (str_len == -1)
            error_handling("read() error!");
        if (packet.cmd == TIME_RES)
        {
            printf("[Client] Rx TIME_RES: %s\n", packet.time_msg);
        }
        else
        {
            printf("[Client] invalid packet!\n");
        }
    }

    close(sock);
    return 0;
}

void error_handling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
