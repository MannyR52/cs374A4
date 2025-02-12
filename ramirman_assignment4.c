#include <signal.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>


#define INPUT_LENGTH 2048
#define MAX_ARGS	 512

int last_status = 0;                // Variable to track last fg process status

pid_t bg_pids[MAX_ARGS];            // Tracks background process PIDs
int bg_pid_count = 0;

bool allow_bg = true;               // Helps toggle bg execution

// Structure holds parsed command info (from provided sample_parser code)
struct command_line
{
	char *argv[MAX_ARGS + 1];       // Arg list
	int argc;                       // Arg count
	char *input_file;               // for input redirection
	char *output_file;              // for output redirection
	bool is_bg;                     // flag for background processes
};


// Signal handler for SIGSTP Crtl + Z
void handle_SIGSTP(int signo)
{
    if (allow_bg)
    {
        write(STDOUT_FILENO, "\nEntering foreground-only mode (& is now ignored)\n", 51);
        allow_bg = false;
    }
    else
    {
        write(STDERR_FILENO, "\nExiting foreground-only mode\n", 30);
        allow_bg = true;
    }

    fflush(stdout);
}


// Function to help with signal handlers
void signal_handler_helper()
{
    struct sigaction SIGINT_action = {0};
    SIGINT_action.sa_handler = SIG_IGN;
    sigaction(SIGINT, &SIGINT_action, NULL);

    struct sigaction SIGTSTP_action = {0};
    SIGTSTP_action.sa_handler = handle_SIGSTP;
    sigfillset(&SIGTSTP_action.sa_mask);
    SIGTSTP_action.sa_flags = SA_RESTART;
    sigaction(SIGTSTP, &SIGTSTP_action, NULL);
}


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
bool run_builtin(struct command_line *cmd)
{
    if (cmd->argc == 0)     // Returns false if no command is input
    {
        return false;
    }

    if (!strcmp(cmd->argv[0], "exit"))      // Handles exit command, checks for "exit" input
    {
        for (int i=0; i<bg_pid_count; i++)
        {
            kill(bg_pids[i], SIGTERM);          // Terminates bg processes
        }
        exit(0);
    }

    if (!strcmp(cmd->argv[0], "cd"))        // Executes cd command, checks for "cd" input
    {
        char *path = (cmd->argc > 1) ? cmd->argv[1] : getenv("HOME");       // Uses the given directory or goes to HOME if none input
        
        if (chdir(path) != 0)       // Changes working directory
        {
            perror("cd");           // prints error if fails
        }
        return true;
    }

    if (!strcmp(cmd->argv[0], "status"))        // Handles status command, checks for "status" input
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

void execute_other_commands(struct command_line *cmd)
{
    pid_t spawn_pid = fork();

    if (spawn_pid == -1)
    {
        perror("fork");
        exit(1);
    }

    if (spawn_pid == 0)             // This is child process
    {
        if (!cmd->is_bg)
        {
            struct sigaction SIGINT_child = {0};
            SIGINT_child.sa_handler = SIG_DFL;
            sigaction(SIGINT, &SIGINT_child, NULL);
        }
        
        if (cmd->input_file)        // Input redirection
        {
            int input_fd = open(cmd->input_file, O_RDONLY);
            if (input_fd == -1)
            {
                fprintf(stderr, "cannot open %s for input\n", cmd->input_file);
                exit(1);
            }
            dup2(input_fd, 0);
            close(input_fd);
        }
        else if (cmd->is_bg)        // bg, w/ no input file, redirects to /dev/null
        {
            int dev_null = open("/dev/null", O_RDONLY);
            if (dev_null == -1)
            {
                perror("cannot open /dev/null for input");
                exit(1);
            }
            dup2(dev_null, 0);
            close(dev_null);
        }
        

        if (cmd->output_file)       // Output redirection
        {
            int output_fd = open(cmd->output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (output_fd == -1)
            {
                fprintf(stderr, "cannot open %s for output\n", cmd->output_file);
                exit(1);
            }
            dup2(output_fd, 1);
            close(output_fd);
        }
        else if (cmd->is_bg)        // bg w/ no output file, redirects to /dev/null
        {
            int dev_null = open("/dev/null", O_RDONLY);
            if (dev_null == -1)
            {
                perror("cannot open /dev/null for output");
                exit(1);
            }
            dup2(dev_null, 1);
            close(dev_null);
        }

        execvp(cmd->argv[0], cmd->argv);        // Execute command
        fprintf(stderr, "%s: no such file or directory found\n", cmd->argv[0]);
        exit(1);
    }
    else        // For parent process
    {
        if (cmd->is_bg)     // For background process
        {
            printf("background pid is %d\n", spawn_pid);
            fflush(stdout);
            bg_pids[bg_pid_count++] = spawn_pid;                // Stores bg process PID
        }
        else                // For foreground process
        {
            waitpid(spawn_pid, &last_status, 0);
            if (WIFSIGNALED(last_status))
            {
                printf("terminated by signal %d\n", WTERMSIG(last_status));
                fflush(stdout);
            }
        }
    }
}


// Function to help check for bg processes
void check_bg_proc()
{
    int i, new_count = 0;
    for (int i=0; i<bg_pid_count; i++)
    {
        int status;
        pid_t pid = waitpid(bg_pids[i], &status, WNOHANG);

        if (pid > 0)
        {
            if (WIFEXITED(status))
            {
                printf("background pid %d is done: exit value %d\n", pid, WEXITSTATUS(status));
            }
            else
            {
                printf("background pid %d is done: terminated by signal %d\n", pid, WTERMSIG(status));
            }
            fflush(stdout);
            bg_pids[i] = bg_pids[--bg_pid_count];
        }
    }
}


int main()
{
	signal_handler_helper();
    struct command_line *curr_command;

	while(true)
	{
		check_bg_proc();
        
        curr_command = parse_input();

        // Continues to next prompt if command is NULL
        if (!curr_command)              
        {
            continue;
        }

        // Runs built in commands
        if (!run_builtin(curr_command))
        {
            //printf("Executing external command: %s\n", curr_command->argv[0]);
            execute_other_commands(curr_command);
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
