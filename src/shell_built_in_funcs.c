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
extern int background_processes[];
extern int background_process_counter;
extern int status;          // tracks foreground statuses


/* 
* Shell built-in command for changing directories.  Takes list of command line arguments
* and updates current dir to entered path.  If no path provided, moves dir to HOME dir.
*/
int cmd_cd(char **args)
{
    char cwd[1024];         // var to store directory string

    // if no path arguments found, changes dir to HOME
    if (args[1] == NULL) 
    {
        if (chdir(getenv("HOME")) != 0) 
        {
            perror("smallsh");
        } 
    } 
    // else change dir to the entered directory
    else 
    {
        if (chdir(args[1]) != 0) {
            perror("smallsh");
        }
    }

    // print new directory path
    getcwd(cwd, sizeof(cwd));
    printf("%s\n", cwd);
    fflush(stdout);
    return 1;
}

/* 
* Shell built-in command for printing the last status of the most recently terminated process.
* Checks if the terminated status was an exit status or a termination signal and prints an appropriate
* message for each.
*/
int cmd_status(int status)
{   
    // if status registers as exited, print out the exit status
    if(WIFEXITED(status))
    {
        printf("exit value %d\n", WEXITSTATUS(status));
    }
    // else status caused by termination, print out termination signal
    else
    {
        printf("terminated by signal %d\n", WTERMSIG(status));
    }
    return 1;
}

/* 
* Shell built-in command for exiting the shell.  Handles terminating each open child process before
* exiting the shell.
*/
int cmd_exit()
{
    // scan through list of background tasks and terminate
    for (int i = 0; i < background_process_counter; i++)
    {
        kill(background_processes[i], SIGTERM);
    }

    exit(0);    // only exit after all other child processes ended
}

