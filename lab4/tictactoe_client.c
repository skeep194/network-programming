// 2019113928 정종운
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>

#define BUF_SIZE 30
void error_handling(char *message);

#define BOARD_SIZE 3
typedef struct
{
    int board[BOARD_SIZE][BOARD_SIZE];
} GAMEBOARD;

int availble_space(GAMEBOARD *gboard)
{
    for (int i = 0; i < BOARD_SIZE; ++i)
    {
        for (int j = 0; j < BOARD_SIZE; ++j)
        {
            if (gboard->board[i][j] == 0)
            {
                return 1;
            }
        }
    }
    return 0;
}

#define INIT_VALUE 0 // 초기값 0
#define S_VALUE 1    // 서버가 선택한 위치
#define C_VALUE 2    // 클라이언트가 선택한 위치 ...
void draw_board(GAMEBOARD *gboard)
{
    char value = ' ';
    int i, j;
    printf("+-----------+\n");
    for (i = 0; i < BOARD_SIZE; i++)
    {
        for (j = 0; j < BOARD_SIZE; j++)
        {
            if (gboard->board[i][j] == INIT_VALUE) // 초기값 0
                value = ' ';
            else if (gboard->board[i][j] == S_VALUE) // Server 표시 (1)
                value = 'O';
            else if (gboard->board[i][j] == C_VALUE) // Client 표시 2
                value = 'X';
            else
                value = ' ';
            printf("| %c ", value);
        }
        printf("|");
        printf("\n+-----------+\n");
    }
}

int check_range(GAMEBOARD *gboard, int row, int col)
{
    if (!(0 <= row && row < BOARD_SIZE && 0 <= col && col < BOARD_SIZE))
        return 0;
    return gboard->board[row][col] == INIT_VALUE;
}

int main(int argc, char *argv[])
{
    int sock;
    socklen_t adr_sz;

    struct sockaddr_in serv_adr, from_adr;
    if (argc != 3)
    {
        printf("Usage : %s <IP> <port>\n", argv[0]);
        exit(1);
    }

    sock = socket(PF_INET, SOCK_DGRAM, 0);
    if (sock == -1)
        error_handling("socket() error");

    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_adr.sin_port = htons(atoi(argv[2]));

    printf("Tic-Tac-Toe Client\n");
    GAMEBOARD gboard;
    memset(gboard.board, 0, sizeof(gboard.board));

    while (1)
    {
        draw_board(&gboard);

        int row = -1, col = -1;
        while (1)
        {
            printf("Input row, column: ");
            scanf("%d %d", &row, &col);
            if (check_range(&gboard, row, col))
                break;
            printf("Wrong index. Input again!\n");
        }

        gboard.board[row][col] = C_VALUE;
        draw_board(&gboard);
        sendto(sock, &gboard, sizeof(gboard), 0,
               (struct sockaddr *)&serv_adr, sizeof(serv_adr));

        if (!availble_space(&gboard))
        {
            printf("No available space. Exit Client\n");
            printf("Tic Tac Toe Client Close\n");
            break;
        }

        adr_sz = sizeof(from_adr);
        recvfrom(sock, &gboard, sizeof(gboard), 0,
                 (struct sockaddr *)&from_adr, &adr_sz);
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