# Operating Systems and Networks - Assignment 3

**Name**: AKSHETT RAI JINDAL

**Roll Number:** 2019114001

## Prerequisites

* You need to have the following programs installed:
    * `gcc`
    * `make`
* Your terminal should support colors

## Assumptions

* The host name has maximum of 256 characters
* The commands entered are no more than 10000 characters long
* The path sizes are a max of `PATH_MAX` characters long (`linux/limits.h`)
* The arguments are tokenized on spaces and tabs
* The different commands are tokenized on the basis of `;`
* The 6 month period for `ls -l` is calculated on the basis of day on which file was modified and does not take the hour or minutes or seconds of the day into account.
* The history file is not deleted during a shell session
* The history is stored in the file `$HOME/.2019114001_ash_history` - **HERE THE $HOME IS THE USER HOME DIR AND NOT THE ONE FROM WHERE THE SHELL WAS EXECUTED**

## Bonus

* From the bonus part, the following is implemented:
  * `replay` command to run a command at fixed intervals for a fixed period of time
  * `baywatch` command to watch some things about the system:
    * `interrupt` - the number of interrupts given to i8042 with IRC id 1
    * `dirty` - the size of the dirty memory used by the system
    * `newborn` - the PID of the newly born processes in the system

## Compilation

* Run `make`
* Then to run the shell, execute `./ash`

## Source Directory Structure
```
src
├── utils -- Utilities used throughout the code
│  ├── terminal_modes.h, terminal_modes.c -- toggling the raw mode of the terminal
│  ├── run_command.h, run_command.c -- forming pipes, handling redirection and start execution of the commands
│  ├── prompt.h, prompt.c -- printing the shell prompt on the terminal
│  ├── process_path.h, process_path.c -- processing any symbols present in paths
│  ├── parser.h, parser.c -- parse the user input and identify the various commands
│  ├── history.h, history.c -- managing the history file
│  ├── create_process.h, create_process.c -- forking for running non-internal commands
│  └── child_process_list.h, child_process_list.c -- managing the jobs started by the shell
├── main.c -- entry point of the shell, take input and pass to parser and command runner
├── headers.h -- the libraries used throughout the source code
├── globals.c -- some global variables to be used throughout the shell
├── consts.h -- some global constant values to be used throughout the shell
└── commands -- Inbuilt commands of the shell
   ├── sig.h, sig.c -- send signals to the background jobs
   ├── replay.h, replay.c -- replay a command at fixed intervals for a period of time
   ├── pwd.h, pwd.c -- print the present working directory
   ├── pinfo.h, pinfo.c -- print the information about the state of a process
   ├── ls.h, ls.c -- list files and directories
   ├── jobs.h, jobs.c -- display the jobs running by the shell
   ├── history.h, history.c -- print content from history file
   ├── fg.h, fg.c -- bring a background job to run in foreground
   ├── echo.h, echo.c -- print something to stdout
   ├── cd.h, cd.c -- change the present working directory
   ├── bg.h, bg.c -- start the background execution of a stopped background process
   └── baywatch.h, baywatch.c -- watch the interrupts, dirty memory and PID of newborn processes of the system
```

## Features and Inbuilt Commands

### Processes and Jobs

* Can create new processes for executing any command other than the inbuilt commands
* Can run the processes in `BACKGROUND` by adding a `&` at the end of the arguments:
    * Not Supported for inbuilt commands
    * The BG processes are stored as jobs in the shell
    * When a job exists, then feedback is given by user about the exit code

### Signals

* `Ctrl+C`:
  * `SIGINT` signal is sent to the current foreground running process
  * Ignored if no command is running in the shell
* `Ctrl+Z`:
  * The current running foreground process is suspended by sending `SIGSTSTP` signal
  * Ignored if no command is running in the shell
* `Ctrl+D`:
  * Exits from the shell if the shell is currently asking for input from user
  * Ignored if the shell is executing some command

### Piping & Redirection with files

* The `|` symbol can be used to create pipes between various commands. Multiple pipes can be created at the same time between various processes (`command1 | command2 | command3 | ...`)
* Redirection is also implemented with files:
  *  `<`: Input Redirection
  *  `>`: Output Redirection with truncation if file already exists
  * `>>`: Output Redirection and appended to the file if it already exists
* Piping and redirection can be used together but Redirection will take precedence over Piping (like `bash`) if the output/input is both redirected and piped

### Job Management Commands

* `jobs`:
  * Print out the current jobs sorted alphabetically by the command run and show whether its stopped or running
  * Flags `-r` and `-s` can be used to print only the running or only the stopped jobs
* `sig`:
  * Send a signal to a job
* `bg`:
  * Resume the execution of a stopped job by sending `SIGCONT`
* `fg`:
  * Bring a background job to foreground and resume execution if stopped

### replay

* This command allows the user to run a command at fixed intervals for a fixed period of time

### baywatch

* This command is equivalent to the watch command but can be used for only 3 specific things:
  * `interrupt` - the number of interrupts given to i8042 with IRC id 1
  * `dirty` - the size of the dirty memory used by the system
  * `newborn` - the PID of the newly born processes in the system
* This keeps on running until the user presses `q` key on the keyboard

### cd

* Changes the current working directory of the shell
* Both absolute and relative paths supported
* Support for following flags:
    * `.` - Current Working Directory
    * `..` - Parent Working Directory
    * `-` - Previous Working Directory
    * `~` - Home Directory of the shell
* No arguments: Home directory

### echo

* Outputs the arguments passed to the terminal
* Removes the extra spaces

### exit

* Exits out of the shell

### history

* Print out commands history
* By default tries to print the latest 10 commands
* It also supports `history <n>` where the latest `n` commands are printed from history
* A command does not get stored in history if it is exactly same as the last entry in the history file
* History is stored in `$HOME/.2019114001_ash_history`
* Supports managing history for multiple shell sessions at the same time

### ls

* Supports `-a` and `-l` flags
* Supports multiple arguments and flags in any order
* Supports multiple-flag arguments like `-la`, `-al`, `-alal`
* The arguments can be both directory and file

### pinfo

* Prints information about a process
* Can take one process id (PID) as argument
* If no arguments are passed, info is shown for current process
* Shows the following stats:
    * `Process ID`
    * `Process Status`
    * `Virtual Memory`
    * `Executable Path`

### pwd

* Prints out the current working directory

### repeat

* Can repeat a command for `n` number of times
* Syntax: `repeat <n> <command> <command-args>`
