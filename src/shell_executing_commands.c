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
extern int background_boolean;              // if true(1), then bg process cmds with '&' are allowed
extern int background_processes[];
extern int background_process_counter;
extern int status;                          // tracks foreground statuses


/* 
* Command router which routes the entered command to an appropriate function.  Will ignore
* blank lines and # comments and will route commands for the built-in functions cd, exit, 
* and status as well as other command line inputs.
*/
int command_router(struct command_input *currCommand)
{
    // ignores comments and empty input lines
    if (strcmp(currCommand->command, "\n") == 0 || currCommand->args[0][0] == '#')
    {
    }
    // routes for built in functions
    else if (strcmp(currCommand->command,"cd") == 0)
    {
        cmd_cd(currCommand->args);
    } 
    else if (strcmp(currCommand->command,"exit") == 0)
    {
        cmd_exit();
    }
    else if (strcmp(currCommand->command,"status") == 0)
    {
        cmd_status(status);
    }
    // else runs entered input as an other command via execvp()
    else 
    {
        exec_other_commands(currCommand);
    }
    return 1;
}

/* 
* Function for catching and executing all other command inputs from command line.
* Takes in the parsed command instructions via an arguments array from the command line
* and spawns child processes as appropriate.
*
* Handles foreground and background blocking calls and manages background child processes.
*/
void exec_other_commands(struct command_input *currCommand)
{
    int childStatus;

	// Fork a new process
	pid_t childPid = fork();

    // switch statements for error / child / and parent process instructions
	switch(childPid){
    // for errors
	case -1:
		perror("fork()\n");
		exit(1);
		break;
    // for child process
	case 0:
        set_child_signal_handlers(currCommand->background_flag);    // set signal handlers for child processes
        check_redirects(currCommand);                               // check for input/output redirects

        // execute new program via execvp, passes an array of command line args
        // checks if execvp() returns in error and exits as appropriate 
        if (execvp(currCommand->args[0], currCommand->args) == -1) {
            perror(currCommand->args[0]);
            exit(1);
        }
		break;
    // for parent process
	default:
        // check for if child is bg process and if bg processes are currently allowed
        if(currCommand->background_flag == 1 && background_boolean == 1) 
        {
            // if it is a background process, proceed with no hang and print child process pid
            waitpid(childPid, &childStatus, WNOHANG);
            printf("background pid is %d\n", childPid);
            fflush(stdout);

            // store background child pid for tracking
            background_processes[background_process_counter] = childPid;
            background_process_counter++;
        }
        // else handle child as foreground process
        else 
        {
            // if foreground process, wait for child termination
		    childPid = waitpid(childPid, &status, 0);
            fflush(stdout);
	    }

		// Loop to check for any terminated background child processes
        // loops until no pid is returned e.g. no new bg child process has terminated
		while((childPid = waitpid(-1, &childStatus, WNOHANG)) > 0)
        {
            // for each terminated process print id and status
            printf("background pid %d is done: ", childPid);
            fflush(stdout);
            cmd_status(childStatus); 

            // clear bg pid from background tracker array
            for (int i = 0; i < background_process_counter; i++)
            {
                if (background_processes[i] == childPid)
                {
                    background_processes[i] = 0;
                }
            }
        }
	}
}

/* 
* Checks for input and output redirect commands from commandline input.  If requested,
* attempts to open the new redirected file, creates a corresponding file descriptor and
* then points stdin and stdout to the file as appropriate.
* 
* Also handles cases for background child processes by routing stdin and stdout to /dev/null
*/
void check_redirects(struct command_input *currCommand)
{
    int dup_fd; // var for catching dup2() fd return

    // if background flag set - set stdin and stdout to '/dev/null' file first
    if(currCommand->background_flag == 1 && background_boolean == 1)
    {   
        // attempt to open source file
        char bg_redirect[] = "/dev/null";
        int sourceFD = open(bg_redirect, O_RDONLY);
        if (sourceFD == -1) { 
            perror("source open()"); 
            exit(1); 
        }

        // redirect stdin to source file
        dup_fd = dup2(sourceFD, 0);
        if (dup_fd == -1) { 
            perror("source dup2()"); 
            exit(1); 
        }
        close(sourceFD);

        // attempt to open destination file
        int targetFD = open(bg_redirect, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (targetFD == -1) { 
            perror("target open()"); 
            exit(1); 
        }
        // redirect stdout to target file
        dup_fd = dup2(targetFD, 1);
        if (dup_fd == -1) { 
            perror("target dup2()"); 
            exit(1);
        }
        close(targetFD);
    }

    // source redirect flag set, open source file and set
    if(currCommand->input_redirect != -1)
    {
        // attempt to open source file
        int sourceFD = open(currCommand->args[currCommand->input_redirect + 1], O_RDONLY);
        if (sourceFD == -1) { 
            fprintf(stderr, "cannot open %s for input\n", currCommand->args[currCommand->input_redirect + 1]);
            fflush(stdout);
            exit(1); 
        }
        // remove redirect symbol from args list
        currCommand->args[currCommand->input_redirect] = NULL;

        // redirect stdin to source file
        dup_fd = dup2(sourceFD, 0);
        if (dup_fd == -1) { 
            perror("source dup2()"); 
            exit(1); 
        }
        close(sourceFD);
    }

    // Target redirect flag set, open target file and set
    if(currCommand->output_redirect != -1)
    {
        // attempt to open target file
        int targetFD = open(currCommand->args[currCommand->output_redirect + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (targetFD == -1) { 
            fprintf(stderr, "cannot open %s for out\n", currCommand->args[currCommand->output_redirect + 1]);
            fflush(stdout);
            // perror("target open()"); 
            exit(1); 
        }
        // remove redirect symbol from args list
        currCommand->args[currCommand->output_redirect] = NULL;
    
        // redirect stdout to target file
        dup_fd = dup2(targetFD, 1);
        if (dup_fd == -1) { 
            perror("target dup2()"); 
            exit(1); 
        }
        close(targetFD);
    }
}


