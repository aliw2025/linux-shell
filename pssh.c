#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include<sys/wait.h>
#include <unistd.h>
#include <readline/readline.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "builtin.h"
#include "parse.h"
#include <signal.h>

/*******************************************
 * Set to 1 to view the command line parse *
 *******************************************/
#define DEBUG_PARSE 0


static const char *shell_path[] = { "/bin/", "/usr/bin/", NULL };
void print_banner ()
{
    printf ("                    ________   \n");
    printf ("_________________________  /_  \n");
    printf ("___  __ \\_  ___/_  ___/_  __ \\ \n");
    printf ("__  /_/ /(__  )_(__  )_  / / / \n");
    printf ("_  .___//____/ /____/ /_/ /_/  \n");
    printf ("/_/ Type 'exit' or ctrl+c to quit\n\n");
}



/* returns a string for building the prompt
 *
 * Note:
 *   If you modify this function to return a string on the heap,
 *   be sure to free() it later when appropirate!  */
static char* build_prompt ()
{
	//fflush(std);
    return  "$ ";
}


/* return true if command is found, either:
 *   - a valid fully qualified path was supplied to an existing file
 *   - the executable file was found in the system's PATH
 * false is returned otherwise */
static int command_found (const char* cmd)
{
    char* dir;
    char* tmp;
    char* PATH;
    char* state;
    char probe[PATH_MAX];

    int ret = 0;

    if (access (cmd, X_OK) == 0)
        return 1;

    PATH = strdup (getenv("PATH"));

    for (tmp=PATH; ; tmp=NULL) {
        dir = strtok_r (tmp, ":", &state);
        if (!dir)
            break;

        strncpy (probe, dir, PATH_MAX-1);
        strncat (probe, "/", PATH_MAX-1);
        strncat (probe, cmd, PATH_MAX-1);

        if (access (probe, X_OK) == 0) {
            ret = 1;
            break;
        }
    }

    free (PATH);
    return ret;
}
// function to execute non builtin tasks
void exec_task(char *cmd,char *argv[]){
		char path[256];
		//char cmd[500];
		// create first path
		sprintf(path,"%s%s",shell_path[1],argv[0]);		
		//call execvp to replace cureent process with program
		int r=execv(path, argv);
		// if failed to find in first path
		if(r==-1){
			// create 2nd path
			sprintf(path,"%s%s",shell_path[0],argv[0]);
			//printf("path2:%s\n",path);
			r=execv(path, argv);
			// if not found in 2nd path also report error
			if(r==-1){
				printf("error: %s cannot run\n",cmd);
				exit(-1);
			}							
		}
			
}
void handler_sigttin (int sig)
{
    while (tcgetpgrp(STDIN_FILENO) != getpid ())
        pause ();
}

void handler_sigttou (int sig)
{
    while (tcgetpgrp(STDOUT_FILENO) != getpid ())
        pause ();
}

// signal handler for sigint
void sig_handler1(int signum){
		// print the  signal recieved message 
		// this task requrie to change the group if child process
		// group changing is done in process creation
		// again registering the handler to recieve next signal
		signal(SIGINT,sig_handler1);
	 //	int r=kill_jobs(SIGINT);
	 	//if(!r)
		//	exit(0);		
		//puts("i am sig1");
}
// ssignal handler for sigstp
void sig_handler2(int signum)
{		// print the signal recieved message		
		//log_ctrl_z();
		// register the singal again to recive next signal

		signal(SIGTSTP,sig_handler2);
	 	//int r=kill_jobs(SIGTSTP);
}
// structure for pipes
struct pipe_struct{

	 int pipe_fds[2];
};
/* Called upon receiving a successful parse.
 * This function is responsible for cycling through the
 * tasks, and forking, executing, etc as necessary to get
 * the job done! */
void execute_tasks (Parse* P)
{  	
    	unsigned int t;
    	// crate pipes for data transfer bw childs    	
    	struct pipe_struct pipes[P->ntasks];   	
    	int fd_in,fd_out;
	char com[500];
	pid_t job_pids[P->ntasks];
	for (t = 0; t < P->ntasks; t++) {
		// create current pipe if there are more than one tasks
		if(P->ntasks>1)
		pipe(pipes[t].pipe_fds);
        	
        	if (is_builtin (P->tasks[t].cmd)) {
            		builtin_execute (P->tasks[t]);
        	}
        	else if (command_found (P->tasks[t].cmd)) {
      			// create a child process
	    		int pid=fork();
	    		job_pids[t] = pid;
			setpgid (job_pids[t], job_pids[0]);
	    		// if its a child process 
    			if(pid==0){
    				// if tasks are more tha 1
    				if(P->ntasks>1){
    					// if its the first task
    					if(t==0){
  						// no need for reading from pipe
  						close(pipes[t].pipe_fds[0]);
  						// make the stdoout of the child point to the pipe
  						dup2(pipes[t].pipe_fds[1], 1);
  						
  					}else if (t==P->ntasks-1){ // if its the last task
  						// close the write end of the previous child
  						close(pipes[t-1].pipe_fds[1]);
  						// make the stdin of the current child to point to read 
  						// end of the pipe
						int r=dup2(pipes[t-1].pipe_fds[0], 0);
						if(r==-1){
							//printf("error\n");
							perror("er2: ");
						}
						// close the read end of the previous child
						close(pipes[t-1].pipe_fds[0]);
						
						// restore stdout as its lask task
						dup(pipes[t].pipe_fds[1]);
  			
  					}
  					else{
  						// if its not the last one or first one
  						// close the write end of the previous
  						close(pipes[t-1].pipe_fds[1]);
  						// point the current read end to previous read end
						int r=dup2(pipes[t-1].pipe_fds[0], 0);
						if(r==-1){
							//printf("error\n");
							perror("er2: ");
						}
						// close previous read end
						close(pipes[t-1].pipe_fds[0]);
						// point stdout to current pipe write end
						r=dup2(pipes[t].pipe_fds[1], 1);
						if(r==-1){	
							//printf("error\n");
							perror("er2: ");
						}
						
  					}
  				}
			// if input file is provided
			if(P->infile!=NULL){
				/// open file
		  		fd_in = open(P->infile, O_RDONLY );        
		  		if (fd_in ==-1) 
		 		{ 
        				printf("cannot open input file %s\n",P->infile);
					exit(-1);
				                 
    				}else{
    					// redirect input to input file
					dup2(fd_in,STDIN_FILENO);
						
				} 
			}
			// if output file is provided only last task need output filt
			if(t==P->ntasks-1){
				if(P->outfile!=NULL){
				
		  			fd_out = open(P->outfile, O_WRONLY |O_CREAT,0777);
	  		 
    					if (fd_out ==-1) 
    					{ 
        					printf("cannot open output file %s\n",P->outfile);
						exit(-1);                 
    					}else{
    						// redirect out to output file
						dup2(fd_out,STDOUT_FILENO);
					} 
				}	
			}	
        	    	exec_task(P->tasks[t].cmd,P->tasks[t].argv);
    		}
	    	else{	// if its a parent process it wait for the child to finish				
	    		//wait(NULL);

	    		if(!P->background && t==P->ntasks-1){
	    			// handle fg

				// passing it to handling function
				int i=0;
				strcpy(com,"");
				//strcat(com,P->tasks[t].cmd);
				//strcat(com," ");	
				while(P->tasks[t].argv[i]!=NULL){
				
					strcat(com,P->tasks[t].argv[i]);
					strcat(com," ");	
					i++;	
				}
	    			//printf("foreground process %d started running %s\n",pid, P->tasks[t].cmd);
				//printf("%d ",pid); 
				//handle_fg_process(job_pids[0],pid,com);
				void (*old)(int);
			  	old = signal (SIGTTOU, SIG_IGN);
				tcsetpgrp (STDIN_FILENO, job_pids[0]);
				tcsetpgrp (STDOUT_FILENO, job_pids[0]);
				signal (SIGTTOU, old);
				handle_process(job_pids[0],pid,com,1);	
	    		}
	    		else if(P->background && t==P->ntasks-1){
	 			int i=0;
				strcpy(com,"");
				//strcat(com,P->tasks[t].cmd);
				//strcat(com," ");	
				while(P->tasks[t].argv[i]!=NULL){
				
					strcat(com,P->tasks[t].argv[i]);
					strcat(com," ");	
					i++;	
				}
				strcat(com," &");
				//printf("background process %d  %s started\n",pid, P->tasks[t].cmd);
				//handle_bg_process(job_pids[0],pid,com);
				
				handle_process(job_pids[0],pid,com,2);
				
	    		
	    		}else{
	    			int i=0;
				strcpy(com,"");
				//strcat(com,P->tasks[t].cmd);
				//strcat(com," ");	
				while(P->tasks[t].argv[i]!=NULL){
				
					strcat(com,P->tasks[t].argv[i]);
					strcat(com," ");	
					i++;	
				}

				if(!P->background){
					handle_process(job_pids[0],pid,com,3);
					//printf("foreground process %d  %s piped\n",pid, P->tasks[t].cmd);
				}else{
					handle_process(job_pids[0],pid,com,4);
					//printf("background process %d  %s piped\n",pid, P->tasks[t].cmd);
				}
				//handle_pipe_process(job_pids[0],pid,P->com);
	    		}
	    		// close the unused pipe end.. this one condition took 6 hours 
	    		if(P->ntasks>1)
	    			close(pipes[t].pipe_fds[1]);
		        	         
	    	}
        	    
        }
        else {
            printf ("pssh: command not found: %s\n", P->tasks[t].cmd);
            break;
        }
    }
}


int main (int argc, char** argv)
{
    //signal(SIGINT,sig_handler1);
    //signal(SIGTSTP,sig_handler2);
  //  int x=SIGTSTP;
    //printf("l %d",x);
    //signal (SIGCHLD, handler_sigchld);
    signal (SIGTTIN, handler_sigttin);
    signal (SIGTTOU, handler_sigttou);
    
    char* cmdline;
    Parse* P;

    print_banner ();

    while (1) {
	
        cmdline = readline (build_prompt());
        if (!cmdline)       /* EOF (ex: ctrl-d) */
            exit (EXIT_SUCCESS);
      
        P = parse_cmdline (cmdline);
        if (!P)
            goto next;

        if (P->invalid_syntax) {
            printf ("pssh: invalid syntax\n");
            goto next;
        }

#if DEBUG_PARSE
        parse_debug (P);
#endif

        execute_tasks (P);

    next:
        parse_destroy (&P);
        free(cmdline);
    }
 //               printf ("here\n");
}
