#include "headers.h"

void fg(int pid)
{
    // Check if the process exists
    if (kill(pid, 0) == -1)
    {
        printf("No such process found\n");
        return;
    }

    // Temporarily ignore SIGTTOU
    struct sigaction sa_ignore;
    sa_ignore.sa_handler = SIG_IGN;
    sigemptyset(&sa_ignore.sa_mask);
    sa_ignore.sa_flags = 0;
    sigaction(SIGTTOU, &sa_ignore, NULL);

    // Bring the process to the foreground
    tcsetpgrp(STDIN_FILENO, pid);

    // Resume the process if it was stopped
    if (kill(pid, SIGCONT) == -1)
    {
        perror("Failed to continue the process");
        return;
    }

    // Wait for the process to terminate or stop again
    int status;
    waitpid(pid, &status, WUNTRACED);

    // Restore terminal control to the shell
    tcsetpgrp(STDIN_FILENO, getpgrp());

    // Restore the default handling of SIGTTOU
    struct sigaction sa_default;
    sa_default.sa_handler = SIG_DFL;
    sigemptyset(&sa_default.sa_mask);
    sa_default.sa_flags = 0;
    sigaction(SIGTTOU, &sa_default, NULL);
}

void bg(int pid)
{
    if (kill(pid, 0) == -1)
    {
        printf("No such process found\n");
        return;
    }

    // Continue the process in the background
    setpgid(pid, getpgrp());
    if (kill(pid, SIGCONT) == -1)
    {
        perror("Failed to continue the process");
        return;
    }

    printf("Process [%d] continued in the background\n", pid);
}
