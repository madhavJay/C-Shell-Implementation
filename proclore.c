#include "headers.h"

void print_process_info(int pid, int bg)
{
    char path[PATH_MAX];
    char status[100];
    char exe_path[PATH_MAX];
    char buffer[256];
    char process_status;
    int fd;

    pid_t pgid = getpgid(pid);
    if (pgid == -1)
    {
        perror("Error getting process group ID");
        return;
    }

    pid_t fgpgid = getpgrp();

    if (fgpgid == -1)
    {
        perror("Error getting foreground process group ID");
        return;
    }
    snprintf(path, sizeof(path), "/proc/%d/stat", pid);
    fd = open(path, O_RDONLY);
    if (fd == -1)
    {
        perror("Error opening stat file");
        return;
    }

    read(fd, buffer, sizeof(buffer));
    close(fd);

    char *token = strtok(buffer, " ");
    token = strtok(NULL, " ");
    token = strtok(NULL, " ");

    process_status = *token;

    snprintf(path, sizeof(path), "/proc/%d/statm", pid);
    fd = open(path, O_RDONLY);
    if (fd == -1)
    {
        perror("Error opening statm file");
        return;
    }

    read(fd, buffer, sizeof(buffer));
    close(fd);
    char *vm_size = strtok(buffer, " ");

    snprintf(path, sizeof(path), "/proc/%d/exe", pid);
    ssize_t len = readlink(path, exe_path, sizeof(exe_path) - 1);
    if (len != -1)
    {
        exe_path[len] = '\0';
    }
    else
    {
        perror("Error reading executable path");
        return;
    }

    printf("PID: %d\n", pid);
    pgid == fgpgid && !bg ? printf("Process Status: %c+\n", process_status) : printf("Process Status: %c\n", process_status);
    printf("Process Group: %d\n", pgid);
    printf("Virtual Memory: %s kB\n", vm_size);
    printf("Executable Path: %s\n", exe_path);
}