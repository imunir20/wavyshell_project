# WASH -- WAvySHell
 
 All code in shell.c except for parsing code is written and coded by Ibrahim Munir
 
 Credit for code beyond that mentioned above belongs to the GMU CS Department
 
 WASH is a custom shell and supports the following features:
 
 #- Job Control:
   - Jobs are either run in the foreground, where the user must wait for the job to finish executing  before starting another job/inputting a new command, foreground jobs can be interrupted by keyboard interactions
   
   - Jobs can also be run in the background, so the user can run multiple jobs/commands at the same time; simply add a space and "&" to the end of the command to make it a background job
 
 
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
       
 - 
