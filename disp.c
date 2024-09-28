#include "headers.h"

void get_prompt(char *prompt, char *home_dir, char *last_command, long exec_time)
{
    char current_dir[PATH_MAX];
    char relative_dir[PATH_MAX];
    char hostname[PATH_MAX];
    struct passwd *pw;
    uid_t uid;
    gethostname(hostname, PATH_MAX - 1);
    getcwd(current_dir, sizeof(current_dir)); // Get the current directory

    uid = geteuid();
    pw = getpwuid(uid); // Get the username
    char *username = pw->pw_name;
    char *system_name = hostname; // get the system name

    if (strncmp(current_dir, home_dir, strlen(home_dir)) == 0) // check if starting of current dir is same as that of homedir
    {
        snprintf(relative_dir, sizeof(relative_dir), "~%s", current_dir + strlen(home_dir)); // starts printing current_dir from end of home_dir length
    }
    else // have left home so use absolute path
    {
        strncpy(relative_dir, current_dir, sizeof(relative_dir));
    }

    // Include the last command and execution time in the prompt
    if (last_command && exec_time >= 0)
    {
        snprintf(prompt, PATH_MAX, "<%s@%s:%s %s : %lds> ", username, system_name, relative_dir, last_command, exec_time);
    }
    else
    {
        snprintf(prompt, PATH_MAX, "<%s@%s:%s> ", username, system_name, relative_dir);
    }
}
