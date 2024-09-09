#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
void error_handling(char *message);

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("[Error] mymove Usage: ./mymove src_file dest_file");
        return 1;
    }
    int src_fd = open(argv[1], O_RDONLY);
    char buf[10];
    int total = 0;
    int dst_fd = open(argv[2], O_WRONLY | O_CREAT, 0644);
    while (1)
    {
        int size = read(src_fd, buf, sizeof(buf));
        if (size == -1)
        {
            printf("read() fail, is it exist file?\n");
            return 2;
        }
        if (size == 0)
            break;
        total += size;

        int w = write(dst_fd, buf, size);
        if (w == -1)
        {
            printf("write() fail\n");
            return 3;
        }
    }
    close(src_fd);
    close(dst_fd);
    remove(argv[1]);
    printf("move from %s to %s (bytes: %d) finished.\n", argv[1], argv[2], total);
    return 0;
}
