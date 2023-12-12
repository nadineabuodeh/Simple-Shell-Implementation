#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
/*
*This block of code includes the necessary 
*header files for the program to use functions such as 
*printf, fgets, fork, execvp, waitpid, and other system calls.
*/

#define MAX_LINE 80 /* The maximum length command */
/*
*This line defines the maximum length of a command that can be entered by the user.
*/
#define MAX_HISTORY 10 /* The maximum number of commands in history */
/* Define a constant for the maximum number of commands in history */

char history[MAX_HISTORY][MAX_LINE]; /* Array to store command history */
/* Define a two-dimensional array to store command history */
int history_count = 0; /* Number of commands in history */
/* Initialize the number of commands in history to zero */

/* Print the command history */
void print_history() {
/*loop through the command history array 
*backwards to print most recent command first 
*/
    int i;
    for (int i = history_count; i > 0; i--) {
                printf("%d %s", i, history[history_count-i]);
            }

}

/* Retrieve a command from the history */
char* get_from_history(int index) {
    int i = history_count - index;
    if (i < 0 || i >= history_count) {
        return NULL;
    }
    return history[i];
}

int main(void) {
    char *args[MAX_LINE/2 + 1]; /* Command line arguments */
    int should_run = 1; /* Flag to determine when to exit program */
/*
*This defines the main function of the program. 
*It declares an array of pointers called args to 
*hold the command line arguments entered by the user, 
*and a flag variable should_run which is used to determine 
*when the program should exit.
*/
    while (should_run) {
        printf("debbiesh> ");
        fflush(stdout);
/*
*This starts the main loop of the program, 
*where the user is prompted to enter a command by printing 
*"debbiesh> " on the console and flushing the output buffer.
*/
        /* Read user input */
        char input[MAX_LINE];
        fgets(input, MAX_LINE, stdin);
/*
*This reads the user's input from the console 
*and stores it in a character array called input.
*/
        /* Check if user entered "exit" */
        if (strcmp(input, "exit\n") == 0) {
            should_run = 0;
            continue;
        }
/*
*This checks if the user entered the "exit" command. 
*If so, it sets the should_run flag to 0 and continues 
*to the next iteration of the loop.
*/
        /* Check if user entered "history" */
        if (strcmp(input, "history\n") == 0) {
            print_history();/* call print_history function to print command history */
            continue;/* jump to the beginning of the loop to get the next command */
        }

        /* Check if user entered "!!" */
        if (strcmp(input, "!!\n") == 0) {
            if (history_count == 0) {/* check if there is any command in history */
                printf("No commands in history.\n");
                continue;
            }
            strcpy(input, get_from_history(1)); /* get the most recent command from history and copy it to input variable */

            printf("%s", input);/* print the command */
        }

        /* Check if user entered "!N" */
        if (input[0] == '!' && input[1] >= '1' && input[1] <= '9') {
            int index = atoi(input+1);/* convert the number after ! to an integer */
            char* command = get_from_history(index);/* get the command from history */
            if (command == NULL) {/* check if the command exists in history */
                printf("No such command in history.\n");
                continue;
            }
            strcpy(input, command);/* copy the command to input variable */
            printf("%s", input);/* print the command */
        }

        /* Add command to history */
        if (history_count == MAX_HISTORY) {/* check if history array is full */
            /* Shift history array to make room for new command */
            int i;
            for (i = 0; i < MAX_HISTORY-1; i++) {
                strcpy(history[i], history[i+1]); /* shift each command one place to the left */
            }
            history_count--;
/*
*After the loop is finished, history_count is 
*decremented to reflect the fact that we just shifted 
*all the commands to the left by one position.
*/
        }
        strcpy(history[history_count], input);
        history_count++;
/*
*These two lines add the new command to 
*the history array. The strcpy function copies the input 
*command into the next available position in the history array, 
*which is at index history_count.
*And history_count is incremented to reflect 
*the fact that we just added a new command to the history.
*/

        /* Parse user input into command line arguments */
        int argc = 0;
        char *token = strtok(input, " \n");
        while (token != NULL) {
            args[argc] = token;
            argc++;
            token = strtok(NULL, " \n");
        }
        args[argc] = NULL;
/*
*This code tokenizes the user's input and stores 
*each token as a separate element in the args array. 
*The strtok function is used to split the input string 
*into separate tokens using space and newline characters as delimiters.
*/
        /* Check if pipe character is present */
        int has_pipe = 0;
        int pipe_index = -1;
        for (int i = 0; i < argc; i++) {
            if (strcmp(args[i], "|") == 0) {
                has_pipe = 1;
                pipe_index = i;
                break;
            }
        }

        if (has_pipe) {
            /* Create pipe */
            int pipefd[2];
            if (pipe(pipefd) == -1) { //the pipe() function is used for inter-process communication in Linux
                perror("pipe"); //prints a descriptive error message to stderr
                exit(1);
            }

            /* Fork first child process */
            pid_t pid1 = fork();

            if (pid1 < 0) {
                /* Error forking */
                printf("debbiesh: error forking\n");
            } else if (pid1 == 0) {
                /* Child process - first command */
                close(pipefd[0]);  /* Close unused read end of the pipe */

                /* Redirect stdout to the write end of the pipe */
                dup2(pipefd[1], STDOUT_FILENO); 
                //the standard output is redirected to 
		//the write end of the pipe. This means 
		//that any data written to the 
		//standard output will be sent to 
		//the write end of the pipe, 
		//allowing it to be read by the 
		//subsequent command in the pipeline.
                /* Execute the first command */
                args[pipe_index] = NULL; /* Terminate arguments before the pipe character */
                execvp(args[0], args); //execute program
                printf("debbiesh: command not found: %s\n", args[0]);
                exit(1);
            } else {
                /* Fork second child process */
                pid_t pid2 = fork();

                if (pid2 < 0) {
                    /* Error forking */
                    printf("debbiesh: error forking\n");
                } else if (pid2 == 0) {
                    /* Child process - second command */
                    close(pipefd[1]);  /* Close unused write end of the pipe */

                    /* Redirect stdin to the read end of the pipe */
                    dup2(pipefd[0], STDIN_FILENO); //In summary, it is used to redirect 
						//the output of a program to a 
						//specific destination, 
						//such as a file or a pipe.

                    /* Execute the second command */
                    execvp(args[pipe_index + 1], args + pipe_index + 1); //execute program
                    printf("debbiesh: command not found: %s\n", args[pipe_index + 1]);
                    exit(1);
                } else {
                    /* Parent process */
                    close(pipefd[0]);  /* Close unused read end of the pipe */
                    close(pipefd[1]);  /* Close unused write end of the pipe */

                    /* Wait for both child processes to terminate */
                    int status;
                    waitpid(pid1, &status, 0);
                    waitpid(pid2, &status, 0);
                }
            }
        } else {
           /* Fork a child process */
        pid_t pid = fork();

        if (pid < 0) {
            /* Error forking */
            printf("debbiesh: error forking\n");
        } else if (pid == 0) {
            /* Child process */
            execvp(args[0], args);
            printf("debbiesh: command not found: %s\n", args[0]);
            exit(1);
        } else {
            /* Parent process */
            if (strcmp(args[argc-1], "&") != 0) {
                /* Wait for child process to terminate */
                int status;
                waitpid(pid, &status, 0);
            }
/*
*This block of code forks a child process, 
*and executes the command entered by the user in the 
*child process using the execvp function. If the fork 
*function returns a negative value, it prints an error message. 
*If the fork function returns 0, it means that the code is 
*running in the child process, so it executes the command using execvp. 
*If the fork function returns a positive value, it means that the code is 
*running in the parent process, so it waits for the child process
*/
        }
    }
        }
    return 0;
}
