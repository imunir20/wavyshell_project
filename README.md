# WASH -- WAvy SHell
 
 All code in shell.c except for parsing code is written and coded by Ibrahim Munir
 
 Credit for code beyond that mentioned above belongs to the GMU CS Department
 
 - To run the program, simply type and enter the name of the executable, default executable name will be "shell"
 
 WASH is a custom UNIX shell and supports the following features:
 
 - Job Control:
   - Jobs are either run in the foreground, where the user must wait for the job to finish executing  before starting another job/inputting a new command, foreground jobs can be interrupted by keyboard interactions
   
   - Jobs can also be run in the background, so the user can run multiple jobs/commands at the same time; simply add a space and "&" to the end of the command to make it a background job
   
   - Background jobs are tracked through a linked list data structure, which adds and deletes nodes accordingly (e.g. if a background jobs finishes and terminates, it will also be removed from the linked list)
 
 
 - Non-built-in commands:
   - Examples include "ls", "pwd", "cat [file name]", etc.
 
 
 - Built-in commands:
    - help, quit, fg, bg, jobs, kill
    
    - help: Displays basic help information about shell
    
    - quit: Exits the shell program
    
    - fg [JobID]: Brings a job to foreground and continues it, if not already foreground and not already running

    - bg [JobID]: Continues a job in the background
    
    - jobs: Displays the current background jobs by JobID number
    
    - kill [SIGNAL] [PID]: Sends specified signal to process with specified PID
       - Example: "kill 20 8608" -- Send a SIGTSTP signal to process with PID 8608
       
 - Control operators:
   - Control operators (&&, ||) can be used to connect two commands in one command line
   - Depending on the control operator used and the exit status of the first command, the second command will be either executed or skipped
   - AND (&&) operators will continue if the first command returns an exit status of 0
   - OR (||) operators will continue if the first command does NOT return an exit status of 0
   - Control operator commands may be executed in the background, simply add a space and "&" after the second commands
     - Example: "my_echo 0 && my_echo 99" -- AND List will continue
     - Example: "my_echo 99 || my_echo 0" -- OR List will continue
     - Example: "my_echo 99 && my_echo 0" -- AND List will stop
     - Example: "my_echo 0 || my_echo 99" -- OR List will stop
     - Example: "my_echo 39 || my_echo 23" -- OR List will continue, run in background
     - NOTE: my_echo is test program included in the repository, it returns the first command line argument as its exit status
 
 
 - Keyboard Interactions:
   - CTRL-C: Terminates the current foreground job, if there is one
   - CTRL-Z: Stops the current foreground job, moves it to the background job, if there is one
   

  - File Redirection:
    - Redirect files with a command
    - "<" will direct the following file's input into the command
    - ">" will erase the following file's contents and direct the output of the command into the file
    - ">>" will append the output of the command into the following file
      - Example: "wc < hello.txt"
      - Example: "wc < hello.txt > test.txt"
      - Example: "wc < hello.txt >> test.txt"

      
