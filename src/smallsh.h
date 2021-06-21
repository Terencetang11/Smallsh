/* Name : Terence Tang
*  Course : CS344 - Operating Systems
*  Date : May 3, 2021
*  Assignment #3: Smallsh
*  Description:  Header file for defining the small shell library of functions 
*                used for this assignment.
*/


// Declaring Constants
#define MAX_BUFFER_SIZE 2048
#define ARG_BUFFER_SIZE 512
#define MAX_BG_PROCS 100

/* struct for input command information */
struct command_input
{
    char *command;
    char **args;
    int input_redirect;
    int output_redirect;
    int background_flag;
};

// Function Declarations
char* read_line();
struct command_input* parse_line(char *line);
int free_mem(char *line, struct command_input *currCommand);
char* replace_string(const char* string, const char* old_str, const char* new_str);
void exec_other_commands(struct command_input *currCommand);
int command_router(struct command_input *currCommand);
int cmd_cd(char **args);
int cmd_exit();
int cmd_status(int status);
void check_redirects(struct command_input *currCommand);
void handle_SIGTSTP();
void set_default_signal_handlers();
void set_child_signal_handlers(int process_bg_flag);
