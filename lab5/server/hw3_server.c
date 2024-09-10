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
    Packet packet;
    read(sock, &packet, sizeof(packet));
    char filename[100];
    strcpy(filename, packet.buf);
    FILE* file = fopen(packet.buf, "r");
    if(file == NULL)
    {
        packet.seq = -1;
        printf("%s File Not Found\n", packet.buf);
        write(sock, &packet, sizeof(packet));
        return;
    }
    packet.seq = SEQ_START;
    int sent = 0;
    printf("[Server] sending %s\n\n", packet.buf);
    while(1)
    {
        packet.buf_len = fread(packet.buf, sizeof(char), BUF_SIZE, file);
        write(sock, &packet, sizeof(packet));
        printf("[Server] Tx SEQ: %d, %d byte data\n", packet.seq, packet.buf_len);
        sent += packet.buf_len;
        if(feof(file)) break;
        read(sock, &packet, sizeof(packet));
        packet.seq = packet.ack;
        printf("[Server] Rx ACK: %d\n\n", packet.ack);
    }
    printf("%s sent (%d Bytes)\n", filename, sent);
}



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
    printf("--------------------\n");
    printf("   File Transmission Server\n");
    printf("--------------------\n");
    // SOCK_STREAM: TCP
    // SOCK_DGRAM: UDP
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

    clnt_addr_size = sizeof(clnt_addr);
    clnt_sock = accept(serv_sock, (struct sockaddr *)&clnt_addr, &clnt_addr_size);
    if (clnt_sock == -1)
        error_handling("accept() error");
    // TCP: write, UDP: sendto
    process(clnt_sock);
    printf("Server socket close\n");
    // close socket
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
