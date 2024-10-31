#define _POSIX_SOURCe
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/times.h>


#define LINE_LENGTH 100
#define MAX_ARGS 5
#define MAX_LENGTH 20
#define MAX_BG_PROC 1

extern char **environ;


/**
 * @brief Tokenize a C string 
 * 
 * @param str - The C string to tokenize 
 * @param delim - The C string containing delimiter character(s) 
 * @param argv - A char* array that will contain the tokenized strings
 * @param input - ptr to a string representing the input 
 * @param output - ptr to a string string representing the output 
 * @param background - number representing if a process is in the background (!0) or not (0) 
 * Make sure that you allocate enough space for the array.
 */
int tokenize(char* str, const char* delim, char ** argv, char **input, char **output, int *background){
    char* token;
    token = strtok(str, delim);
    int t = 0;

    for(size_t i = 0; token != NULL; ++i){
        // token is "<", so the next token will be the input
        if (strcmp(token, "<") == 0) {
            // getting next token
            token = strtok(NULL, " ");
            
            if (token) {
                *input = token; // storing input filename
            }
        } 
       
        else if (strcmp(token, ">") == 0) {
            
            token = strtok(NULL, " ");

            if (token) { 
                *output = token;
            }
        }

        else if (strcmp(token, "&") == 0) {
            *background = 1; // background flag
        }

        else {
            argv[i] = token;
            
            
            t++;
        }
        token = strtok(NULL, delim);
    }

    // setting last to NULL to indicate end of string 
    argv[t] = NULL;
    return 0;

}


void change_dir(char *path) {
    /*
    Changing the directory in the shell using POSIX function chdir

    path (char*): ptr to string to recieve and use path
    */

    // going home if empty cd 
    // if (path == NULL) {
    //     path = getenv("HOME"); 
    // }

    // empty cd
    if (path == NULL) {
        printf("TKShell: Expected argument to \"cd\"\n");
        return;
    }
    else {
        if (chdir(path) == -1) {
            printf("TKShell: No such file or directory\n");
        };
    }
}


void print_cwd() {
    /*
    Prints current working directory using POSIX API getcwd 
    */
 
    // to store path
    char current_dir[LINE_LENGTH];

    // getcwd from POSIX API
    if (getcwd(current_dir, LINE_LENGTH) != NULL) { 
        printf("%s\n", current_dir);
    } 
}


void exit_shell(struct tms *time_struct, long ticks_per_sec, int *backgroud) {
    /*
    Exiting shell
    */  

    times(time_struct);
    // converting ticks to seconds
    double user_time = (double)time_struct->tms_cutime / ticks_per_sec;
    double sys_time = (double)time_struct->tms_cstime / ticks_per_sec;
    
    printf("User Time: %.8f\n", user_time); 
    printf("Sys Time: %.8f\n", sys_time);  

    // waiting for background processes
    if (*backgroud != 0) {
        waitpid(*backgroud, NULL, 0);
    }
    
    exit(0);

}


void handle_sigint(int sig) {
    // ignoring this signal in the shell by just returning, child process should still get signal 
    printf("\n");
   
}

void handle_sigtstp(int sig) {
    printf("\n");
   
}


void execute_command(char **args, char *input, char *output, int *background) {
    /*
    Executing a non-piping command that has functionality for input/output redirection and background execution 

    args (char **): array of strings of input 
    input (char *): ptr to string representing the input   
    output (char *): ptr to string representing the output 
    background (int): number representing if a process is in the background (1) or not (0) 
    */

    // duping parent process and creating child process
    pid_t pid = fork(); // syscall, returns 0 to child process, PID of child process to parent process


    if (pid == 0) { 
        // inside the child process (background), so need to reset signal to default
    
        if (input) {

            // checking if input file exists
            if (access(input, F_OK) == -1) { 
                printf("TKShell: %s: No such file or directory\n", input);
                exit(1);  // exiting child process
            }
            // file descriptor, a integer representing an opened file
            int input_file_descriptor = open(input, O_RDONLY);


            // dupes file descriptor of opened file and replaces STDIN_FILENO with it, so any reads from stdin will use input file instead of terminal
            dup2(input_file_descriptor, STDIN_FILENO); // STDIN_FILENO represents file descriptor number for stdin that defaults to standard input stream
            // closes original input_file_descriptor as current input stream uses STDIN_FILENO, but the input file is still being accessed by STDIN_FILENO
            close(input_file_descriptor);
        }

        if (output) {
            // write only, create if not exist, clearing content of file (not appending to file)
            int output_file_descriptor = open(output, O_WRONLY | O_CREAT | O_TRUNC);

            // redirects standard output to an output file like "output.txt" instead of stdout 
            dup2(output_file_descriptor, STDOUT_FILENO); // syscalls
            close(output_file_descriptor);
        }
        // checking if the command exists
        if (access(args[0], X_OK) != 0) {
            printf("TKShell: Command not found\n");
            exit(1);
        }

        
        execve(args[0], args, NULL); 
        
        // replaces current process (child process) to a new process specificed by args[0] (the command)
        int exec_comm_return = execve(args[0], args, NULL); 
        if (exec_comm_return == -1) {
            exit(1);
        }
    }

    // as a parent process, waiting for child if not background
    if (*background) {
        printf("PID %d is sent to background\n", pid);  
        
        *background = 0;  // resetting background flag
    } else {
        waitpid(pid, NULL, 0); 
    }
}


void execute_piped_command(char **command1_args, char **command2_args) {
    /*
    Executing a command that has piping ('|') (WILL NOT HAVE BACKGROUND NOR I/O REDIRECTION EXECUTION)

    command1_args char(**): array of strings of first command of the pipe
    command2_args char(**): array of strings of second command of the pipe
    */

    int pipe_file_descriptor[2]; // length 2, [0] is for reading and [1] is for writing

    pid_t pid1;
    pid_t pid2;

    // creating pipe
    pipe(pipe_file_descriptor);
 
    pid1 = fork();
   
    if (pid1 == 0) {
        
    
        // duping and redirecting stdout of the FIRST command to write end of pipe instead of terminal 
        dup2(pipe_file_descriptor[1], STDOUT_FILENO);
        close(pipe_file_descriptor[0]); 
        close(pipe_file_descriptor[1]); 
        
        // execution of first part of the entire command
        execve(command1_args[0], command1_args, NULL);
    }

    pid2 = fork();

    if (pid2 == 0) {
       
    
        // duping and redirecting stdin of SECOND command to read end of pipe 
        dup2(pipe_file_descriptor[0], STDIN_FILENO);
        close(pipe_file_descriptor[0]); 
        close(pipe_file_descriptor[1]); 
        
      
        execve(command2_args[0], command2_args, NULL);
       
    }

    // parent process -> closing pipe and waiting for children to finish
    close(pipe_file_descriptor[0]);
    close(pipe_file_descriptor[1]);
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
}



int main() {
    char input[LINE_LENGTH];
    char *args[MAX_ARGS];
    char *args2[MAX_ARGS]; // in case of piping to create args for the second command 
    
    struct tms time_struct;
    long ticks_per_s = sysconf(_SC_CLK_TCK);
    times(&time_struct);

    // signal handlers
    struct sigaction sa_int, sa_stp;

    // SIGINT (ctrl+c)
    sa_int.sa_flags = 0;
    sigemptyset(&sa_int.sa_mask);
    sa_int.sa_handler = handle_sigint;
    sigaction(SIGINT, &sa_int, NULL);

    // SIGTSTP (ctrl+z)
    sa_stp.sa_handler = handle_sigtstp;
    sigemptyset(&sa_stp.sa_mask);
    sa_stp.sa_flags = 0;
    sigaction(SIGTSTP, &sa_stp, NULL);

    printf("Welcome to TKShell!\n\n");

    while (1) {
        printf("TKShell > ");
        fflush(stdout);  // flushing output so prompt appears before reading input 


        if (fgets(input, sizeof(input), stdin) == NULL) {
            continue;  

        }
        // find newline character and replace with null terminator, when user finishes input they have to press Enter = "\n" = end of input -> replace with '\0'
        int idx_of_newline = strcspn(input, "\n");
        if (input[idx_of_newline] == '\n') {
            input[idx_of_newline] = '\0'; 
        }

        // finding the index where there is a '|' in the string 
        char *pipe_position = strchr(input, '|');
        char *input_file = NULL;
        char *output_file = NULL;
        int background = 0;

        if (pipe_position != NULL) {
      
            *pipe_position = 0; // spliting input at the pipe symbol, 0 indicates end of string 
            pipe_position++; // moving ptr to next character after '|'
            
            // tokenizing two args based on before and after piping location
            tokenize(input, " ", args, &input_file, &output_file, &background);
            tokenize(pipe_position, " ", args2, &input_file, &output_file, &background);

            execute_piped_command(args, args2);
        } 
        else {
           
            // no piping
            tokenize(input, " ", args, &input_file, &output_file, &background);


            // empty input, user just pressing Enter
            if (args[0] == NULL) {
                continue;  
            }

            else if (strcmp(args[0], "cd") == 0) {
                // cannot run'cd' or 'pwd' commands in the background
                if (!background) {
                    change_dir(args[1]);
                }
            } 

            else if (strcmp(args[0], "pwd") == 0) {
                if (!background) {
                    print_cwd();
                }
            } 

            else if (strcmp(args[0], "exit") == 0) {
                exit_shell(&time_struct, ticks_per_s, &background); 
                break;
            } 

            else {
                execute_command(args, input_file, output_file, &background);
            }
        }
    }

    return 0;
}
