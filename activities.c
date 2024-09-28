#include "headers.h"

void print_process_info2(int pid, struct BgProc bg[1024], int bglen)
{
    char path[PATH_MAX];
    char buffer[256];
    char process_status;
    char exe_path[PATH_MAX];
    int fd;
    char proc_name[256];
    pid_t pgid;

    pgid = getpgid(pid);
    if (pgid == -1)
    {
        return; // Process doesn't exist or no permission
    }

    // Get the foreground process group ID (assuming it's your shell's group)
    pid_t fgpgid = getpgrp();
    if (fgpgid == -1)
    {
        perror("Error getting foreground process group ID");
        return;
    }

    // Read the process status from /proc/[pid]/stat
    snprintf(path, sizeof(path), "/proc/%d/stat", pid);
    fd = open(path, O_RDONLY);
    if (fd == -1)
    {
        return; // Process may have ended or permission denied
    }

    ssize_t bytes_read = read(fd, buffer, sizeof(buffer) - 1);
    if (bytes_read == -1)
    {
        close(fd);
        return;
    }
    buffer[bytes_read] = '\0'; // Null-terminate the buffer
    close(fd);

    // Extract the process status (3rd field)
    char *token = strtok(buffer, " "); // Skip pid
    token = strtok(NULL, " ");         // Skip command name
    token = strtok(NULL, " ");         // Get process status
    process_status = *token;

    snprintf(path, sizeof(path), "/proc/%d/comm", pid);
    FILE *file = fopen(path, "r");
    if (file)
    {
        if (fgets(proc_name, sizeof(proc_name), file))
        {
            // Remove newline character from end
            proc_name[strcspn(proc_name, "\n")] = '\0';
        }
        fclose(file);
    }
    else
    {
        perror("Failed to open file");
    }

    for (int i = 0; i < bglen; i++)
    {
        if (pid == bg[i].p_id)
            printf("%d : %s - %s\n", pid, bg[i].proc_name, (process_status == 'T') ? "Stopped" : "Running");
    }
}

void activities(struct BgProc bg[1024], int bglen)
{
    DIR *proc_dir;
    struct dirent *entry;

    proc_dir = opendir("/proc");
    if (!proc_dir)
    {
        perror("Unable to open /proc directory");
        return;
    }

    // Iterate over all entries in /proc
    while ((entry = readdir(proc_dir)) != NULL)
    {
        // Ensure it's a numeric directory name (indicating a PID)
        if (isdigit(entry->d_name[0]))
        {
            pid_t pid = atoi(entry->d_name);     // Convert directory name to PID
            print_process_info2(pid, bg, bglen); // Print process details
        }
    }

    closedir(proc_dir);
}
