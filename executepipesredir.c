#include "headers.h"

int is_custom_command(const char *command_all)
{
    // List of custom commands
    const char *custom_commands[] = {
        "hop", "reveal", "log", "proclore", "seek", "activities", "ping", "fg", "bg", "neonate", "iman"};

    int num_custom_commands = sizeof(custom_commands) / sizeof(custom_commands[0]);

    for (int i = 0; i < num_custom_commands; i++)
    {
        if (strncmp(command_all, custom_commands[i], strlen(custom_commands[i])) == 0)
        {
            return 1;
        }
    }
    return 0;
}

void execute(char **cmds, int num_cmds, char *input_file, char *output_file, int append, char *home_dir)
{
    int i, in_fd = 0;
    int pd[2], status;
    pid_t pid;

    // Handle input redirection (if any)
    if (input_file)
    {
        in_fd = open(input_file, O_RDONLY);
        if (in_fd < 0)
        {
            perror("open input");
            exit(1);
        }
        dup2(in_fd, 0);
        close(in_fd);
    }

    for (i = 0; i < num_cmds - 1; i++)
    {
        pipe(pd);
        pid = fork();

        if (pid == 0)
        {
            dup2(pd[1], STDOUT_FILENO); // Redirect output to pipe
            close(pd[0]);               // Close read end of pipe in child
            close(pd[1]);
            remove_leading_spaces(cmds[i]);
            if (is_custom_command(cmds[i]) || i > 0)
                get_all(cmds[i], home_dir, 1, 0);
            else
            {
                char *args[] = {"/bin/sh", "-c", cmds[i], NULL};
                execvp(args[0], args);
            }

            exit(1);
        }

        // Parent process
        dup2(pd[0], STDIN_FILENO); // Redirect input from pipe
        close(pd[1]);              // Close write end of pipe in parent
        close(pd[0]);              // Close read end after dup2
    }

    pid = fork();

    if (pid == 0) // Child process for the last command
    {
        if (output_file) // Handle output redirection (if any)
        {
            int out_fd;
            if (append)
                out_fd = open(output_file, O_WRONLY | O_CREAT | O_APPEND, 0644);
            else
                out_fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);

            if (out_fd < 0)
            {
                perror("open output");
                exit(1);
            }
            dup2(out_fd, STDOUT_FILENO); // Redirect final output to the file
            close(out_fd);
        }

        // char *args[] = {"/bin/sh", "-c", cmds[i], NULL};
        // execvp(args[0], args); // Execute the last command
        // perror("execvp failed");
        get_all(cmds[i], home_dir, 1, 0);
        exit(1);
    }

    for (int j = 0; j < num_cmds; j++)
    {
        wait(NULL);
    }
}