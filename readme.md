# smallsh
 	By: Terence Tang
 	tangte@oregonstate.edu
 	CS 344 Operating Systems
 	5/5/2021

## Included files:
    - smallsh.c
    - shell_input.c
    - shell_built_in_funcs.c
    - shell_executing_commands.c
    - shell_signal_handling.c
    - smallsh.h

## Description:
    Simple os shell application written in C. Features many common and helpful shell functions:
 	- Command prompt for entering and running command lines
	- Handling comments and blank lines.  Comments begin with a '#' char
	- Expansion of variable.  The $$ var is expanded to shell pid
	- Built-in commands for cd, exit, and status functions
	- Executes other commands via fork(), execvp(), and waitpid()
	- Input and output redirection
	- Supports running foreground and background processes
	- Custom signal handlers for catching SIGINT and SIGTSTP

## Environment Requirements:
    - smallsh implemented in C and compiled by GNU C Compiler (gcc)

## Installation
    Instructions on how to compile Smallsh:
	- Terminal Command for compiling -
    gcc --std=c99 -o smallsh src/smallsh.c src/shell_input_output.c src/shell_executing_commands.c  src/shell_built_in_funcs.c src/shell_signal_handling.c

    - Terminal Command for running 'smallsh' executable -
    ./smallsh

## Tests
	Test script provided for functionality demoing purposes.  
	- To run, please use the following command:
	./tests/testscript