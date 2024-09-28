#ifndef HEADERS_H_
#define HEADERS_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>
#include <limits.h>
#include <string.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <grp.h>
#include <time.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#include "disp.h"
#include "valdefs.h"
#include "utilities.h"
#include "hop.h"
#include "reveal.h"
#include "proclore.h"
#include "seek.h"
#include "parse.h"
#include "executepipesredir.h"
#include "main.h"
// #include "activities.h"
#include "signal.h"
#include "fgbg.h"
#include "neonate.h"
#include "iman.h"

extern char prev_dir[PATH_MAX];

struct BgProc
{
    int p_id;
    char proc_name[CMD_MAX];
};

struct alias
{
    char name[CMD_MAX];    // alias name
    char command[CMD_MAX]; // command mapped to the alias
};

void print_process_info2(int pid, struct BgProc bg[1024], int bglen);
void activities(struct BgProc bg[1024], int bglen);

#endif
