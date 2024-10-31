# TKShell

Custom lightweight UNIX Shell 


## System Calls Used


**`chdir()`**: Changes current directory of shell (built-in `cd` command)

**`getcwd()`**: Retrieves current working directory (built-in `pwd` command)

**`fork()`**: Creating a new process. Parent process waits for child to finish while child executes the command

**`execve()`**: Running external commands. Replaces current process image with a new one requested by the command

**`waitpid()`**: Used by parent process to wait for child process to terminate

**`pipe()`**: Creates a pipe used for communication between two processes

**`dup2()`**: Duplicates file descriptors. Used to redirect std I/O for commands with file redirection 

**`open()`**: Opens files for reading or writing. Used for I/O redirection

**`close()`**: Closes file descriptors when they are no longer needed

**`access()`**: Checks if a command is executable and exists


**`sigaction()`**: Sets up signal handlers for certain signals. Inside the shell using SIGINT (ctrl+c) or SIGSTP (ctrl+z) will NOT crash the shell but will affect the child processes

**`times()`**: Measures User and System CPU time used by processes



## Testing and Usage


**Command Execution**:
- Tested commands like `ls`, `find`, `sort`, and custom executables to ensure they all function properly
- Also tested built-in commands such as `cd`, `pwd`, and `exit` so they give the correct outputs and error handling

**Piping**:
- Tested simple pipes (e.g., `/usr/bin/find ./ | /usr/bin/sort`), ensuring that output from one command correctly goes into the input of the next

**I/O Redirection**:
- Tested simple I/O redirection commands like `/usr/bin/echo "test" > ../a.out`

**Signal Handling**:
- Verified that pressing `ctrl+c` and `ctrl+c` does NOT cause the shell to crash, but does terminate child proccesses by running a executeable inside the shell and killing the process without killing the shell

**Background Execution**:
- Tested background execution of process by typing a `&` alongside another command such as `ls &`
and ensured it returned the appropriate message
- Conducted another test with `/usr/bin/sleep 10 &`, which sends the process to the background. The shell immediately returns to the `dragonshell >` prompt, allowing for new inputs without waiting the full 10 seconds

**Testing Errors Cases**:
- Tested commands that were invalid and gave errors such as invalid commands `invalid_command` and commands with empty arguments such as `cd _`
- Redirection with invalid files such as `/usr/bin/cat < input.txt` (input.txt does not exist) was tested to ensure proper error messages are printed
