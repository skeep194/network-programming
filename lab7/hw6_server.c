// 2019113928 정종운
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/select.h>

#define BUF_SIZE 2048
void error_handling(char *buf);

int main(int argc, char *argv[])
{
    int serv_sock, clnt_sock;
    struct sockaddr_in serv_adr, clnt_adr;
    struct timeval timeout;
    fd_set reads, cpy_reads;

    socklen_t adr_sz;
    int fd_max, str_len, fd_num, i;
    char buf[BUF_SIZE];
    if(argc != 2) {
        printf("Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port = htons(atoi(argv[1]));

    if(bind(serv_sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr)) == -1)
        error_handling("bind() error");

    if(listen(serv_sock, 5) == -1)
        error_handling("listen() error");

    FD_ZERO(&reads);
    FD_SET(serv_sock, &reads);
    fd_max = serv_sock;

    int sender = -1;
    int receiver = -1;

    while(1)
    {
        cpy_reads = reads;
        timeout.tv_sec = 3;
        timeout.tv_usec = 3000;

        if((fd_num = select(fd_max+1, &cpy_reads, 0, 0, &timeout))== -1)
            break;
        if(fd_num == 0)
            continue;
        
        for(i=0; i<fd_max+1; i++)
        {
            if(FD_ISSET(i, &cpy_reads))
            {
                if(i == serv_sock)  // connection request
                {
                    adr_sz = sizeof(clnt_adr);
                    clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_adr, &adr_sz);
                    
                    if(fd_max < clnt_sock)
                        fd_max = clnt_sock;
                    printf("connected client: %d (fd_max: %d)\n", fd_max, 
							clnt_sock);
                    if(receiver == -1)
                    {
                        receiver = clnt_sock;
                        FD_SET(clnt_sock, &reads);
                    }
                    else if(sender == -1)
                    {
                        sender = clnt_sock;
                        FD_SET(clnt_sock, &reads);
                    }
                }
               else if(i == sender)
               {
                    str_len = read(i, buf, BUF_SIZE);
                    if(str_len == 0)
                    {
                        close(i);
                        printf("closed client: %d\n", i);
                        FD_CLR(i, &reads);
                    }
                    else
                    {
                        printf("Forward [%d] ----> [%d]\n", i, receiver);
                        write(receiver, buf, str_len); // echo
                    }
                }
                else if(i == receiver)
                {
                    str_len = read(i, buf, BUF_SIZE);
                    if(str_len == 0)
                    {
                        close(i);
                        printf("closed client: %d\n", i);
                        FD_CLR(i, &reads);
                    }
                    else
                    {
                        printf("Backward [%d] <---- [%d]\n", sender, i);
                        write(sender, buf, str_len); // echo
                    }
                }
            }
        }
    }
    close(serv_sock);
    return 0;
}

void error_handling(char *buf)
{
    fputs(buf, stderr);
    fputc('\n', stderr);
    exit(1);
}
