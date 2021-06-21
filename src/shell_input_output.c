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


/*
*   Process the file provided as an argument to the program to
*   create a linked list of student structs and print out the list.
*   Compile the program as follows:
*       gcc --std=gnu99 -o smallsh main.c
*/


/* 
* Prompts user for input and reads line from standard input via get line.
* Returns pointer to stored input.
*/
char* read_line()
{   
    // display console prompt
    printf(": ");
    fflush(stdout);

    // set up variables - getline will allocate memory for lineptr and set buffersize
    char *line = malloc(MAX_BUFFER_SIZE * sizeof(char) + 1);
    size_t buffersize = 0;

    // get user input via commandline
    if (fgets(line, MAX_BUFFER_SIZE, stdin) == NULL){
        // if getline returns in error, check for EOF and or error code and exit
        if (feof(stdin)) {
            exit(EXIT_SUCCESS);  // We recieved an EOF
        } else  {
            perror("read_line");
            exit(EXIT_FAILURE);
        }
    }
    return line;
}

/* 
* Parses user input and reads each token into a command_input struct which 
* stores the top level command of input and the follow-up arguments.
* Returns command_input struct
*/
struct command_input* parse_line(char *line)
{
    int index = 0;                          // tracks index of CL arguments entered
    char *saveptr;                          // For use with strtok_r

    // set up variables for $$ replacement
    char *pid = malloc(12);                 // Str version of parent pid for replacement
    sprintf(pid, "%d", getpid());
    fflush(stdout);
    char *expansionStr = "$$";

    // initialize struct for storing command info, initial values for redirect flags and bg process set
    struct command_input *currCommand = malloc(sizeof(struct command_input));
    currCommand->input_redirect = -1;
    currCommand->output_redirect = -1;
    currCommand->background_flag = 0;

    // The first token is the input command, if NULL or blank line, then returns appropriate command struct
    char *token = strtok_r(line, " \n", &saveptr);
    if (token == NULL)
    {
        currCommand->command = calloc(2 + 1, sizeof(char));
        strcpy(currCommand->command, "\n");
        return currCommand;
    }

    // if input line is not blank, proceeds to capture command and argument inputs
    currCommand->command = calloc(strlen(token) + 1, sizeof(char));
    strcpy(currCommand->command, token);

    // set up array of char pointers to store arguments from line using tokens
    currCommand->args = malloc(ARG_BUFFER_SIZE * sizeof(char*));

    while (token != NULL)
    {
        fflush(stdout);
        // check for input or output redirect characters and mark index as appropriate
        if (strcmp(token, "<") == 0)
        {
            currCommand->input_redirect = index;
        }
        else if (strcmp(token, ">") == 0)
        {
            currCommand->output_redirect = index;
        }
        
        // if $$ encountered, expand / replace string and free memory allocation used for replacement
        if (strstr(token, expansionStr) != NULL)
        {
            token = replace_string(token, expansionStr, pid);
            currCommand->args[index] = calloc(strlen(token) + 1, sizeof(char));
            strcpy(currCommand->args[index], token);;
            free(token);
        }
        // else copy token over to our args pointer array
        else 
        {
            currCommand->args[index] = calloc(strlen(token) + 1, sizeof(char));
            strcpy(currCommand->args[index], token);;
        }

        // scan next argument
        index++;
        token = strtok_r(NULL, " \n", &saveptr);
    }

    // check for bg process flag - mark bg bool and removes '&' char from args list
    if (strcmp(currCommand->args[index-1], "&") == 0)
    {
        currCommand->background_flag = 1;
        currCommand->args[index-1] = NULL;
    }

    // sets last element to null value and returns struct
    currCommand->args[index] = NULL;
    return currCommand;
}

/* 
* Frees all dynamically allocated memory elements as part of the current command input
* Includes freeing memory for input line, the command_input struct, and it's args array
*/
int free_mem(char *line, struct command_input *currCommand)
{
    // resets command line input datastructure
    memset(line, '\0', strlen(line));
    int index = 0;

    // checks if command was empty line, if not, free up all allocated mem
    if (strcmp(currCommand->command, "\n") != 0 )
    {
        char* arg = currCommand->args[index];
        while (arg != NULL)
        {
            free(arg);
            index++;
            arg = currCommand->args[index];
        }
    }
    free(currCommand->command);
    free(currCommand);
    return 1;
}

/* 
* Function to replace a substring with another value, all within another string (haystack)
*/
char* replace_string(const char* string, const char* old_str, const char* new_str)
{
    char *result;
    int i, count = 0;
    int newStrLen = strlen(new_str);
    int oldStrLen = strlen(old_str);
  
    // Counting the number of times the old string occurs in string (before null char)
    for (i = 0; string[i] != '\0'; i++) {
        if (strstr(&string[i], old_str) == &string[i]) {
            count++;
  
            // jumps to index past old str, tracks string's len without the old str chars
            i += oldStrLen - 1;
        }
    }
  
    // Making new string of enough length
    result = (char*)malloc(i + count * (newStrLen - oldStrLen) + 1);
  
    i = 0;
    while (*string) {
        // compare the substring with the result and replace w/ new_str (pid)
        if (strstr(string, old_str) == string) {
            strcpy(&result[i], new_str);
            i += newStrLen;
            string += oldStrLen;
        }
        // else copy over string char by char
        else result[i++] = *string++;
        
    }
  
    // adds null char and returns new string
    result[i] = '\0';
    return result;
}