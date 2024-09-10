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
    int serv_sock;
    socklen_t clnt_adr_sz;

    struct sockaddr_in serv_adr, clnt_adr;
    if (argc != 2)
    {
        printf("Usage : %s <port>\n", argv[0]);
        exit(1);
    }

    serv_sock = socket(PF_INET, SOCK_DGRAM, 0);
    if (serv_sock == -1)
        error_handling("UDP socket creation error");

    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port = htons(atoi(argv[1]));

    if (bind(serv_sock, (struct sockaddr *)&serv_adr, sizeof(serv_adr)) == -1)
        error_handling("bind() error");

    GAMEBOARD gboard;
    memset(gboard.board, 0, sizeof(gboard.board));
    printf("Tic-Tac-Toe Server\n");
    srand(time(NULL));

    while (1)
    {
        draw_board(&gboard);
        clnt_adr_sz = sizeof(clnt_adr);
        recvfrom(serv_sock, &gboard, sizeof(gboard), 0,
                 (struct sockaddr *)&clnt_adr, &clnt_adr_sz);
        draw_board(&gboard);
        if (availble_space(&gboard) == 0)
        {
            printf("No available space. Exit this program.\n");
            printf("Tic Tac Toe Server Close\n");
            break;
        }
        int row = -1, col = -1;
        while (!check_range(&gboard, row, col))
        {
            row = rand() % 3;
            col = rand() % 3;
        }
        sleep(1);
        gboard.board[row][col] = S_VALUE;
        printf("Server choose: [%d, %d]\n", row, col);
        sendto(serv_sock, &gboard, sizeof(gboard), 0,
               (struct sockaddr *)&clnt_adr, clnt_adr_sz);
    }
    close(serv_sock);
    return 0;
}

void error_handling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
