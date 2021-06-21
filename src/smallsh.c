/* 
*  Name : Terence Tang
*  Course : CS344 - Operating Systems
*  Date : May 3, 2021
*  Assignment #3: Smallsh
*  Description:  Simple shell implementation featuring many common and helpful shell functionality, e.g. changing directories
*                running executables with given list of arguments, redirecting input/output, etc.
*/

// compiled using gcc option --std=c99

#define _GNU_SOURCE
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "smallsh.h"

// Declare Global Variables
int background_boolean = 1;                   // if true(1), then bg process cmds with '&' are allowed
int background_processes[MAX_BG_PROCS];
int background_process_counter = 0;
int status = 0;                                // tracks foreground statuses

int main()
{
    set_default_signal_handlers();              // sets default signal handlers for parent process

    // continues to loop for additional user command line inputs
    while(1)
    {
        char* line = read_line();
        struct command_input *currCommand = parse_line(line);
        command_router(currCommand);
        free_mem(line, currCommand);
    }
    return 0;
}
