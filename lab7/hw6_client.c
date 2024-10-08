// 2019113928 정종운
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/select.h>

#define BUF_SIZE 2048
void error_handling(char *message);
char buffer[BUF_SIZE];

void sender(char* argv[])
{
    int fd1, fd2;
    fd1=open("rfc1180.txt", O_RDONLY);
	fd2=socket(PF_INET, SOCK_STREAM, 0);
	if(fd2==-1)
		error_handling("socket() error");
	struct sockaddr_in serv_adr;
	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family=AF_INET;
	serv_adr.sin_addr.s_addr=inet_addr(argv[1]);
	serv_adr.sin_port=htons(atoi(argv[2]));

    printf("\nFile Sender Start!\n");

    if(connect(fd2, (struct sockaddr*)&serv_adr, sizeof(serv_adr))==-1)
		error_handling("connect() error!");
	else
		puts("Connected...........");

    printf("fd1: %d, fd2: %d\n", fd1, fd2);
    int max_fd = fd1 < fd2 ? fd2 : fd1;
    printf("max_fd: %d\n", max_fd);

    fd_set reads;
    FD_ZERO(&reads);
    FD_SET(fd1, &reads);
    fd_set writes;
    FD_ZERO(&writes);

    //timeout은 3초
    struct timeval timeout;
    timeout.tv_sec = 3;
    timeout.tv_usec = 3000;

    int read_bytes;

    while(1)
    {
        fd_set cpy_reads = reads;
        fd_set cpy_writes = writes;
        int fd_num;
        if((fd_num = select(max_fd+1, &cpy_reads, &cpy_writes, 0, &timeout))== -1)
            break;
        if(fd_num == 0)
            continue;

        for(int i=0;i<max_fd+1;i++)
        {
            if(FD_ISSET(i, &cpy_reads))
            {
                if(i == fd1)
                {
                    read_bytes = read(i, buffer, sizeof(buffer));
                    if(read_bytes == 0)
                    {
                        close(fd1);
                    }
                    FD_CLR(fd1, &reads);
                    FD_SET(fd2, &writes);
                }
                else if(i == fd2)
                {
                    read_bytes = read(i, buffer, sizeof(buffer));
                    if(read_bytes == 0)
                    {
                        close(fd2);
                    }
                    FD_CLR(fd2, &reads);
                    FD_SET(fd1, &reads);
                    write(1, buffer, read_bytes);
                }
            }
        }
        for(int i=0;i<max_fd+1;i++)
        {
            if(FD_ISSET(i, &cpy_writes))
            {
                if(i == fd2)
                {
                    if(read_bytes > 0)
                    {
                        write(i, buffer, read_bytes);
                        sleep(1);
                    }
                    FD_CLR(fd2, &writes);
                    FD_SET(fd2, &reads);
                }
            }
        }
    }
}

void receiver(char* argv[])
{
    int fd2;
	fd2=socket(PF_INET, SOCK_STREAM, 0);
	if(fd2==-1)
		error_handling("socket() error");
	struct sockaddr_in serv_adr;
	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family=AF_INET;
	serv_adr.sin_addr.s_addr=inet_addr(argv[1]);
	serv_adr.sin_port=htons(atoi(argv[2]));

    printf("\nFile Receiver Start!\n");

    if(connect(fd2, (struct sockaddr*)&serv_adr, sizeof(serv_adr))==-1)
		error_handling("connect() error!");
	else
		puts("Connected...........");

    printf("fd2: %d\n", fd2);
    int max_fd = fd2;
    printf("max_fd: %d\n", max_fd);

    fd_set reads, cpy_reads, writes, cpy_writes;
    FD_ZERO(&reads);
    FD_SET(fd2, &reads);

    //timeout은 3초
    struct timeval timeout;
    timeout.tv_sec = 3;
    timeout.tv_usec = 3000;

    int read_size;

    while(1)
    {
        cpy_reads = reads;
        cpy_writes = writes;
        int fd_num;
        if((fd_num = select(max_fd+1, &cpy_reads, &cpy_writes, 0, &timeout))== -1)
            break;
        if(fd_num == 0)
            continue;
        for(int i=0;i<max_fd+1;i++)
        {
            if(FD_ISSET(i, &cpy_reads))
            {
                if(i == fd2)
                {
                    read_size = read(i, buffer, sizeof(buffer));
                    FD_CLR(fd2, &reads);
                    FD_SET(fd2, &writes);
                }
            }
        }
        for(int i=0;i<max_fd+1;i++)
        {
            if(FD_ISSET(i, &cpy_writes))
            {
                if(i == fd2)
                {
                    write(1, buffer, read_size);
                    write(i, buffer, read_size);
                    FD_CLR(fd2, &writes);
                    FD_SET(fd2, &reads);
                }
            }
        }
    }
    close(fd2);
}

int main(int argc, char *argv[])
{
	if(argc!=3) {
		printf("Usage : %s <IP> <port>\n", argv[0]);
		exit(1);
	}

    printf("--------------------------------------------\n");
    printf(" Choose function\n");
    printf(" 1. Sender,   2. Receiver\n");
    printf("--------------------------------------------\n");
    printf(" ==>");
    int fun;
    scanf("%d", &fun);
    if(fun == 1)
    {
        sender(argv);
    }
    else if(fun == 2)
    {
        receiver(argv);
    }
    else
    {
        printf("invalid function number!\n");
    }
	return 0;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}