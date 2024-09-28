#include "headers.h"

void print_file_details(char *path, struct dirent *d)
{
    struct stat fileStat;
    char fullpath[1024];
    snprintf(fullpath, sizeof(fullpath), "%s/%s", path, d->d_name);

    if (stat(fullpath, &fileStat) < 0)
    {
        perror("stat");
        return;
    }

    printf((S_ISDIR(fileStat.st_mode)) ? "d" : "-");
    printf((fileStat.st_mode & S_IRUSR) ? "r" : "-");
    printf((fileStat.st_mode & S_IWUSR) ? "w" : "-");
    printf((fileStat.st_mode & S_IXUSR) ? "x" : "-");
    printf((fileStat.st_mode & S_IRGRP) ? "r" : "-");
    printf((fileStat.st_mode & S_IWGRP) ? "w" : "-");
    printf((fileStat.st_mode & S_IXGRP) ? "x" : "-");
    printf((fileStat.st_mode & S_IROTH) ? "r" : "-");
    printf((fileStat.st_mode & S_IWOTH) ? "w" : "-");
    printf((fileStat.st_mode & S_IXOTH) ? "x" : "-");

    printf(" %ld", fileStat.st_nlink);

    struct passwd *pw = getpwuid(fileStat.st_uid);
    struct group *gr = getgrgid(fileStat.st_gid);
    printf(" %s %s", pw->pw_name, gr->gr_name);

    printf(" %5ld", fileStat.st_size);

    char timebuf[80];
    struct tm *timeinfo = localtime(&fileStat.st_mtime);
    strftime(timebuf, sizeof(timebuf), "%b %d %H:%M", timeinfo);
    printf(" %s", timebuf);

    if (lstat(fullpath, &fileStat) == 0)
    {
        if (S_ISDIR(fileStat.st_mode))
        {
            printf(COLOR_DIRECTORY);
        }
        else if (fileStat.st_mode & S_IXUSR)
        {
            printf(COLOR_EXECUTABLE);
        }
    }
    printf(" %s\n", d->d_name);
    printf(COLOR_FILE);
}

void reveal(char *path, int a, int l, char *home_dir)
{
    struct dirent **namelist;
    int n;

    char current_path[PATH_MAX]; // Use a buffer for getcwd

    if (strcmp(path, "~") == 0)
        path = home_dir;
    if (strcmp(path, "-") == 0)
        path = prev_dir;
    if (*path == '\0')
        path = getcwd(current_path, sizeof(current_path)); // Store the result in a buffer

    n = scandir(path, &namelist, NULL, alphasort);
    if (n < 0)
    {
        if (errno == ENOENT)
        {
            perror("Directory doesn't exist");
            return;
        }
        else
        {
            perror("Unable to read directory");
            return;
        }
    }

    for (int i = 0; i < n; i++)
    {
        if (!a && namelist[i]->d_name[0] == '.')
        {
            free(namelist[i]);
            continue;
        }
        if (l)
            print_file_details(path, namelist[i]);
        else
        {
            struct stat fileStat;
            char fullpath[1024];
            snprintf(fullpath, sizeof(fullpath), "%s/%s", path, namelist[i]->d_name);

            if (lstat(fullpath, &fileStat) == 0)
            {
                if (S_ISDIR(fileStat.st_mode))
                {
                    printf(COLOR_DIRECTORY);
                }
                else if (fileStat.st_mode & S_IXUSR)
                {
                    printf(COLOR_EXECUTABLE);
                }
            }
            printf("%s\n", namelist[i]->d_name);
            printf(COLOR_FILE);
        }

        free(namelist[i]); // Free each entry after use
    }

    free(namelist); // Free the namelist array itself
}