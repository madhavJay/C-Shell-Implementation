#include "headers.h"

char log_buffer[MAX_LOG_SIZE][CMD_MAX];
struct alias alias_table[MAX_ALIASES];
struct BgProc Bg[1024];
int bg_count = 0;
int log_count = 0;
int log_start = 0;
int alias_count = 0;
char prompt[PATH_MAX];
int saved_stdin;
int saved_stdout;
int saved_stderr;
int current_fg = -1;
char *current_fg_name = "";

void save_file_descriptors()
{
    saved_stdin = dup(STDIN_FILENO);
    saved_stdout = dup(STDOUT_FILENO);
    saved_stderr = dup(STDERR_FILENO);
    if (saved_stdin == -1 || saved_stdout == -1 || saved_stderr == -1)
    {
        perror("dup failed");
    }
}

void reset_file_descriptors()
{
    if (dup2(saved_stdin, STDIN_FILENO) == -1)
    { // Restore stdin
        perror("Failed to reset stdin");
    }
    if (dup2(saved_stdout, STDOUT_FILENO) == -1)
    { // Restore stdout
        perror("Failed to reset stdout");
    }
    if (dup2(saved_stderr, STDERR_FILENO) == -1)
    { // Restore stderr
        perror("Failed to reset stderr");
    }

    // Close the saved descriptors as they're no longer needed
    close(saved_stdin);
    close(saved_stdout);
    close(saved_stderr);
}

int handle_redirection(char *command)
{
    int input_fd = -1, output_fd = -1, append = 0;
    char *input_file = NULL, *output_file = NULL;
    char *redir_input = strchr(command, '<');
    char *redir_output = strchr(command, '>');

    // Handle input redirection
    if (redir_input)
    {
        *redir_input = '\0'; // Terminate the command string
        redir_input++;       // Move to the file name

        input_file = strtok(redir_input, " \t\n");
        if (!input_file)
        {
            fprintf(stderr, "Error: No input file specified\n");
            return -1;
        }
    }

    // Handle output redirection (either > or >>)
    if (redir_output)
    {
        append = (*(redir_output + 1) == '>');
        *redir_output = '\0';             // Terminate the command string
        redir_output += (append ? 2 : 1); // Move to the file name

        output_file = strtok(redir_output, " \t\n");
        if (!output_file)
        {
            fprintf(stderr, "Error: No output file specified\n");
            return -1;
        }
    }

    // Open input file if necessary
    if (input_file)
    {
        input_fd = open(input_file, O_RDONLY);
        if (input_fd < 0)
        {
            perror("Error opening input file");
            return -1;
        }
        if (dup2(input_fd, STDIN_FILENO) < 0)
        {
            perror("Error redirecting input");
            return -1;
        }
    }

    // Open output file if necessary
    if (output_file)
    {
        int flags = O_WRONLY | O_CREAT;
        flags |= append ? O_APPEND : O_TRUNC;
        output_fd = open(output_file, flags, 0644);
        if (output_fd < 0)
        {
            perror("Error opening output file");
            return -1;
        }
        if (dup2(output_fd, STDOUT_FILENO) < 0)
        {
            perror("Error redirecting output");
            return -1;
        }
    }

    return 0;
}

void load_myshrc()
{
    FILE *file = fopen(".myshrc", "r");
    if (file == NULL)
    {
        perror("Failed to open .myshrc");
        return;
    }

    char line[CMD_MAX];
    while (fgets(line, sizeof(line), file))
    {
        if (strncmp(line, "alias", 5) == 0)
        {
            char *alias = strtok(line, " ");
            alias = strtok(NULL, "=");
            char *command = strtok(NULL, "\n");
            remove_leading_spaces(command);
            remove_trailing_spaces(command);
            remove_leading_spaces(alias);
            remove_trailing_spaces(alias);
            if (alias != NULL && 1)
            {
                strcpy(alias_table[alias_count].name, alias);
                strcpy(alias_table[alias_count].command, command);
                alias_count++;
            }
            else
            {
                printf("Invalid format. Expected 'alias a = b'\n");
            }
        }
        else if (strstr(line, "func") == line)
        {
            continue;
        }
    }
    fclose(file);
}

char *check_alias(char *input_command)
{
    for (int i = 0; i < alias_count; i++)
    {
        if (strcmp(input_command, alias_table[i].name) == 0)
        {
            return alias_table[i].command; // Return the alias command
        }
    }
    return input_command; // No alias, return original command
}

int is_foreground()
{
    int devtty;
    if ((devtty = open("/dev/tty", O_RDWR)) < 0)
    {
        return 0;
    }
    return 1;
}

void load_log()
{
    FILE *file = fopen(LOG_FILE, "r");
    if (file == NULL)
        return;

    while (fgets(log_buffer[log_count], CMD_MAX, file))
    {
        log_buffer[log_count][strcspn(log_buffer[log_count], "\n")] = 0;
        log_count = (log_count + 1) % MAX_LOG_SIZE;
    }
    fclose(file);
}

void save_log(char *home_dir)
{
    char log_path[PATH_MAX];
    snprintf(log_path, sizeof(log_path), "%s/%s", home_dir, LOG_FILE);

    FILE *file = fopen(log_path, "w");
    if (file == NULL)
    {
        perror("Failed to open log file");
        return;
    }

    for (int i = 0; i < log_count; i++)
    {
        fprintf(file, "%s\n", log_buffer[(log_start + i) % MAX_LOG_SIZE]);
    }
    fclose(file);
}

void add_to_log(const char *cmd, char *home_dir)
{
    if (log_count > 0 && strcmp(cmd, log_buffer[(log_start + log_count - 1) % MAX_LOG_SIZE]) == 0)
        return;

    if (log_count > 0 && strcmp(cmd, log_buffer[log_start]) == 0)
        return;

    if (log_count == MAX_LOG_SIZE)
    {
        log_start = (log_start - 1 + MAX_LOG_SIZE) % MAX_LOG_SIZE;
    }
    else
    {
        log_count++;
    }

    log_start = (log_start - 1 + MAX_LOG_SIZE) % MAX_LOG_SIZE;
    strncpy(log_buffer[log_start], cmd, CMD_MAX);

    save_log(home_dir);
}

void print_log()
{
    for (int i = log_count - 1; i >= 0; i--)
    {
        printf("%s\n", log_buffer[(log_start + i) % MAX_LOG_SIZE]);
    }
}

void purge_log(char *home_dir)
{
    log_count = 0;
    log_start = 0;
    save_log(home_dir);
}

char *execute_log_command(int index)
{
    char *cmd = log_buffer[(log_start + index - 1) % MAX_LOG_SIZE];
    return strdup(cmd);
}

int contains_log_command(const char *cmd)
{
    return strstr(cmd, "log") != NULL;
}

char prev_dir[PATH_MAX] = "";

void sigchld_handler(int signo)
{
    int status;
    pid_t pid;
    char *proc;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
    {
        if (WIFEXITED(status))
        {
            for (int i = 0; i < bg_count; i++)
            {
                if (Bg[i].p_id == pid)
                {
                    proc = Bg[i].proc_name;
                    Bg[i].p_id = -1;
                }
            }
            printf("%s exited normally (%d)\n", proc, pid);
        }
        else
        {
            for (int i = 0; i < bg_count; i++)
            {
                if (Bg[i].p_id == pid)
                {
                    proc = Bg[i].proc_name;
                }
            }
            printf("%s exited abnormally (%d)\n", proc, pid);
        }
    }
}
void sigtstp_handler(int sig)
{
    if (current_fg > 0)
    {
        printf("%d\n", current_fg);
        Bg[bg_count].p_id = current_fg;
        strcpy(Bg[bg_count++].proc_name, current_fg_name);
        kill(current_fg, SIGSTOP);
        current_fg = -1;
    }
    else
    {
        perror("No FG proc");
    }
}

void sigint_handler(int sig)
{
    if (current_fg > 0)
    {
        // Check if the process is still valid
        if (kill(current_fg, 0) == -1)
        {
            perror("No such process");
            return;
        }

        // Send SIGINT to the foreground process group
        if (kill(-current_fg, SIGINT) == -1)
        {
            perror("Failed to send SIGINT");
        }
        else
        {
            printf("SIGINT sent to foreground process %d\n", current_fg);
            current_fg = -1;
        }
    }
    else
    {
        printf("No foreground process to send SIGINT\n");
    }
}

void get_all(char *command, char *home_dir, int pipe, int log)
{
    if (!contains_log_command(command))
    {
        remove_leading_spaces(command);
        remove_trailing_spaces(command);
        add_to_log(command, home_dir);
    }

    char *command_all = NULL;
    char *next_command = NULL;
    char last_command[CMD_MAX] = "";
    long exec_time = -1;
    char *commands[CMD_MAX];
    int num_commands = 0;

    char *token = strtok(command, ";");
    while (token != NULL && num_commands < CMD_MAX)
    {
        commands[num_commands++] = strdup(token);
        token = strtok(NULL, ";");
    }

    for (int i = 0; i < num_commands; i++)
    {
        command_all = commands[i];
        char **cmds;
        int num_cmds = 0;
        char *inp_file;
        char *output_file;
        int append;
        parse_command(command_all, &cmds, &num_cmds, &inp_file, &output_file, &append);
        if (!log)
            save_file_descriptors();
        // char *command_dup = strdup(command_all);
        // next_command = strtok(NULL, ";");
        if (num_cmds > 1)
        {
            execute(cmds, num_cmds, inp_file, output_file, append, home_dir);
            reset_file_descriptors();
            continue;
        }
        command_all = commands[i];
        char *background_command = strchr(command_all, '&');
        if (background_command != NULL)
        {
            *background_command = '\0';
            background_command++;
            remove_leading_spaces(command_all);
            remove_leading_spaces(background_command);
            remove_trailing_spaces(background_command);
            remove_trailing_spaces(command_all);
            handle_redirection(command_all);

            pid_t pid = fork();
            if (pid == 0)
            {
                setsid();
                char *args[] = {"/bin/sh", "-c", command_all, NULL};
                execvp(args[0], args);
                perror("execvp failed");
                exit(1);
            }
            else if (pid > 0)
            {
                printf("%d\n", pid);
                Bg[bg_count].p_id = pid;
                strcpy(Bg[bg_count++].proc_name, command_all);
            }
            if (*background_command != '\0')
                commands[i--] = background_command;
            reset_file_descriptors();
            continue;
        }
        else
        {
            handle_redirection(command_all);
            remove_leading_spaces(command_all);
            remove_trailing_spaces(command_all);
            command_all = check_alias(command_all);
            if (strncmp(command_all, "hop", 3) == 0)
            {
                command_all += 3;
                if (*command_all != ' ' && *command_all != '\0')
                {
                    fprintf(stderr, "Error: Unrecognized Command encountered.\n");
                    command_all = next_command;
                    reset_file_descriptors();
                    continue;
                }
                remove_leading_spaces(command_all);
                hop(command_all, home_dir);
            }
            else if (strncmp(command_all, "reveal", 6) == 0)
            {
                command_all += 6;
                if (*command_all != ' ' && *command_all != '\0')
                {
                    fprintf(stderr, "Error: Unrecognized Command encountered.\n");
                    command_all = next_command;
                    reset_file_descriptors();
                    continue;
                }
                remove_leading_spaces(command_all);
                int flag_a = 0;
                int flag_l = 0;
                int invalid = 0;
                int dash = 0;

                while (*command_all == '-')
                {
                    dash = 1;
                    command_all++;
                    while (*command_all == 'a' || *command_all == 'l')
                    {
                        if (*command_all == 'a')
                        {
                            flag_a = 1;
                        }
                        else if (*command_all == 'l')
                        {
                            flag_l = 1;
                        }
                        command_all++;
                    }
                    if (*command_all != ' ' && *command_all != '\0' && *command_all != '-')
                    {
                        invalid = 1;
                    }
                    remove_leading_spaces(command_all);
                }
                if (invalid)
                {
                    fprintf(stderr, "Error: Unrecognized flag encountered.\n");
                    command_all = next_command;
                    reset_file_descriptors();
                    continue;
                }

                if (!invalid && !flag_a && !flag_l && dash)
                {
                    reveal("-", flag_a, flag_l, home_dir);
                    command_all = next_command;
                    reset_file_descriptors();
                    continue;
                }
                reveal(command_all, flag_a, flag_l, home_dir);
            }
            else if (strncmp(command_all, "log", 3) == 0)
            {
                command_all += 3;
                if (*command_all != ' ' && *command_all != '\0')
                {
                    fprintf(stderr, "Error: Unrecognized Command encountered.\n");
                    command_all = next_command;
                    reset_file_descriptors();
                    continue;
                }
                remove_leading_spaces(command_all);
                if (strcmp(command_all, "purge") == 0)
                {
                    purge_log(home_dir);
                }
                else if (strncmp(command_all, "execute ", 8) == 0)
                {
                    int index = atoi(command_all + 8);
                    if (index < 1 || index > log_count)
                    {
                        printf("Invalid log index\n");
                        reset_file_descriptors();
                        continue;
                    }
                    char *log_command = execute_log_command(index);
                    if (log_command)
                    {
                        printf("Executing: %s\n", log_command);
                        char temp_command[CMD_MAX];
                        strncpy(temp_command, log_command, CMD_MAX);
                        free(log_command);
                        get_all(temp_command, home_dir, 0, 1);
                    }
                }
                else
                {
                    print_log();
                }
            }
            else if (strncmp(command_all, "proclore", 8) == 0)
            {
                int pid;
                command_all += 8;
                if (*command_all != ' ' && *command_all != '\0')
                {
                    fprintf(stderr, "Error: Unrecognized Command encountered.\n");
                    command_all = next_command;
                    reset_file_descriptors();
                    continue;
                }
                remove_leading_spaces(command_all);
                if (*command_all != '\0')
                    pid = atoi(command_all);
                else
                    pid = getpid();
                int bg = 0;
                for (int i = 0; i < bg_count; i++)
                {
                    if (pid == Bg[i].p_id)
                    {
                        bg = 1;
                    }
                }
                print_process_info(pid, bg);
            }
            else if (strncmp(command_all, "seek", 4) == 0)
            {
                int search_files = 1, search_dirs = 1, execute_flag = 0;
                int invalid = 0;
                command_all += 4;
                if (*command_all != ' ' && *command_all != '\0')
                {
                    fprintf(stderr, "Error: Unrecognized Command encountered.\n");
                    command_all = next_command;
                    reset_file_descriptors();
                    continue;
                }
                remove_leading_spaces(command_all);
                if (*command_all == '-')
                {
                    search_files = search_dirs = 0;
                    while (*command_all == '-')
                    {
                        command_all++;
                        while (*command_all == 'e' || *command_all == 'f' || *command_all == 'd')
                        {
                            if (*command_all == 'e')
                            {
                                execute_flag = 1;
                            }
                            else if (*command_all == 'd')
                            {
                                search_dirs = 1;
                            }
                            else if (*command_all == 'f')
                            {
                                search_files = 1;
                            }
                            else
                                invalid = 1;

                            command_all++;
                        }
                        if (*command_all != ' ' && *command_all != '\0')
                        {
                            invalid = 1;
                            break;
                        }
                        remove_leading_spaces(command_all);
                    }
                    if (invalid)
                    {
                        fprintf(stderr, "Error: Unrecognized flag encountered.\n");
                        command_all = next_command;
                        reset_file_descriptors();
                        continue;
                    }
                    if (search_dirs == search_files && search_dirs != 0)
                    {
                        fprintf(stderr, "Error: -f and -d in seek.\n");
                        command_all = next_command;
                        reset_file_descriptors();
                        continue;
                    }
                    if (search_dirs == 0 && search_files == 0)
                    {
                        search_dirs = search_files = 1;
                    }
                }
                if (*command_all == '\0')
                {
                    fprintf(stderr, "Error: Missing target directory.\n");
                    command_all = next_command;
                    reset_file_descriptors();
                    continue;
                }
                char *tar = strtok(command_all, " \t");
                char *tar_dir = strtok(NULL, " \t");
                if (tar_dir == NULL)
                {
                    char curr_dir[PATH_MAX];
                    getcwd(curr_dir, sizeof(curr_dir));
                    int match_count = 0;
                    search_directory(command_all, curr_dir, search_files, search_dirs, execute_flag, home_dir, &match_count, curr_dir);
                    command_all = next_command;
                    reset_file_descriptors();
                    continue;
                }
                int match_count = 0;
                search_directory(command_all, tar_dir, search_files, search_dirs, execute_flag, home_dir, &match_count, tar_dir);
            }
            else if (strncmp(command_all, "activities", 10) == 0)
            {
                activities(Bg, bg_count);
            }
            else if (strncmp(command_all, "ping", 4) == 0)
            {
                command_all += 4;
                char *tar = strtok(command_all, " \t");
                char *tar_dir = strtok(NULL, " \t");
                int pid = atoi(tar);
                int signo = atoi(tar_dir);
                ping_command(pid, signo);
            }
            else if (strncmp(command_all, "fg", 2) == 0)
            {
                command_all += 2;
                remove_leading_spaces(command_all);
                remove_trailing_spaces(command_all);
                int pid = atoi(command_all);
                current_fg = pid;
                fg(pid);
                for (int i = 0; i < bg_count; i++)
                {
                    if (pid == Bg[i].p_id)
                    {
                        Bg[i].p_id = -1;
                        break;
                    }
                }
            }
            else if (strncmp(command_all, "bg", 2) == 0)
            {
                command_all += 2;
                remove_leading_spaces(command_all);
                remove_trailing_spaces(command_all);
                int pid = atoi(command_all);
                bg(pid);
            }
            else if (strncmp(command_all, "neonate", 7) == 0)
            {
                command_all += 7;
                remove_leading_spaces(command_all);
                remove_trailing_spaces(command_all);
                command_all += 3;
                remove_leading_spaces(command_all);
                int time = atoi(command_all);
                while (1)
                {
                    pid_t pid = get_most_recent_pid();
                    if (pid != -1)
                    {
                        printf("%d\n", pid);
                        fflush(stdout);
                    }
                    else
                    {
                        fprintf(stderr, "Error retrieving PID.\n");
                    }

                    sleep(time);

                    if (getch() == 'x')
                    {
                        break;
                    }
                }
            }
            else if (strncmp(command_all, "iMan", 4) == 0)
            {
                command_all += 4;
                remove_leading_spaces(command_all);
                remove_trailing_spaces(command_all);
                command_all = strtok(command_all, " \t");
                if (strlen(command_all) == 0 || command_all == NULL)
                {
                    fprintf(stderr, "Error: Invalid Command\n");
                }
                else
                {
                    fetchManPage(command_all);
                }
            }
            else
            {
                pid_t pid = fork();
                if (pid == 0)
                {
                    current_fg = pid;
                    current_fg_name = command_all;
                    char *args[] = {"/bin/sh", "-c", command_all, NULL};
                    tcsetpgrp(STDIN_FILENO, pid);
                    execvp(args[0], args);
                    perror("execvp failed");
                    exit(1);
                }
                else if (pid > 0)
                {

                    tcsetpgrp(STDIN_FILENO, getpid());
                    current_fg = pid;
                    current_fg_name = command_all;
                    time_t start, end;
                    time(&start);
                    int status;
                    waitpid(pid, &status, WUNTRACED);
                    time(&end);
                    exec_time = (long)difftime(end, start);
                    strncpy(last_command, command_all, CMD_MAX);

                    if (difftime(end, start) <= 2)
                    {
                        exec_time = -1;
                    }
                }
            }
        }
        command_all = next_command;
        if (!pipe)
            reset_file_descriptors();
        if (pipe)
            exit;
    }
    get_prompt(prompt, home_dir, last_command, exec_time);
}
int main()
{
    load_log();
    load_myshrc();
    char home_dir[PATH_MAX];
    char command[CMD_MAX];
    signal(SIGCHLD, sigchld_handler);
    signal(SIGINT, sigint_handler);
    signal(SIGTSTP, sigtstp_handler);

    getcwd(home_dir, sizeof(home_dir)); // Get the home directory

    get_prompt(prompt, home_dir, NULL, -1); // take input
    while (1)                               // run till user quites
    {
        printf("%s", prompt);
        if (fgets(command, sizeof(command), stdin) == NULL)
        {
            printf("\nExiting shell...\n");
            exit(1);
            break;
        }
        command[strcspn(command, "\n")] = 0; // remove all \n s
        get_all(command, home_dir, 0, 0);    // tokenize
    }

    return 0;
}