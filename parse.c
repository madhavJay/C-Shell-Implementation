#include "headers.h"
void parse_command(char *input, char ***cmds, int *num_cmds, char **input_file, char **output_file, int *append)
{
    char *token;
    char *command[CMD_MAX];
    int cmd_index = 0;

    // Initialize
    *input_file = NULL;
    *output_file = NULL;
    *append = 0;

    // Split the input by '|' (pipes)
    token = strtok(input, "|");
    while (token != NULL)
    {
        command[cmd_index++] = strdup(token);
        token = strtok(NULL, "|");
    }

    *num_cmds = cmd_index;
    *cmds = malloc(sizeof(char *) * (*num_cmds));

    // Process each command, check for redirection in first and last commands
    for (int i = 0; i < *num_cmds; i++)
    {
        char *cmd = command[i];

        // Handle input redirection (<) only in the first command
        if (i == 0)
        {
            char *in = strstr(cmd, "<");
            if (in)
            {
                *in = '\0';                                    // Split the command and file
                *input_file = strdup(strtok(in + 1, " \t\n")); // Get input file
            }
        }

        // Handle output redirection (>, >>) only in the last command
        if (i == *num_cmds - 1)
        {
            char *out = strstr(cmd, ">");
            if (out)
            {
                if (*(out + 1) == '>')
                { // Append mode (>>)
                    *append = 1;
                    *out = '\0';                                     // Split the command and file
                    *output_file = strdup(strtok(out + 2, " \t\n")); // Get output file
                }
                else
                { // Overwrite mode (>)
                    *append = 0;
                    *out = '\0';                                     // Split the command and file
                    *output_file = strdup(strtok(out + 1, " \t\n")); // Get output file
                }
            }
        }

        // Save the cleaned-up command
        (*cmds)[i] = cmd;
    }
}

void print_parsed(char **cmds, int num_cmds, char *input_file, char *output_file, int append)
{
    printf("Parsed Commands:\n");
    for (int i = 0; i < num_cmds; i++)
    {
        printf("Command %d: %s\n", i + 1, cmds[i]);
    }
    if (input_file)
    {
        printf("Input file: %s\n", input_file);
    }
    if (output_file)
    {
        printf("Output file: %s (%s)\n", output_file, append ? "append" : "overwrite");
    }
}
