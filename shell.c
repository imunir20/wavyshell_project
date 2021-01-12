

// All code except parsing code in main() and its associated code (credit to GMU CS Dept.)
// has been designed and coded by
// Ibrahim Munir

#include "shell.h"
#include "parse.h"

/* Constants */
#define DEBUG 0            // Used to turn parsing DEBUG information on (1) and off (0)



/* STRUCT DEFINITIONS */

/* Job Struct Definition */
typedef struct job_struct {
	int jobID;               // Assigned Job ID
	int pid;                 // Process ID of job
	int is_running;   // Execution status of job; 1 if it is running, 0 if it isn't (stopped)
	char *command;     // Initial command line that started the job
	struct job_struct *next;   // Link to next job struct node
} Job;



/* Job Linked List Definition */
typedef struct list_struct {
	Job *head;     // The head of the linked list, which holds the data of the FG job
	Job *tail;     // The tail of the linked list
	int count;     // Current amount of jobs
} List;	




/* GLOBALS */

 static const char *shell_path[] = { "./", "/usr/bin/", NULL };
 static const char *built_ins[] = { "quit", "help", "kill", 
 "fg", "bg", "jobs", NULL};

 static List *jobs;         // Global List of Jobs
 static int signaled = 0;   // Global variable that indicates that a signal has been sent


/* FUNCTION PROTOTYPES */
void chldHandler(int sig);
void intHandler(int sig);
void tstpHandler(int sig);
int isFg(int pid);
void initList();
Job *generateJob(int jobID, int pid, int is_running, char *command);
void addFgJob(Job *fg);
void removeFgJob();
void addBgJob(Job *bg);
Job *removeBgJobId(int jobID);
Job *removeBgJobPid(int pid);
void freeList();
void freeJob(Job *job);
int performBuiltIn(char *argv[]);
void execQuit();
void execHelp();
void execKill(char *argv[]);
void execFg(char *argv[]);
void execBg(char *argv[]);
void execJobs();
void performNonBuiltIn(char *argv[], char *argv2[], Cmd_aux aux);
void getCommandName(char fullCommand[], char *argv[], char *argv2[], Cmd_aux aux);
void executeProgram(char fullCommand[], char *args[], sigset_t prev_mask);
void logJobStart(int pid, char fullCommand[], Cmd_aux aux);
void logJobEnd(int pid, char fullCommand[], Cmd_aux aux);
void executeControlOperator(char *argv[], char *argv2[], Cmd_aux aux);
void performRedirection(char *argv[], char *argv2[], Cmd_aux aux);


/* The entry of your shell program */
int main() {
  char cmdline[MAXLINE];        /* Command line */
  char *cmd = NULL;

  /* Intial Prompt and Welcome */
  log_prompt();
  log_help();	


	initList();


	struct sigaction chld;                  // Installing signal handler for SIGCHLD
	memset(&chld, 0, sizeof(sigaction));
	chld.sa_handler = chldHandler;
	sigaction(SIGCHLD, &chld, NULL);

	struct sigaction inter;                // Installing signal handler for SIGINT
	memset(&inter, 0, sizeof(sigaction));
	inter.sa_handler = intHandler;
	sigaction(SIGINT, &inter, NULL);	
	
	struct sigaction tstp;                 // Installing signal handler
	memset(&tstp, 0, sizeof(sigaction));
	tstp.sa_handler = tstpHandler;
	sigaction(SIGTSTP, &tstp, NULL);	




  /* Shell looping here to accept user command and execute */
  while (1) {
    char *argv[MAXARGS], *argv2[MAXARGS];     /* Argument list */
    Cmd_aux aux;                /* Auxilliary cmd info: check parse.h */
	

    /* Print prompt */
    log_prompt();

    /* Read a line */
    // note: fgets will keep the ending '\n'
    if (fgets(cmdline, MAXLINE, stdin) == NULL) {
      if (errno == EINTR)
        continue;
      exit(-1);
    }

	

    if (feof(stdin)) {  /* ctrl-d will exit shell */
      exit(0);
    }

    /* Parse command line */
    if (strlen(cmdline)==1)   /* empty cmd line will be ignored */
      continue;     

    cmdline[strlen(cmdline) - 1] = '\0';        /* remove trailing '\n' */

    cmd = malloc(strlen(cmdline) + 1);
    snprintf(cmd, strlen(cmdline) + 1, "%s", cmdline);

    /* Bail if command is only whitespace */
    if(!is_whitespace(cmd)) {
      initialize_argv(argv);    /* initialize arg lists and aux */
      initialize_argv(argv2);
      initialize_aux(&aux);
      parse(cmd, argv, argv2, &aux); /* call provided parse() */

      if (DEBUG)  /* display parse result, redefine DEBUG to turn it off */
        debug_print_parse(cmd, argv, argv2, &aux, "main (after parse)");

      /* After parsing: your code to continue from here */
      /*================================================*/
	

    }


	                                        // NOTE: Signal handlers installation not a function b/c it require function pointers,
											// which will cause unused variable warnings in main, thus not implemented to limit
											// compiler warnings
	
	if(aux.in_file != NULL || aux.out_file != NULL)        // If there is an in or out file, perform file redirection
	{
		performRedirection(argv, argv2, aux);
	}
	else if(aux.control != 0)                              // If there is a control operator, perform job w/ control operator
	{
		executeControlOperator(argv, argv2, aux);
	}
	else if(performBuiltIn(argv) != 0)                    // Priority for built-in commands over non-built-in commands
	{
		performNonBuiltIn(argv, argv2, aux);
	}




    free_options(&cmd, argv, argv2, &aux);
  }
  freeList();     // Frees allocated memory of global list of jobs

  return 0;
}



/* Signal handler for SIGCHLD
 * Reaps processes, checks if they were terminated normally,
 * or possibly terminated by a signal
 * Also checks if processes are stopped, or were continued
 */
void chldHandler(int sig)
{
	int pid;
	int status;
	//char *jobCmd;
	Job *sigJob;
	while((pid = waitpid(-1, &status, WNOHANG || WUNTRACED || WCONTINUED)) > 0)       // Check for "zombies"
	{

		// SIGJOB = REMOVEBGJOBPID(PID); IF IT WORKS, THEN REMOVE FROM IF STATEMENTS


		if(WIFEXITED(status))                  // Use status to check for normal termination
		{
		//	printf("WIFEXITED\n");
			if(isFg(pid) != 1)
			{	

				sigJob = removeBgJobPid(pid);
				if(sigJob != NULL)
				{	
					sigJob->is_running = 0;
					log_job_bg_term(pid, sigJob->command);
				}	
			}
			/*
			else
			{
				jobCmd = (jobs->head)->command;
				removeFgJob();
				log_job_fg_term(pid, jobCmd);
			}
			*/
		}
		else if(WIFSIGNALED(status))                    // Check if process was terminated by a signal
		{
			//printf("WIFSIGNALED\n");
			if(isFg(pid) != 1)
			{	
				sigJob = removeBgJobPid(pid);
				if(sigJob != NULL)
				{	
					sigJob->is_running = 0;
					log_job_bg_term_sig(pid, sigJob->command);
				}
			}
			/*
			else
			{
				jobCmd = (jobs->head)->command;
				removeFgJob();
				log_job_fg_term_sig(pid, jobCmd);
			}
			*/
		}
		else if(WIFSTOPPED(status))                     // Check if process was stopped
		{
			//printf("WIFSTOPPED\n");	
			/*
			if(isFg(pid) == 1)
			{

			}	
			*/		
			if(isFg(pid) != 1)
			{	
				// sigJob = removeBgJobPid(pid);
				// sigJob->is_running = 0;		
				log_job_bg_stopped(pid, sigJob->command);
			}
			/*
			else
			{
				jobCmd = (jobs->head)->command;
				removeFgJob();
				log_job_fg_stopped(pid, jobCmd);
			}	
			*/
		}
		else if(WIFCONTINUED(status))                  // Check if process was continued
		{
			//printf("WIFCONTINUED\n");
			if(isFg(pid) != 1)
			{	
				// sigJob = removeBgJobPid(pid);
				// sigJob->is_running = 1;
				log_job_bg_cont(pid, sigJob->command);
			}

			/*
			else
			{
				jobCmd = (jobs->head)->command;
				removeFgJob();
				log_job_fg_cont(pid, jobCmd);
			}
			*/
		}	
	}
}





/* Signal handler for SIGINT
 * Logs acknowledgment of CTRL + C keys
 * Forwards signal to foreground process
 */
void intHandler(int sig)
{
	int status;
	log_ctrl_c();
	if((jobs->head)->pid != -1 && (jobs->head) != NULL)
	{
		//printf("SIGINT Handler!\n");
		int termPid = (jobs->head)->pid;
		char *termCmd = (jobs->head)->command;
		kill((jobs->head)->pid, SIGINT);
		waitpid(termPid, &status, 0);
		removeFgJob();
		log_job_fg_term_sig(termPid, termCmd);
		signaled = 1;
	}
	//printf("SIGINT Handler outside!\n");
}





/* Signal handler for SIGTSTP
 * Logs acknowledgment of CTRL + Z keys
 * Forwards signal to foreground process
 */
void tstpHandler(int sig)
{
	int status;
	log_ctrl_z();
	if((jobs->head)->pid != -1 && (jobs->head) != NULL)           // FIX: changed == NULL to != NULL to mirror above function
	{
		//printf("SIGTSTP Handler!\n");
		int stoppedPid = (jobs->head)->pid;
		char *stoppedCmd = (jobs->head)->command;
		kill((jobs->head)->pid, SIGTSTP);
		waitpid(stoppedPid, &status, WNOHANG || WUNTRACED);
		
		//printf("Done with waitpid\n");
		(jobs->head)->is_running = 0;

		Job *newBgJob = generateJob(50, jobs->head->pid, 0, jobs->head->command);		


		addBgJob(newBgJob);
		removeFgJob();
		newBgJob->is_running = 0;
		log_job_fg_stopped(stoppedPid, stoppedCmd);
		printf("This one being logged.\n");
		signaled = 1;
	}
	//printf("SIGTSTP Handler outside!\n");
}


/* Checks if a process with given PID
 * is foreground or not
 */
int isFg(int pid)
{
	if((jobs->head)->pid == pid)
	{
		return 1;           // Return 1, true, if pid same as head's pid which is where FG job is located
	}
	return 0;               // Else, return 0, false, it is a hackground process, or the job/pid doesn't exist
}




/* Allocates space for List, 
 * the head and tail of the list,
 * and initializes the current job count
 */
void initList()
{
	jobs = malloc(sizeof(List));
	if(jobs == NULL)
	{
		exit(1);
	}
	Job *dummyHead = generateJob(0, -1, -1, "");
	Job *dummyTail = generateJob(0, -1, -1, "");
	jobs->head = dummyHead;
	jobs->tail = dummyTail;
	(jobs->head)->next = jobs->tail;
	jobs->count = 0;
}


/* Generates a job with the given parameters
 */
Job *generateJob(int jobID, int pid, int is_running, char *command)
{
	Job *newJob = malloc(sizeof(Job));
	if(newJob == NULL)
	{
		exit(1);
	}	
	newJob->jobID = jobID;
	newJob->pid = pid;
	newJob->is_running = is_running;
	newJob->command = malloc((sizeof(char) * strlen(command)) + 1);
	if(newJob->command == NULL)
	{
		exit(1);
	}	
	strcpy(newJob->command, command);
	newJob->next = NULL;
	return newJob;
}	


/* Adds a job to the foreground
 * in the List
 */
void addFgJob(Job *fg)
{
	fg->next = (jobs->head)->next;
	jobs->head = fg;
	if((fg->jobID) <= 0)
	{
		fg->jobID = 0;
	}
	fg->is_running = 1;
}



/* Removes the foreground job
 * from the List
 */
void removeFgJob()
{
	(jobs->head)->jobID = 0;         // Ensure jobID for foreground job is still 0
	(jobs->head)->pid = -1;          // Make PID negative to indicate a job that is invalid/no longer existing
	(jobs->head)->is_running = 0;
}




/* Adds a job to the background
 * in the List
 */
void addBgJob(Job *bg)
{
	if((jobs->tail)->pid != -1)             // If there is a tail, adjust links, make the new BG job the tail
	{
		(jobs->tail)->next = bg;
		bg->jobID = ((jobs->tail)->jobID) + 1;
		bg->is_running = 1;
		jobs->tail = bg;
	}
	else                             // Else, no current BG jobs, make the job the 1st and the tail of the list
	{
		jobs->tail = bg;
		(jobs->head)->next = jobs->tail;
		(jobs->tail)->jobID = 1;
		(jobs->tail)->is_running = 1;		
	}	
	jobs->count = (jobs->count) + 1;
}




/* Removes a background job
 * from the List using given Job ID
 * Returns removed job on success
 * Returns NULL on error
 */
Job *removeBgJobId(int jobID)
{
	Job *target;                   /* Job with matching given jobID to be removed from the list and returned */
	Job *current;                  /* Job used to traverse list and fix links after removal */
	if(((jobs->head)->next) == NULL)          /* Dereferences the first bg job of the list to see if it is NULL */
	{
		return NULL;
	}
	if((((jobs->head)->next)->jobID) == jobID)       // If first bg job of list has matching jobID: remove it, fix links, adjust count, return job 
	{
		target = (jobs->head)->next;
		if(target->next == NULL)
		{	
			jobs->tail = generateJob(0, -1, -1, "");
			(jobs->head)->next = jobs->tail;
		}
		else
		{	
			(jobs->head)->next = target->next;
		}	
		target->next = NULL;
		jobs->count = (jobs->count) - 1;
		return target;	
	}
	current = (jobs->head)->next;               /* Current job is first bg job since traversal begins at first bg job */
	while((current->next) != NULL)        /* While there are more bg jobs in the list, do the following... */
	{
		if(((current->next)->jobID) == jobID)    /* If the next bg job has the matching job ID: remove it, fix links, adjust count, return job */
		{
			target = current->next;
			if(target->next == NULL)
			{
				jobs->tail = current;
			}
			current->next = target->next;
			target->next = NULL;
			jobs->count = (jobs->count) - 1;
			return target;
		}
		else                       /* Else, just move on and check the next job in the list (if there is one) */
		{
			current = current->next;
		}
	}
	return NULL;                   /* Return NULL if whole linked list has been traversed and job has not been found to indicate an error */
}





/* Removes a background job
 * from the List using given PID
 * Returns removed job on success
 * Returns NULL on error
 */
Job *removeBgJobPid(int pid)
{
	Job *target;                   /* Job with matching given pid to be removed from the list and returned */
	Job *current;                  /* Job used to traverse list and fix links after removal */
	if(((jobs->head)->next) == NULL)          /* Dereferences the first bg job of the list to see if it is NULL */
	{
		return NULL;
	}
	if((((jobs->head)->next)->pid) == pid)       /* If the first bg job of the list has matching pid: remove it, fix links, adjust count, return job */
	{
		target = (jobs->head)->next;
		if(target->next == NULL)
		{	
			jobs->tail = generateJob(0, -1, -1, "");
			(jobs->head)->next = jobs->tail;
		}
		else
		{	
			(jobs->head)->next = target->next;
		}	
		target->next = NULL;
		jobs->count = (jobs->count) - 1;
		return target;	
	}
	current = (jobs->head)->next;               /* Current job is first bg job since traversal begins at first bg job */
	while((current->next) != NULL)        /* While there are more bg jobs in the list, do the following... */
	{
		if(((current->next)->pid) == pid)    /* If the next bg job has the matching job pid: remove it, fix links, adjust count, return job */
		{
			target = current->next;
			if(target->next == NULL)
			{
				jobs->tail = current;
			}
			current->next = target->next;
			target->next = NULL;
			jobs->count = (jobs->count) - 1;
			return target;
		}
		else                       /* Else, just move on and check the next job in the list (if there is one) */
		{
			current = current->next;
		}
	}
	return NULL;                   /* Return NULL if whole linked list has been traversed and job has not been found to indicate an error */
}

	
/* Frees up memory allocated in and
 * within the List Struct
 */
void freeList()
{
	Job *current = jobs->head;
	Job *prev = jobs->head;
	jobs->count = 0;
	while(prev != NULL)
	{
		current = prev->next;
		prev->next = NULL;
		freeJob(prev);
		prev = current;
	}
	free(jobs);
	jobs = NULL;
}



/* Frees up memory allocated in and
 * within a Job Struct
 */
void freeJob(Job *job)
{
	free(job->command);
	job->command = NULL;
	free(job);
	job = NULL;
}




/* Determines and runs if a built-in command was called
 * Returns -1 if command was not built-in
 * Return 0 if command WAS built-in and performed
 */
int performBuiltIn(char *argv[])
{
	// Go through all built-in commands and see which one was called by user
	if(strcmp(argv[0], built_ins[0]) == 0)
	{
		execQuit();
	}
	else if(strcmp(argv[0], built_ins[1]) == 0)
	{
		execHelp();
	}
	
	else if(strcmp(argv[0], built_ins[2]) == 0)
	{
		execKill(argv);
	}
	else if(strcmp(argv[0], built_ins[3]) == 0)
	{
		execFg(argv);
	}
	else if(strcmp(argv[0], built_ins[4]) == 0)
	{
		execBg(argv);
	}
	else if(strcmp(argv[0], built_ins[5]) == 0)
	{
		execJobs();
	}
	
	else   // If still in loop, then in must not be a built-in command
	{
		return -1;
	}
	return 0;     // If out of loop, then must've been built-in command and executed
}	


/* Prints/logs the help information */
void execHelp()
{
	log_help();
}


/* Prints/logs the quitting prompt, and exits from the shell program */
void execQuit()
{
	log_quit();
	exit(0);
}



/* Sends signal to intended process with given PID
 * Uses argv to determine the signal to be sent and the process's PID
 * Logs appropriate messages associated with signal
 */
void execKill(char *argv[])
{
	int status;
	int signal = atoi(argv[1]);    // Translate
	int pid = atoi(argv[2]);
	char *command;
	Job *current = jobs->head;
	while(current!= NULL && current->pid != pid)  // Find process/job within List
	{	
		current = current->next;
	}
	if(current != NULL)
	{	
		command = current->command;
	}	
	kill(pid, signal);        // Send signal
	log_kill(signal, pid);
	if(signal == 9 || signal == 2)            // SIGKILL, SIGINT scenario
	{
		if(isFg(pid) != 1)      // Doesn't equals head's PID, must be background job
		{
			//log_job_bg_term_sig(pid, command);	  // SIGCHLD Handler takes care of logging messages
		}
		else
		{
			waitpid(pid, &status, 0);
			//log_job_fg_term_sig(pid, command);
		}
		current->is_running = 0;
	}
	if(signal == 18)                    // SIGCONT scenario
	{
		if(isFg(pid) != 1)
		{
			log_job_bg_cont(pid, command);
		}
		else
		{
			waitpid(pid, &status, 0);
			log_job_fg_cont(pid, command);
		}
		current->is_running = 1;
	}
	if(signal == 20)                   // SIGTSTP scenario
	{
		if(isFg(pid) != 1)
		{
			log_job_bg_stopped(pid, command);
		}
		else
		{
			waitpid(pid, &status, 0);
			log_job_fg_stopped(pid, command);
		}
		current->is_running = 0;
	}
}




/* Moves specified process with given ID
 * to the foreground if it exists
 * Continues process if it was stopped in
 * the background
 * Logs appropriate messages
 */
void execFg(char *argv[])
{
	int pid;
	int status;
	int id = atoi(argv[1]);
	char command[100];
	strcpy(command, "fg ");
	strcat(command, argv[1]);
	Job *newFg = removeBgJobId(id);
	if(newFg == NULL)
	{
		log_jobid_error(id);
	}
	else if(newFg->is_running == 1)     // If running already, just put in foreground
	{
		log_job_fg(newFg->pid, command);
		pid = newFg->pid;
		addFgJob(newFg);
		waitpid(newFg->pid, &status, 0);
		removeFgJob();
		log_job_fg_term(pid, command);
	}
	else                                 // Else if stopped, move to foreground, continue job
	{
		addFgJob(newFg);
		newFg->is_running = 1;
		log_job_fg(newFg->pid, command);
		pid = newFg->pid;
		log_job_fg_cont(newFg->pid, command);
		kill(newFg->pid, 18);       // Send SIGCONT to continue the stopped job
		//log_job_fg_cont(newFg->pid, command);
		waitpid(newFg->pid, &status, WCONTINUED);
		removeFgJob();
		//log_job_fg_term(pid, command);
	}
}


/* Continues a currently stopped
 * background job to a continued state
 * using given ID
 * Logs appropriate messages
 */
void execBg(char *argv[])
{
	int id = atoi(argv[1]);
	char command[100];
	strcpy(command, "bg ");
	strcat(command, argv[1]);
	Job *current = (jobs->head)->next;
	while((current != NULL) && (current->jobID != id))
	{
		current = current->next;
	}
	if(current == NULL)
	{
		log_jobid_error(id);
	}
	else
	{
		log_job_bg(current->pid, command);
		if(current->is_running != 1)         // If the job is not running (stopped), set it to running
		{
			current->is_running = 1;
			log_job_bg_cont(current->pid, command);
			kill(current->pid, 18);         // Send SIGCONT signal to continue execution of job 
			
		}
	}
}



/* Logs the current number of
 * background jobs currently running/stopped
 * Logs the details of the jobs
 */
void execJobs()
{
	char state[100];
	log_job_number(jobs->count);
	Job *current = (jobs->head)->next;
	while(current != NULL && current->pid != -1)   // Cycle through all BG jobs in global List
	{
		if(current->is_running == 1)         // Check if running or stopped
		{
			strcpy(state, "Running");
		}
		else
		{
			strcpy(state, "Stopped");
		}
		log_job_details(current->jobID, current->pid, state, current->command);
		current = current->next;
	}
}




/* Performs the entered command as a non-built-in command
 * First attempts to check command in path "./"
 * If unsuccessful, will attempt the path "/usr/bin/"
 * If unsuccessful again, will print/log error message, exits child process
 */

void performNonBuiltIn(char *argv[], char *argv2[], Cmd_aux aux)
{
	int pid;
	int status;
	sigset_t mask, prev_mask;
	char fullCommand[150];        // String that has command and args all in one string
	getCommandName(fullCommand, argv, argv2, aux);
	sigaddset(&mask, SIGCHLD);
	sigprocmask(SIG_BLOCK, &mask, &prev_mask);       // Block SIGCHLD, prevent race conditions
	if((pid = fork()) == 0)          // Assign pid to pid variable, check if program is in child process currently
	{
		setpgid(0, 0);
		executeProgram(fullCommand, argv, prev_mask);
	}	   
	else                    // Else, if program is currently in parent process
	{
		logJobStart(pid, fullCommand, aux);
		Job *newJob = generateJob(0, pid, 1, fullCommand);
		if(aux.is_bg != 1)           // If FG job, wait for it
		{	
			addFgJob(newJob);
			waitpid(pid, &status, WUNTRACED);

			
			if(signaled == 1)
			{
				signaled = 0;
			}
			else
			{	
				removeFgJob();
				logJobEnd(pid, fullCommand, aux);
			}	
		}
		else                         // Else, don't wait for BG job, just add it to the global List of jobs
		{
			addBgJob(newJob);
		}
		sigprocmask(SIG_SETMASK, &prev_mask, NULL);        // Restore mask
	}
}




/* Uses argv and argv2 to fully form
 * the original command as a string
 */
void getCommandName(char fullCommand[], char *argv[], char *argv2[], Cmd_aux aux)         
{
	int i = 1;
	strcpy(fullCommand, argv[0]);   // Concatenate command
	while(argv[i] != NULL)          // While there are args still left, concatenate it to the full command name
	{
		strcat(fullCommand, " ");
		strcat(fullCommand, argv[i]);
		i++;
	}
	if(aux.in_file != NULL)                  // File Redirection symbols
	{
		strcat(fullCommand, " < ");
		strcat(fullCommand, aux.in_file);
	}
	if(aux.out_file != NULL)
	{
		if(aux.is_append == 0)
		{
			strcat(fullCommand, " > ");
			strcat(fullCommand, aux.out_file);
		}	
		else
		{
			strcat(fullCommand, " >> ");
			strcat(fullCommand, aux.out_file);
		}	
	}	
	if(argv2[0] != NULL)                // Control operators part of string
	{
		if(aux.control == 1)
		{	
			strcat(fullCommand, " && ");
		}
		else
		{
			strcat(fullCommand, " || ");
		}
		strcat(fullCommand, argv2[0]);
		i = 1;
		while(argv2[i] != NULL)          // While there are args still left, concatenate it to the full command name
		{
			strcat(fullCommand, " ");
			strcat(fullCommand, argv[i]);
			i++;
		}
	}
	if(aux.is_bg == 1)            // Background job symbol
	{
		strcat(fullCommand, " &");

	}
}	



/* Executes a non-built-in command using the two paths:
 * "./" and "/usr/bin/"
 * If command/program not found, logs appropriate message
 * and child process will exit
 */
void executeProgram(char fullCommand[], char *args[], sigset_t prev_mask)
{
	char path[150];
	strcpy(path, shell_path[0]);
	strcat(path, args[0]);
	sigprocmask(SIG_SETMASK, &prev_mask, NULL);       // Block SIGCHLD to prevent race conditions
	if(execv(path, args) == -1)    // Check if command is located in "./" path
	{
		strcpy(path,shell_path[1]);
		strcat(path, args[0]);
		if(execv(path, args) == -1) // If not found, check if the command is in "/usr/bin/" path
		{
			log_command_error(fullCommand);     // If still not found, report the error
			exit(1);
		}
	}
	exit(0);
}


/* Logs the start of a job
 */
void logJobStart(int pid, char fullCommand[], Cmd_aux aux)
{
	if(aux.is_bg != 1)              // Check if the current job is foreground
	{
		log_start_fg(pid, fullCommand);   
	}
	else
	{
		log_start_bg(pid, fullCommand);     
	}
}


/* Logs the end of a job
 */
void logJobEnd(int pid, char fullCommand[], Cmd_aux aux)
{
	if(aux.is_bg != 1)              // Check if the current job is foreground
	{	
		log_job_fg_term(pid, fullCommand);
	}
	else
	{
		log_job_bg_term(pid, fullCommand);
	}
}



/* Executes a control operator command as one job,
 * splitting the job into command in order to perform
 * the command successfully
 */
void executeControlOperator(char *argv[], char *argv2[], Cmd_aux aux)
{
	int pid1;
	int pid2;
	int status1;
	int status2;
	sigset_t mask, prev_mask;
	sigset_t mask2, prev_mask2;
	//sigset_t mask3, prev_mask_3;
	int exitStatus;
	char fullCommand[150];
	Job *newJob;
	getCommandName(fullCommand, argv, argv2, aux);
	
	if(fork() == 0)
	{
		setpgid(0, 0);

		sigaddset(&mask, SIGCHLD);
		sigprocmask(SIG_BLOCK, &mask, &prev_mask);      // Block SIGCHLD to prevent race
		if((pid1 = fork()) == 0)
   		{
			setpgid(0, 0);
			executeProgram(fullCommand, argv, prev_mask);      // Execute first command
		}
		else
		{		
			logJobStart(pid1, fullCommand, aux);
			// Job *newJob = generateJob(0, pid1, 1, fullCommand);
			newJob = generateJob(0, pid1, 1, fullCommand);

			if(aux.is_bg != 1)
			{
				addFgJob(newJob);			
				waitpid(pid1, &status1, 0);
				removeFgJob();
				// free(newJob);
			}
			else
			{
				addBgJob(newJob);
				waitpid(pid1, &status1, 0);   // NEW WAITPID LINE
			}
			sigprocmask(SIG_SETMASK, &prev_mask, NULL);
		
		
			if(WIFEXITED(status1))                         // Extract the exit code
			{
				exitStatus = WEXITSTATUS(status1);
			}
		}
		if(((aux.control == 1) && (exitStatus == 0)) || ((aux.control == 2) && (exitStatus != 0)))  // Depending on exit code, cont or stop AND/OR list
		{
			sigaddset(&mask2, SIGCHLD);
			sigprocmask(SIG_BLOCK, &mask2, &prev_mask2);             // Block SIGCHLD again to prevent race for second command
			if((pid2 = fork()) == 0)                                 // Fork for second command
			{
				setpgid(0, 0);
				executeProgram(fullCommand, argv2, prev_mask2);         // Execute second command
			}
			else
			{
				// Job *newJob = generateJob(0, pid2, 1, fullCommand);
				newJob->pid = pid2;
				if(aux.control == 1)
				{
					log_and_list(pid1, pid2, fullCommand);
				}
				else
				{
					log_or_list(pid1, pid2, fullCommand);
				}
				if(aux.is_bg != 1)
				{
					addFgJob(newJob);
					waitpid(pid2, &status2, 0);
					removeFgJob();
					logJobEnd(pid2, fullCommand, aux);	
				}
				else
				{
					addBgJob(newJob);
					waitpid(pid2, &status2, 0);      // NEW FIX
					removeBgJobPid(pid2);            // NEW FIX
					logJobEnd(pid2, fullCommand, aux);   // NEW FIX
				}
				sigprocmask(SIG_SETMASK, &prev_mask2, NULL);
			}
		}		
		else                                                // Else, stop the list if conditions were not met to continue the AND/OR list
		{
			if(aux.control == 1)
			{
				log_and_list(pid1, -1, fullCommand);
			}
			else
			{
				log_or_list(pid1, -1, fullCommand);
			}
			logJobEnd(pid1, fullCommand, aux);              // Log the end of the job
		}
	}
}




/* Performs file redirection using the given
 * in_fle and out_file
 * If file is not found, appropriate message is logged
 */
void performRedirection(char *argv[], char *argv2[], Cmd_aux aux)
{
	int pid;
	int status;
	sigset_t mask, prev_mask;
	int fdInput = -1;
	int fdOutput = -1;
	char fullCommand[150];
	getCommandName(fullCommand, argv, argv2, aux);
	sigaddset(&mask, SIGCHLD);
	sigprocmask(SIG_BLOCK, &mask, &prev_mask);       // Block SIGCHLD to prevent race
	if((pid = fork()) == 0)
	{
		setpgid(0, 0);
		if(aux.in_file != NULL)
		{        
			if((fdInput = open(aux.in_file, O_RDONLY, 0600)) < 0)   // Open in_file for input reading
			{
				log_file_open_error(aux.in_file);
				exit(1);
			}
			dup2(fdInput, STDIN_FILENO);      // Adjust the FDs
		}
		if((aux.out_file != NULL) && (aux.is_append == 0))
		{                                  
			if((fdOutput = open(aux.out_file, O_WRONLY | O_TRUNC | O_CREAT, 0600)) < 0)   // Open out_file for truncate/erase
			{	
				log_file_open_error(aux.out_file);
				exit(1);
			}
			dup2(fdOutput, STDOUT_FILENO);        // Adjust the FDs
		}
		if((aux.out_file != NULL) && (aux.is_append == 1))
		{                                  
			if((fdOutput = open(aux.out_file, O_WRONLY | O_APPEND | O_CREAT, 0600)) < 0)  // Open out_file for appending
			{	
				log_file_open_error(aux.out_file);
				exit(1);
			}
			dup2(fdOutput, STDOUT_FILENO);        // Adjust the FDs
		}
		executeProgram(fullCommand, argv, prev_mask);           // Finally, execute the program using the new FDs
	}
	logJobStart(pid, fullCommand, aux);
	Job *newJob = generateJob(0, pid, 1, fullCommand);
	if(aux.is_bg != 1)                    // Add the job to foreground or background depending or args
	{
		addFgJob(newJob);
		waitpid(pid, &status, 0);
		removeFgJob();
		logJobEnd(pid, fullCommand, aux);
	}
	else
	{
		addBgJob(newJob);
	}
	sigprocmask(SIG_SETMASK, &prev_mask, NULL);            // Restore mask
}


	

