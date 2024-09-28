#include "headers.h"
void ping_command(int pid, int signal_number)
{
    int signal = signal_number % 32;

    // Check if process exists
    if (kill(pid, 0) == -1)
    {
        if (errno == ESRCH)
        {
            printf("No such process found\n");
        }
        else
        {
            perror("Error checking process");
        }
        return;
    }

    // Send signal to process
    if (kill(pid, signal) == 0)
    {
        printf("Sent signal %d to process with pid %d\n", signal, pid);
    }
    else
    {
        perror("Failed to send signal");
    }
}

// Handle Ctrl-D (logout)
// void logout_shell()
// {

//     printf("Logging out, killing all processes...\n");
//     // Iterate over all processes (depends on how you store/manage background/foreground processes)
//     for (int i = 0; i < num_running_processes; i++)
//     {
//         kill(process_list[i], SIGKILL);
//     }
//     exit(0);
// }