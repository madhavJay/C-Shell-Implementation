#include "headers.h"

void hop(char *Path, char *home_dir)
{
    Path = str_replace(Path, "~", home_dir);
    char curr_dir[PATH_MAX];
    char change_dir[PATH_MAX];
    getcwd(curr_dir, PATH_MAX);
    char *token = strtok(Path, " \t");
    if (token == NULL)
    {

        token = home_dir;
        if (chdir(token) == -1)
        {
            perror("chdir() error");
            return;
        }

        printf("%s\n", getcwd(change_dir, PATH_MAX));
        return;
    }
    while (token != NULL)
    {
        if (strcmp(token, "-") == 0)
        {
            if (strlen(prev_dir) == 0)
            {
                fprintf(stderr, "No previous directory stored.\n");
                return;
            }
            token = prev_dir;
        }
        if (strcmp(token, "~") == 0)
        {
            token = home_dir;
        }

        if (strcmp(token, ".") == 0)
        {
        }

        if (chdir(token) == -1)
        {
            perror("chdir() error");
            return;
        }

        printf("%s\n", getcwd(change_dir, PATH_MAX));

        token = strtok(NULL, " \t");
    }
    strcpy(prev_dir, curr_dir);
}