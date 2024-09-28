#include "headers.h"

void print_colored(const char *path, int is_dir)
{
    if (is_dir)
    {
        printf("\033[1;34m%s/\033[0m\n", path); // Blue for directories
    }
    else
    {
        printf("\033[1;32m%s\033[0m\n", path); // Green for files
    }
}

int has_permission(const char *path, int is_dir)
{
    if (is_dir)
    {
        return access(path, X_OK) == 0;
    }
    else
    {
        return access(path, R_OK) == 0;
    }
}

void search_directory(const char *target, char *base_path, int search_files, int search_dirs, int execute_flag, char *home_dir, int *match_count, char *top_path)
{
    struct dirent *entry;

    base_path = str_replace(base_path, "~", home_dir);
    top_path = str_replace(top_path, "~", home_dir);

    if (*base_path == '~')
    {
        base_path = home_dir;
    }
    if (strcmp(base_path, "-") == 0)
    {
        base_path = prev_dir;
    }

    DIR *dp = opendir(base_path);

    if (dp == NULL)
    {
        perror("opendir");
        return;
    }

    static char *single_match = NULL;
    static int single_match_is_dir = 0;

    while ((entry = readdir(dp)))
    {
        char path[1024];
        snprintf(path, sizeof(path), "%s/%s", base_path, entry->d_name);

        struct stat path_stat;
        stat(path, &path_stat);
        int is_dir = S_ISDIR(path_stat.st_mode);

        if (startsWith(target, entry->d_name) == 1)
        {
            if ((is_dir && search_dirs) || (!is_dir && search_files))
            {
                // Construct the relative path from base_path
                char relative_path[1024];
                snprintf(relative_path, sizeof(relative_path), "%s/%s", base_path, entry->d_name);
                print_colored(relative_path + strlen(top_path) + 1, is_dir);

                (*match_count)++;
                if (*match_count == 1)
                {
                    single_match = strdup(path);
                    single_match_is_dir = is_dir;
                }
                else
                {
                    free(single_match);
                    single_match = NULL;
                }
            }
        }

        if (is_dir && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0 && entry->d_name[0] != '.')
        {
            search_directory(target, path, search_files, search_dirs, execute_flag, home_dir, match_count, top_path);
        }
    }

    closedir(dp);
    // Execute the match only after the entire directory tree has been searched
    if (execute_flag && *match_count == 1 && single_match != NULL)
    {
        if (has_permission(single_match, single_match_is_dir))
        {
            if (single_match_is_dir)
            {
                if (chdir(single_match) == 0)
                {
                    printf("%s/\n", single_match + strlen(base_path) + 1);
                }
                else
                {
                    perror("chdir");
                }
            }
            else
            {
                FILE *file = fopen(single_match, "r");
                if (file)
                {
                    char line[256];
                    while (fgets(line, sizeof(line), file))
                    {
                        printf("%s\n", line);
                    }
                    fclose(file);
                }
                else
                {
                    perror("fopen");
                }
            }
        }
        else
        {
            printf("Missing permissions for task!\n");
        }

        free(single_match);
        single_match = NULL;
    }
    if (*match_count == 0 && strcmp(base_path, top_path) == 0)
    {
        printf("No match found!\n");
    }
}
