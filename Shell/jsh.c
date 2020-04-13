/*
 * Conner Brinkley
 * December 2, 2019
 * Systems Programming
 * 
 * Shell Lab
 * Part II -- A primitive shell with I/O redirection.
 * 
 * */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include "fields.h"
#include "jval.h"


/* RUN THE SHELL */

int runShell(IS userInput, char *prompt) {

    // Variables
    int failed, status, waiting, size, redirect, i;
    int pid, fileDesc1, fileDesc2;
    char **arguments;
    
    // The prompt
    if (prompt != NULL) printf("%s ", prompt);

    // Read in
    while (get_line(userInput) > 0) {
        redirect = 0;

        // Check to see if we need to exit
        if (strcmp(userInput->fields[0], "exit") == 0) {
            return 1;
        }

        // Check to see if we need to wait for the next command
        if (strcmp(userInput->fields[userInput->NF-1], "&") == 0) {
            waiting = 0;
            size = userInput->NF - 1;
        } else {
            waiting = 1;
            size = userInput->NF;
        }

        // Parse the arguments before executing
        arguments = (char **) malloc((sizeof(char *) * userInput->NF) + 1);
        for (i = 0; i < size; i++) {

            // Check for file redirection
            if (strcmp(userInput->fields[i], "<") == 0 ||
                strcmp(userInput->fields[i], ">") == 0 ||
                strcmp(userInput->fields[i], ">>") == 0) {
                    redirect = 1;
                    break;
            }
            
            // Add the argument
            arguments[i] = userInput->fields[i];
        }
        arguments[i] = NULL;

        // Fork and execute
        pid = fork();
        if (pid == 0) {

            // Redirect I/O if necessary
            if (redirect) {

                // Check for input redirection
                if (i < size && strcmp(userInput->fields[i], "<") == 0) {
                    if (i + 1 != size) {
                        fileDesc1 = open(userInput->fields[i+1], O_RDONLY);
                        
                        // Error checking
                        if (fileDesc1 < 0) {
                            perror(userInput->fields[i+1]);
                            exit(1);
                        }

                        // Redirect input
                        if (dup2(fileDesc1, 0) != 0) {
                            perror(userInput->fields[i+1]);
                            exit(1);
                        }
                        close(fileDesc1);
                    }
                    i += 2;
                }

                // Check for output redirection
                if (i < size && strcmp(userInput->fields[i], ">") == 0 ||
                                strcmp(userInput->fields[i], ">>") == 0) {
                    if (i + 1 != size) {
                        
                        // Determine if we are appending or truncating
                        if (strcmp(userInput->fields[i], ">>") == 0) {
                            fileDesc2 = open(userInput->fields[i+1], O_WRONLY | O_APPEND | O_CREAT, 0644);
                        }
                        else fileDesc2 = open(userInput->fields[i+1], O_WRONLY | O_TRUNC | O_CREAT, 0644);
                        
                        // Error checking
                        if (fileDesc2 < 0) {
                            perror(userInput->fields[i+1]);
                            exit(2);
                        }

                        // Redirect output
                        if (dup2(fileDesc2, 1) != 1) {
                            perror(userInput->fields[i+1]);
                            exit(2);
                        }
                        close(fileDesc2);
                    }
                    i += 2;
                }
            }

            // Execute
            failed = execvp(userInput->fields[0], arguments);
            perror(userInput->fields[0]);
            exit(1);
        } else {

            // Check if we wait for the process or continue
            if (waiting) {
                while (i != pid) {
                    i = wait(&status);
                }
            } else {
                free(arguments);
                continue;
            }
        }

        // Return the prompt
        if (prompt != NULL) printf("%s ", prompt);
        free(arguments);
    }

    return 0;
}


/* DRIVER CODE */

int main(int argc, char **argv, char **envp) {
    
    // Variables
    IS userInput;
    char *prompt;
    int exited;

    // Init the input struct and prompt
    userInput = new_inputstruct(NULL);
    if (argc == 2) {
        if (strcmp(argv[1], "-") == 0) {
            prompt = NULL;
        } else prompt = argv[1];
    } else {
        prompt = "jsh1:";
    }

    // Run until we hit EOF
    exited = runShell(userInput, prompt);
    if (!exited && prompt != NULL) printf("\n");
    
    // Housekeeping
    jettison_inputstruct(userInput);
    return 0;
}
