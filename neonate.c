#include "headers.h"

int get_most_recent_pid()
{
    DIR *proc_dir;
    struct dirent *entry;
    pid_t most_recent_pid = 0;
    char path[256];
    FILE *status_file;
    unsigned long long latest_start_time = 0;
    unsigned long long process_start_time;
    char buffer[CMD_MAX];
    char *token;
    int field_count;
    int pid;

    proc_dir = opendir("/proc");
    if (proc_dir == NULL)
    {
        perror("opendir");
        return -1;
    }

    while ((entry = readdir(proc_dir)) != NULL)
    {
        if (entry->d_type == DT_DIR)
        {
            pid_t pid = atoi(entry->d_name);
            if (pid > 0)
            {
                snprintf(path, sizeof(path), "/proc/%d/stat", pid);
                status_file = fopen(path, "r");
                if (status_file)
                {

                    if (fgets(buffer, sizeof(buffer), status_file) != NULL)
                    {
                        // Parse the line to get the 22nd field
                        token = strtok(buffer, " ");
                        field_count = 0;
                        while (token != NULL)
                        {
                            field_count++;
                            if (field_count == 22)
                            {
                                // Extract the 22nd field
                                process_start_time = strtoull(token, NULL, 10);
                                if (process_start_time > latest_start_time)
                                {
                                    latest_start_time = process_start_time;
                                    most_recent_pid = pid;
                                }
                                break;
                            }
                            token = strtok(NULL, " ");
                        }
                    }
                    fclose(status_file);
                    if (process_start_time > latest_start_time)
                    {
                        latest_start_time = process_start_time;
                        most_recent_pid = pid;
                    }
                }
            }
        }
    }
    closedir(proc_dir);
    return most_recent_pid;
}
// Function to get single character input
int getch(void)
{
    struct termios oldt, newt;
    int ch;

    // Get the terminal attributes
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;

    // Set terminal to raw mode to read single characters
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    // Read a single character
    ch = getchar();

    // Restore the old terminal attributes
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

    return ch;
}

int kbhit(void)
{
    struct termios oldt, newt;
    int ch;
    int oldf;
    fd_set readfds;

    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);

    select(STDIN_FILENO + 1, &readfds, NULL, NULL, NULL);
    if (FD_ISSET(STDIN_FILENO, &readfds))
    {
        ch = getchar();
        fcntl(STDIN_FILENO, F_SETFL, oldf);
        return ch;
    }

    fcntl(STDIN_FILENO, F_SETFL, oldf);
    return 0;
}