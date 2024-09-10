// 2019113928 정종운
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

void error_handling(char *message);

#define BUF_SIZE 1024
#define SEQ_START 0
typedef struct {
    int seq;
    int ack;
    int buf_len;
    char buf[BUF_SIZE];
}Packet;

void process(int sock)
{
    char filename[100];
    Packet packet;
    printf("Input file name: ");
    scanf("%s", filename);

    printf("[Client] request %s\n\n", filename);
    strcpy(packet.buf, filename);
    write(sock, &packet, sizeof(packet));

    FILE* file = NULL;
    int received = 0;

    while(1)
    {
        read(sock, &packet, sizeof(packet));
        if(packet.seq == -1)
        {
            printf("File Not Found\n");
            return;
        }
        if(received == 0)
        {
            file = fopen(filename, "w+");
        }
        received += packet.buf_len;
        printf("[Client] Rx SEQ: %d, len: %d bytes\n", packet.seq, packet.buf_len);
        
        packet.ack = packet.seq + packet.buf_len + 1;
        fwrite(packet.buf, sizeof(char), packet.buf_len, file);
        if(packet.buf_len < BUF_SIZE)
        {
            break;
        }
        write(sock, &packet, sizeof(packet));
        printf("[Client] Tx ACK: %d\n\n", packet.ack);
    }
    if(file != NULL)
    {
        fclose(file);
    }
    printf("%s received (%d Bytes)\n", filename, received);
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Usage : %s <IP> <port>\n", argv[0]);
        exit(1);
    }

    int sock;
    struct sockaddr_in serv_addr;
    // SOCK_STREAM: TCP
    // SOCK_DGRAM: UDP
    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock == -1)
        error_handling("socket() error");

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
        error_handling("connect() error!");
    // TCP: read, UDP: recvfrom
    process(sock);
    // close socket
    printf("Client socket close\n");
    close(sock);
    return 0;
}

void error_handling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
