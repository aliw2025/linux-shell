#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "builtin.h"
#include "parse.h"
#include "plist.h"
#include <ctype.h>

int bg_id=1;
int fg_id=1;
int newid=0;
static char* builtin[] = {
    "exit",   /* exits the shell */
    "which",  /* displays full path to command */
    "jobs",  /* displays all active jobs*/
    "fg",  /* move process to foreground*/
    "bg",  /* move process to background*/
    "kill",  /*send signal to a process*/
    NULL
};

/// print jobs
void print_jobs(){
	// take length of queue
	int n=length(q);
	// dispay naumber of active jobs
	int pgid=-1;
	struct PCB *current=q;
	int i;
	// loop throught process and print them
	for( i=0;i<n;i++){
		if(current->pgid==pgid){
		
			printf(" | %s ", current->cmd);
		}else{
			printf("\n[%d] + %s  %s",current->id,current->status, current->cmd);
		}
		pgid=current->pgid;
		current=current->next;
	}
	printf("\n");
}

void remove_pg(int pgid){
	
		int i;
		struct PCB * curr=q;
		struct PCB * del=NULL;

		int n=length(q);
		int flag=0;
		for( i=0;i<n;i++){
			
			if(curr->pgid==pgid){
				//fg_id--;
				if(!flag){
					flag=1;
					printf("[%d] + done %s ",curr->id,curr->cmd);
				}else{
					printf("| %s ",curr->cmd);
				}	
				del=delete(&q,curr->pid);
				if(del!=NULL){
					free(del);
				}	
			}
			curr=curr->next;
		}
		newid--;
		printf("\n");
}

void stop_pg(int pgid){
	
		int i;
		struct PCB * curr=q;
	
		int n=length(q);
		int flag=0;
		for( i=0;i<n;i++){
			
			if(curr->pgid==pgid){
				if(!flag){
					flag=1;
					printf("[%d] + suspended %s ",curr->id,curr->cmd);
				}else{
					printf("| %s ",curr->cmd);
				}			
				strcpy(curr->status ,"stopped");	
			}
			curr=curr->next;
		}
		printf("\n");
}

void continue_pg(int pgid){
	
		int i;
		struct PCB * curr=q;
	
		
		int n=length(q);
		int flag=0;
		for( i=0;i<n;i++){
			
			if(curr->pgid==pgid){
				
				if(!flag){
					flag=1;
					printf("[%d] + continued %s ",curr->id,curr->cmd);
				}else{
					printf("| %s ",curr->cmd);
				}			
				strcpy(curr->status ,"running");	
			}
			curr=curr->next;
		}
		printf("\n");
}



void sig_handler(int signum){

	pid_t pid;
  	int status;
	struct PCB *current=q;
	int es;
	int i;
	int n=length(q);
	//printf("called\n");
	signal(SIGCHLD,sig_handler); 
	// loop through each process and check its status
	for( i=0;i<n;i++){
		/// getting the status of current child
		if(!current->is_bg){
			current=current->next;
			continue;
		}
		if ((pid = waitpid(current->pid, &status, WNOHANG| WUNTRACED |WCONTINUED )) == -1){
			//printf("no %d\n",current->pid);
			perror("wait() error\n");
		} // pid 0 means child is running state	
    		else if (pid == 0) {
      			//register the singal handler agian
			signal(SIGCHLD,sig_handler); 
    		}
 		else {	// if child is exited normaly
 			
			if (WIFSTOPPED (status)) {
				//child received SIGTSTP 
				//printf("stoped\n");
				es = WEXITSTATUS(status);	
		 		//printf("stop Exit status was %d\n", es);
		 		stop_pg(current->pgid);
				
				
			} else if (WIFCONTINUED (status)) {
				// child received SIGCONT 
				//printf("cont\n");
				continue_pg(current->pgid);
				i=0;
				
				
			}
			else if (WIFSIGNALED(status)) {
				// child received SIGCONT 
				//printf("sig\n");
				//del=delete(&q,current->pid);
				remove_pg(current->pgid);
			}
			else if (WIFEXITED(status)){
				// geting ther return value of te child
				es = WEXITSTATUS(status);
				//printf("background process :%d  %s terminated\n", current->pid, current->cmd);
				//del=delete(&q,current->pid);
				remove_pg(current->pgid);
		 		//printf("Exit status was %d\n", es);
			}	
			es = WEXITSTATUS(status);
			if(es==0){
			}					
    		}// move to next process
		if(current->next!=NULL)
		current=current->next;
	}
	
}



int handle_process( int pgid ,int pid, char *cmd,int type){
	// crating a node for list
	//signal(SIGCHLD,sig_handler); 
	struct PCB * temp;
	if(type==0){
		//printf("finding : %d\n",pid);
		temp=find(&q,pid);
		if(temp!=NULL){
			//printf("found him\n");
		}
	}
	else{
		temp= (struct PCB*)malloc(sizeof(struct PCB));
		temp->pid=pid;
		temp->pgid=pgid;
		temp->id=newid;
		strcpy(temp->cmd,cmd);
		strcpy(temp->status,"running");	
		temp->next=NULL;
	}
	//signal(SIGCHLD,sig_handler); 
	
	if(pid==pgid && type!=0){
    		printf("[%d] ",newid);
	}
	if(type==1){
		temp->is_bg=0;				
	
		printf("%d\n",pid);
		newid++;
	}else if(type==2){
		signal(SIGCHLD,sig_handler); 
		temp->is_bg=1;				
		
		printf("%d\n",pid);
		newid++;
		
	}else if(type==3){
		temp->is_bg=0;
		printf("%d ",pid);
	}
	else if(type==4){
		signal(SIGCHLD,sig_handler); 
		temp->is_bg=1;
		printf("%d ",pid);
	}
	if(type!=0)
	insert(&q,temp);
	
	if(type >1){
		return 0;
	}
	
	// as its fg process we need to wait for its termintion
	int status;
	int es;
	getpgid(pid);
	pid_t tempss=pid;
	int flags= WNOHANG| WUNTRACED| WCONTINUED;
	//printf("pid is : %d gid is :%d \n",pid,getpgid(pid));
	// continously loop and check status of current active fg process
	do {
		/// getting the status of current child
		pid = waitpid(tempss, &status, flags); 
		//printf("s: %d",pid);
		if (pid== -1){
			perror("waitpid error: ");
		}// if running
		else if (pid == 0) {
			//printf("0");			
			signal(SIGCHLD,sig_handler);
		}
		else {	// if exited
			
			if (WIFSTOPPED (status)) {
				//child received SIGTSTP 
				//printf("stoped\n");
				es = WEXITSTATUS(status);	
		 		//printf("stop Exit status was %d\n", es);
 				stop_pg(pgid);
				
				
				
			} else if (WIFCONTINUED (status)) {
				// child received SIGCONT 
				//printf("cont\n");
				continue_pg(pgid);
				pid=0;
			
			}else if (WIFSIGNALED(status)) {
				// child received SIGCONT 
				//printf("sig\n");
			}
			else if (WIFEXITED(status)){
				// geting ther return value of te child
				es = WEXITSTATUS(status);
		 		//printf("Exit status was %d\n", es);
			}					
			es = WEXITSTATUS(status);	
		}
	}while(pid==0); // loop if zero means process running
	if(es!=1 && es!=2 ){
		//printf("process %d,  %s terminated\n",pid, cmd);
		signal(SIGCHLD,sig_handler);
	}
	if(es==0){ // in cased of 2 stopped process stays in list otherwise deleted
		//printf("removing pg\n");
		remove_pg(pgid);
	
	}
	void (*old)(int);
  	old = signal (SIGTTOU, SIG_IGN);
	tcsetpgrp (STDIN_FILENO, getpid());
	tcsetpgrp (STDOUT_FILENO, getpid());
	signal (SIGTTOU, old);
				
	
return es;
}



int is_builtin (char* cmd)
{
    int i;

    for (i=0; builtin[i]; i++) {
        if (!strcmp (cmd, builtin[i]))
            return 1;
    }

    return 0;
}
char *  which (const char* cmd)
{
    char* dir;
    char* tmp;
    char* PATH;
    char* state;
    char *probe=(char *)malloc(sizeof(char )*PATH_MAX);
	
    for(int i=0;i<2;i++){
    
    	if(!strcmp(cmd,builtin[i])){
    	
    		sprintf(probe,"%s : shell built-in command",builtin[i]);
    		return probe;	
    	}
    }

    if (access (cmd, X_OK) == 0)
        return NULL;

    PATH = strdup (getenv("PATH"));

    for (tmp=PATH; ; tmp=NULL) {
        dir = strtok_r (tmp, ":", &state);
        if (!dir)
            break;

        strncpy (probe, dir, PATH_MAX-1);
        strncat (probe, "/", PATH_MAX-1);
        strncat (probe, cmd, PATH_MAX-1);

        if (access (probe, X_OK) == 0) {
            //ret = 1;
            return probe;
            break;
        }
    }

    //free (PATH);
    return NULL;
}

int count_argv(char **argv){
	int i=0;
	
	while(argv[i]!=NULL){
	
		i++;	
	
	}
	return i;
}

int valid_id(char *argv,int len){
	
	
	for(int i=0;i<len;i++){
		
		if(!isdigit(argv[i])){
			return 0;
		}
			
	}
	return 1;
}



void builtin_execute (Task T)
{
	
    if (!strcmp (T.cmd, "exit")) {
    
        exit (EXIT_SUCCESS);
    }else if (!strcmp (T.cmd, "which")){
    
    	char * path=which(T.argv[1]);
    	if(path!=NULL){
    		printf("%s\n",path );
    		free(path);
    	}
    	
    }else if (!strcmp (T.cmd, "jobs")){
    
    		print_jobs();
    	
    }else if(strcmp(T.cmd,"fg")==0){
		char tempid[20];
		strcpy(tempid,T.argv[1]);
		strcpy(tempid,tempid+1);
		int id=atoi(tempid);
		// first find the process in the list
		struct PCB *current=find_by_id(&q,id);
		// if process is not found in the list
		if(current==NULL){
			// print error
			printf("no process found\n");
		}else { // if process found
			/// print the process start info
			// print start message
			int pgid=current->pgid;
			int n=length(q);
			struct PCB *curr=q;
			for(int i=0;i<n;i++){
				
				if(curr->pgid==pgid){
					signal(SIGCHLD,sig_handler);
					curr->is_bg=0; 			
				}
				curr=curr->next;
			}
			// if process was stoped process
			if(strcmp(current->status,"running")!=0){
				// change state to running
				strcpy(current->status,"running");
				// send continue siganal to the process
				killpg(current->pid,18);
				// print continue message
			}
			// as process is not foreground wait for it
			void (*old)(int);
			old = signal (SIGTTOU, SIG_IGN);
			tcsetpgrp (STDIN_FILENO,current->pgid );
			tcsetpgrp (STDOUT_FILENO, current->pgid);
			signal (SIGTTOU, old);
			handle_process(current->pgid ,current->pid, current->cmd,0);
			
			
		}	
	}
	else if(strcmp(T.cmd,"bg")==0){ 
		// convert the id to int
		char tempid[20];
		strcpy(tempid,T.argv[1]);
		strcpy(tempid,tempid+1);
		int id=atoi(tempid);
		//printf("id %d\n",id);
		// look process in the list
		struct PCB *current=find_by_id(&q,id);
		// if not foudn exit
		if(current==NULL){
			// print error message	
			printf("no process found\n");
		
		}else {
			// print start message
			int pgid=current->pgid;
			int n=length(q);
			struct PCB *curr=q;
			for(int i=0;i<n;i++){
				
				if(curr->pgid==pgid){
					signal(SIGCHLD,sig_handler);
					curr->is_bg=1; 			
				}
				curr=curr->next;
			}
			
			// if process is stoped
			if(strcmp(current->status,"running")!=0){
				// chagne status to running			
				strcpy(current->status,"running");
				// send continue signal
				killpg(current->pid,18);
				// print termination message
				
			}
			
		}	
	}
	else if(strcmp(T.cmd,"kill")==0){
		// convert pid and signal to int
		int sig;
		int i;
		int id;
		int count=count_argv(T.argv);
		if(count<2){
			 
			 	printf("Usage: kill [-s <signal>] <pid> | %s<job> ...\n","%");
			 	return;
		 }
		if(strcmp(T.argv[1],"-s")==0){
				 
			 if(count<4){
			 
			 	printf("Usage: kill [-s <signal>] <pid> | %s<job> ...\n","%");
			 	return;
			 }
			 if(valid_id(T.argv[2],strlen(T.argv[2])))
			 sig=atoi(T.argv[2]);
			 else{
			
			   printf("pssh: invalid signal: [%s]\n",T.argv[2]);
			   return ;								 
	 		}
			 	
			 i=3;			
		}else{
			
			sig =SIGTERM;
			i=1;
		}
		
		while(T.argv[i]!=NULL){
			struct PCB *current;
			if(T.argv[i][0]=='%'){
			
				if(strlen(T.argv[i])<2){
					printf("pssh: invalid job number: [%s]\n",T.argv[i]);				
					i++;
					continue;
				}
				
				char tempid[20];
	
				strcpy(tempid,T.argv[i]);
				strcpy(tempid,tempid+1);
				if(valid_id(tempid,strlen(tempid))){
					id=atoi(tempid);
				}else{
					printf("-pssh: invalid job number: [%s]\n",tempid);
					i++;
					continue;	
				}
				
				//printf("ID : %d\n",id);
				current=find_by_id(&q,id);
				if(current==NULL ){
					// print error
					printf("pssh: invalid job number: [%d]\n",id);
					i++;
					continue;
				}
					
				
			}else{
				if(valid_id(T.argv[i],strlen(T.argv[i]))){
					id=atoi(T.argv[i]);
				}else{
					printf("-pssh: invalid pid: [%s]\n",T.argv[i]);
					i++;
					continue;	
				}
				id=atoi(T.argv[i]);
				current=find(&q,id);
				if(current==NULL){
					// print error
					printf("pssh: invalid pid: [%d]\n",id);
					i++;
					continue;										
				}
			}
			//printf("sig %d\n",sig);
			killpg(current->pid,sig);
			//perror("status");
			
			if(current->is_bg!=1 ){
				if(strcmp(current->status,"stopped")!=0 || sig==18 ){	
				void (*old)(int);
				old = signal (SIGTTOU, SIG_IGN);
				tcsetpgrp (STDIN_FILENO,current->pgid );
				tcsetpgrp (STDOUT_FILENO, current->pgid);
				signal (SIGTTOU, old);
				handle_process(current->pgid ,current->pid, current->cmd,0);
				}
			}else{
				int pgid=current->pgid;
				int n=length(q);
				struct PCB *curr=q;
				for(int i=0;i<n;i++){
				
					if(curr->pgid==pgid){
						signal(SIGCHLD,sig_handler);
						curr->is_bg=1; 			
					}
					curr=curr->next;
				}

			}	
			i++;	
		}
			
	}
   	 else {
        	printf ("pssh: builtin command: %s (not implemented!)\n", T.cmd);
    	}
}



