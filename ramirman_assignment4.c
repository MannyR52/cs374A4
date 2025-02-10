#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define INPUT_LENGTH 2048
#define MAX_ARGS	 512


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
    if (input[0] == 'n' || input[0] == '#')
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
