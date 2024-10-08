// 2019113928 정종운

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

const int PARENT_TIMEOUT = 2;
const int CHILD_TIMEOUT = 5;
int elapsed_time = 0;
int child_alarm_count = 0;

void parent_timeout(int sig)
{
    elapsed_time += PARENT_TIMEOUT;
    printf("<Parent> time out: %d, elapsed time: %4d seconds\n", PARENT_TIMEOUT, elapsed_time);
    alarm(PARENT_TIMEOUT);
}

void parent_sigint(int sig)
{
    printf("SIGINT: Do you want to exit (y or Y to exit)?\n");
    char key;
    key = getchar();
    if (key == 'y' || key == 'Y')
    {
        exit(0);
    }
}

void parent_sigchld(int sig)
{
    int status;
    pid_t pid = waitpid(-1, &status, 0);
    printf("Child id: %d, sent: %d\n", pid, WEXITSTATUS(status));
}

void child_timeout(int sig)
{
    child_alarm_count++;
    elapsed_time += CHILD_TIMEOUT;
    printf("[Child] time out: %d, elapsed time: %4d seconds(%d)\n", CHILD_TIMEOUT, elapsed_time, child_alarm_count);
    if (child_alarm_count < 5)
    {
        alarm(CHILD_TIMEOUT);
    }
    else
    {
        exit(5);
    }
}

void parent()
{
    struct sigaction act;
    act.sa_handler = parent_timeout;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);

    sigaction(SIGALRM, &act, 0);

    struct sigaction act2;
    act2.sa_handler = parent_sigint;
    act2.sa_flags = 0;
    sigemptyset(&act2.sa_mask);

    sigaction(SIGINT, &act2, 0);

    struct sigaction act3;
    act3.sa_handler = parent_sigchld;
    act3.sa_flags = 0;
    sigemptyset(&act3.sa_mask);

    sigaction(SIGCHLD, &act3, 0);

    alarm(PARENT_TIMEOUT);
    while (1)
    {
        sleep(1);
    }
}

void child()
{
    struct sigaction act;
    act.sa_handler = child_timeout;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);

    sigaction(SIGALRM, &act, 0);
    alarm(CHILD_TIMEOUT);
    while (1)
    {
        sleep(100);
    }
}

int main()
{
    pid_t pid = fork();
    if (pid != 0)
    {
        printf("Parent process created\n");
        parent();
    }
    else
    {
        printf("Child process created.\n");
        child();
    }
}