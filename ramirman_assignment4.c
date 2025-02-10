#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define INPUT_LENGTH 2048
#define MAX_ARGS	 512

int last_status = 0;                // Variable to track last fg process status

// Structure holds parsed command info (from provided sample_parser code)
struct command_line
{
	char *argv[MAX_ARGS + 1];       // Arg list
	int argc;                       // Arg count
	char *input_file;               // for input redirection
	char *output_file;              // for output redirection
	bool is_bg;                     // flag for background processes
};


// Function parses and reads user input (from provided sample_parser code)
struct command_line *parse_input()
{
	char input[INPUT_LENGTH];
    // Allocates memory for command_line struct
	struct command_line *curr_command = (struct command_line *) calloc(1, sizeof(struct command_line));

	// Get input
	printf(": ");
	fflush(stdout);

    // Read user input
	fgets(input, INPUT_LENGTH, stdin);

    // Ignore empty and comment lines
    if (input[0] == '\n' || input[0] == '#' || strlen(input) == 0)
    {
        free(curr_command);
        return NULL;
    }

	// Tokenize the input
	char *token = strtok(input, " \n");
	while (token)
    {
		if(!strcmp(token,"<")){
			curr_command->input_file = strdup(strtok(NULL," \n"));
		} else if(!strcmp(token,">")){
			curr_command->output_file = strdup(strtok(NULL," \n"));
		} else if(!strcmp(token,"&")){
			curr_command->is_bg = true;
		} else{
			curr_command->argv[curr_command->argc++] = strdup(token);
		}
		token=strtok(NULL," \n");
	}
	return curr_command;
}


// Function to help run built-in commands
bool run_builin(struct command_line *cmd)
{
    if (cmd->argc == 0)     // Returns false if no command is input
    {
        return false;
    }

    if (strcmp(cmd->argv[0], "exit") == 0)      // Handles exit command, checks for "exit" input
    {
        exit(0);
    }

    if (strcmp(cmd->argv[0], "cd") == 0)        // Executes cd command, checks for "cd" input
    {
        char *path = (cmd->argc > 1) ? cmd->argv[1] : getenv("HOME");       // Uses the given directory or goes to HOME if none input
        
        if (chdir(path) != 0)       // Changes working directory
        {
            perror("cd");           // prints error if fails
        }
        return true;
    }

    if (strcmp(cmd->argv[0], "status") == 0)        // Handles status command, checks for "status" input
    {
        if (WIFEXITED(last_status))                 // Uses last_status which has last foreground process exit status 
        {
            printf("exit value %d\n", WEXITSTATUS(last_status));        // Prints if exited normally
        }
        else
        {
            printf("terminated by signal %d\n", WTERMSIG(last_status));     // Prints signal if terminated by a signal
        }
        fflush(stdout);
        return true;
    }
    return false;       // Returns false for non-built-in commands
}

int main()
{
	struct command_line *curr_command;

	while(true)
	{
		curr_command = parse_input();

        // Continues to next prompt if command is NULL
        if (!curr_command)              
        {
            continue;
        }

        // Runs built in commands
        if (!run_builin(curr_command))
        {
            printf("Executing external command: %s\n", curr_command->argv[0]);
        }

        // For loop frees memory for parsed command
        for (int i=0; i<curr_command->argc; i++)
        {
            free(curr_command->argv[i]);
        }
        free(curr_command->input_file);
        free(curr_command->output_file);
        free(curr_command);

	}
	return EXIT_SUCCESS;
}
