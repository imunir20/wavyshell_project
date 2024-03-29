# WASH -- WAvy SHell
  
To run the program, simply type and enter the name of the executable, default executable name will be "shell"


 WASH is a custom UNIX shell and supports the following features:
 
 Job Control
 -
   - Jobs are either run in the foreground, where the user must wait for the job to finish executing  before starting another job/inputting a new command, foreground jobs can be interrupted by keyboard interactions
   
   - Jobs can also be run in the background, so the user can run multiple jobs/commands at the same time; simply add a space and "&" to the end of the command to make it a background job
   
   - Background jobs are tracked through a linked list data structure, which adds and deletes nodes accordingly (e.g. if a background jobs finishes and terminates, it will also be removed from the linked list)
 \
 \
 ![](images/jobcontroldemo.png)
 
 Non-built-in commands
 -
   - Examples include "ls", "pwd", "cat [file name]", etc.
 \
 \
 ![](images/nbicmddemo.png)
 
 Built-in commands
 -
   - help, quit, fg, bg, jobs, kill
    
   - help: Displays basic help information about shell
    
   - quit: Exits the shell program
    
   - fg [JobID]: Brings a job to foreground and continues it, if not already foreground and not already running

   - bg [JobID]: Continues a job in the background
    
   - jobs: Displays the current background jobs by JobID number
    
   - kill [SIGNAL] [PID]: Sends specified signal to process with specified PID
      - Example: "kill 20 8608" -- Send a SIGTSTP signal to process with PID 8608
  \
  \
  ![](images/helpquitcmddemo.png)
  ![](images/bicmddemo.png)
  
  
 Control operators
 -
   - Control operators (&&, ||) can be used to connect two commands in one command line
   - Depending on the control operator used and the exit status of the first command, the second command will be either executed or skipped
   - AND (&&) operators will continue if the first command returns an exit status of 0
   - OR (||) operators will continue if the first command does NOT return an exit status of 0
   - Control operator commands may be executed in the background, simply add a space and "&" after the second commands
     - Example: "my_echo 0 && my_echo 99" -- AND List will continue
     - Example: "my_echo 99 || my_echo 0" -- OR List will continue
     - Example: "my_echo 99 && my_echo 0" -- AND List will stop
     - Example: "my_echo 0 || my_echo 99" -- OR List will stop
     - Example: "my_echo 39 || my_echo 23 &" -- OR List will continue, run in background
     - NOTE: my_echo is a test program included in the repository, it returns the first command line argument as its exit status
 \
 \
 ![](images/contop1demo.png)
 ![](images/contop2demo.png)
 ![](images/contop3demo.png)
 
 
 Keyboard Interactions
 -
   - CTRL-C: Terminates the current foreground job, if there is one
   - CTRL-Z: Stops the current foreground job if there is one, and moves it to the background
   \
   \
   ![](images/ctrlcdemo.png)
   ![](images/ctrlzdemo.png)
   

  File Redirection
  -
   - Redirect files with a command
   - "<" will direct the following file's input into the command (if the input file doesn't exist, the shell will print an error message and prompt for another command)
   - ">" will erase the following file's contents and direct the output of the command into the file (if the output file doesn't exist, the shell will create it)
   - ">>" will append the output of the command into the following file (if the output file doesn't exist, the shell will create it)
     - Example: "wc < hello.txt"
     - Example: "wc < hello.txt > test.txt"
     - Example: "wc < hello.txt >> test.txt"
     \
     \
     ![](images/fileredirectiondemo.png)
      

      
