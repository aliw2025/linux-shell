#ifndef _builtin_h_
#define _builtin_h_

#include "parse.h"


int is_builtin (char* cmd);
void sig_handler(int signum);
void print_jobs();
int kill_jobs(int sig);
int handle_process( int pgid ,int pid, char *cmd,int type);
void builtin_execute (Task T);
int builtin_which (Task T);
char *  which (const char* cmd);

#endif /* _builtin_h_ */
