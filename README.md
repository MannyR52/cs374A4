# CS 374 Portfolio Assignment ( SMALLSH )
SMALLSH Operating Systems Portfolio Project

## Introduction

In this assignment you will write your own shell in C called smallsh. smallsh will implement a subset of features of well-known shells, such as bash. Your program will

    Provide a prompt for running commands
    Handle blank lines and comments, which are lines beginning with the # character
    Execute 3 commands exit, cd, and status via code built into the shell
    Execute other commands by creating new processes using a function from the exec() family of functions
    Support input and output redirection
    Support running commands in foreground and background processes
    Implement custom handlers for 2 signals, SIGINT and SIGTSTP

## Learning Outcomes

After successful completion of this assignment, you should be able to do the following

    Describe the Linux process API (Module 6, MLO 2)
    Write programs using the Linux process API (Module 6, MLO 3)
    Explain the concept of signals and their uses (Module 6, MLO 2)
    Write programs using the Linux API for signal handling (Module 7, MLO 3)
    Explain I/O redirection and write programs that can employ I/O redirection (Module 7, MLO 4)

## Program Functionality

Be sure to read the complete assignment. The "Hints and Resources" section has useful tips on implementing various items of program functionality.
1. The Command Prompt

Use the colon : symbol as a prompt for each command line. 

The general syntax of a command line is:

command [arg1 arg2 ...] [< input_file] [> output_file] [&]

where items in square brackets are optional.

    You can assume that a command is made up of words separated by spaces.
    The special symbols <, > and & are recognized, but they must be surrounded by spaces like other words.
    If the command is to be executed in the background, the last word must be &. If the & character appears anywhere else, just treat it as normal text.
    If standard input or output is to be redirected, the > or < words followed by a filename word must appear after all the arguments. Input redirection can appear before or after output redirection.
    Your shell does not need to support any quoting; so arguments with spaces inside them are not possible. We are also not implementing the pipe "|" operator.
    Your shell must support command lines with a maximum length of 2048 characters, and a maximum of 512 arguments.
    You do not need to do any error checking on the syntax of the command line.

2. Comments & Blank Lines

Your shell must allow blank lines and comments.

    Any line that begins with the # character is a comment line and must be ignored. Mid-line comments, such as the C-style //, will not be supported.
    A blank line (one without any commands) must also do nothing.
    Your shell must just re-prompt for another command when it receives either a blank line or a comment line.

3. Built-in Commands

Your shell will support three built-in commands: exit, cd, and status. These three built-in commands are the only ones that your shell will handle itself - all others are simply passed on to a member of the exec() family of functions.

    You do not have to support input/output redirection for these built in commands
    These commands do not have to set any exit status.
    If the user tries to run one of these built-in commands in the background with the & option, ignore that option and run the command in the foreground anyway (i.e. don't display an error, just run the command in the foreground).

exit

The exit command exits your shell. It takes no arguments. When this command is run, your shell must kill any other processes or jobs that your shell has started before it terminates itself.
cd

The cd command changes the working directory of smallsh.

    By itself - with no arguments - it changes to the directory specified in the HOME environment variable
        This is typically not the location where smallsh was executed from, unless your shell executable is located in the HOME directory, in which case these are the same.
    This command can also take one argument: the path of a directory to change to. Your cd command must support both absolute and relative paths.

status

The status command prints out either the exit status or the terminating signal of the last foreground process ran by your shell.

    If this command is run before any foreground command is run, then it must simply return the exit status 0.
    The three built-in shell commands do not count as foreground processes for the purposes of this built-in command - i.e., status must ignore built-in commands.

4. Executing Other Commands

Your shell will execute any commands other than the 3 built-in command by using fork(), exec() and waitpid()

    Whenever a non-built in command is received, the parent (i.e., smallsh) will fork off a child.
    The child will use a function from the exec() family of functions to run the command.
    Your shell must use the PATH variable to look for non-built in commands, and it must allow shell scripts to be executed.
    If a command fails because the shell could not find the command to run, then the shell will print an error message and set the exit status to 1
    A child process must terminate after running a command (whether the command is successful or it fails).

5. Input & Output Redirection

You must do any input and/or output redirection using dup2(). The redirection must be done before using exec() to run the command.

    An input file redirected via stdin must be opened for reading only; if your shell cannot open the file for reading, it must print an error message and set the exit status to 1 (but don't exit the shell).
    Similarly, an output file redirected via stdout must be opened for writing only; it must be truncated if it already exists or created if it does not exist. If your shell cannot open the output file, it must print an error message and set the exit status to 1 (but don't exit the shell).
    Both stdin and stdout for a command can be redirected at the same time (see example below).

6. Executing Commands in Foreground & Background
Foreground Commands

Any command without an & at the end must be run as a foreground command and the shell must wait for the completion of the command before prompting for the next command. For such commands, the parent shell does NOT return command line access and control to the user until the child terminates.
Background Commands

Any non built-in command with an & at the end must be run as a background command and the shell must not wait for such a command to complete. For such commands, the parent must return command line access and control to the user immediately after forking off the child.

    The shell will print the process id of a background process when it begins.
    When a background process terminates, a message showing the process id and exit status will be printed. This message must be printed just before the prompt for a new command is displayed.
    If the user doesn't redirect the standard input for a background command, then standard input must be redirected to /dev/null.
    If the user doesn't redirect the standard output for a background command, then standard output must be redirected to /dev/null.

7. Signals SIGINT & SIGTSTP
SIGINT

A CTRL-C command from the keyboard sends a SIGINT signal to the parent process and all children at the same time (this is a built-in part of Linux).

    Your shell, i.e., the parent process, must ignore SIGINT.
    Any children running as background processes must ignore SIGINT.
    A child running as a foreground process must terminate itself when it receives SIGINT.
        The parent must not attempt to terminate the foreground child process; instead the foreground child (if any) must terminate itself on receipt of this signal.
        If a child foreground process is killed by a signal, the parent must immediately print out the number of the signal that killed it's foreground child process (see the example) before prompting the user for the next command.

SIGTSTP

A CTRL-Z command from the keyboard sends a SIGTSTP signal to your parent shell process and all children at the same time (this is a built-in part of Linux).

    A child, if any, running as a foreground process must ignore SIGTSTP.
    Any children running as background process must ignore SIGTSTP.
    When the parent process running the shell receives SIGTSTP
        The shell must display an informative message (see below) immediately if it's sitting at the prompt, or immediately after any currently running foreground process has terminated
        The shell then enters a state where subsequent commands can no longer be run in the background.
        In this state, the & operator must simply be ignored, i.e., all such commands are run as if they were foreground processes.
    If the user sends SIGTSTP again, then your shell will
        Display another informative message (see below) immediately after any currently running foreground process terminates
        The shell then returns back to the normal condition where the & operator is once again honored for subsequent commands, allowing them to be executed in the background.
   
### Running the program

- Compile using the following: gcc --std=gnu99 -o smallsh ramirman_assignment4.c
- Run in terminal using ./smallsh
