/*
Created by Gin, Feb 29, 2016, CUHK
*/

#include <stdio.h>
#include <string.h>	// Needed by strtok(), strcmp()
#include <limits.h>	// Needed by PATH_MAX
#include <unistd.h>	// Needed by getcwd(), chdir(), exec*()
#include <stdlib.h>	// Needed by setenv()
#include <glob.h>	// Needed by glob(), glob_f
#include <signal.h>	// Needed by signal()
#include <sys/types.h>
#include <sys/wait.h>	// Needed by wait()

#define MAXLENOFCOMMAND 255

#if 0
char **TokenInput(char *input){
	char token[MAXLENOFCOMMAND][MAXLENOFCOMMAND];
	char *tmp = (char *)malloc(sizeof(char) * MAXLENOFCOMMAND);
	int i = 0;
	tmp = strtok(input, " ");// the second variable is the character you wanna to skip
	while(tmp != NULL){ 	
	//	token[i] = (char *)malloc(sizeof(char) * MAXLENOFCOMMAND);
		strcpy(token[i], tmp);
		++i;
		tmp = strtok(NULL, " ");
	}
	return token;
}
#endif

char **TokenInput(char *input){
	char **token = (char **)malloc(MAXLENOFCOMMAND * sizeof(char *));
	char *tmp = (char *)malloc(sizeof(char) * MAXLENOFCOMMAND);
	int i = 0;
	tmp = strtok(input, " ");// the second variable is the character you wanna to skip
	while(tmp != NULL){ 
		token[i] = (char *)malloc(sizeof(char) * strlen(tmp));
		strcpy(token[i], tmp);
		tmp = strtok(NULL, " ");
		++i;
	}
	return token;
}

int PerformBuiltIn(char **token){
	if((strcmp(token[0], "exit") == 0) && (token[1] == NULL)){
		exit(0);	// 0 is the return value when the program exits
	}
	else if(strcmp(token[0], "cd") == 0){
		if(token[1] == NULL){
			if(chdir(getenv("HOME")) != -1){ return 0; }
			else{  return -1; }
		}
		else{
			if(strcmp(token[1], "~") == 0){
				if(chdir(getenv("HOME")) != -1){ return 0; }
				else{  return -1; }
			}
			else{
				if(chdir(token[1]) != -1){ return 0; }
				else{ 
					printf("3150 shell: cd %s: No such file or directory\n", token[1]);
					return -1; 
				}
			}
		}
	}
	return 0;
}

int PerformCommand(char **token){
	if(!fork()){
		printf("I am a child and my pid is %d\n", getpid());
		setenv("PATH", "/bin:/usr/bin:.", 1);
		signal(SIGINT, SIG_DFL);
		signal(SIGQUIT, SIG_DFL);
		signal(SIGTERM, SIG_DFL);
		signal(SIGTSTP, SIG_DFL);
		int ret = execvp(token[0], token);
		if(ret == -1){ printf("[%s]: command not found\n", token[0]); return ret; }
		else{ printf("[%s]: unknown error\n", token[0]); return ret; }
		return ret;
	}
	else{
		wait(NULL);
	}
}

int Perform(char **token){
	if(strcmp(token[0], "cd") == 0 || strcmp(token[0], "exit") == 0){ 
		PerformBuiltIn(token);
		return 0;
	}
	else{
		PerformCommand(token);
		return 0;
	}
}

void HandleSig(){
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGTERM, SIG_IGN);
	signal(SIGTSTP, SIG_IGN);
}

int main(int argc, char *argv[])
{
	printf("My pid is %d\n", getpid());
	HandleSig();
	char input[MAXLENOFCOMMAND];
	char buf[PATH_MAX + 1];
	while(1){
		if(getcwd(buf, PATH_MAX + 1) !=NULL ){
			printf("[3150 shell:%s]$ ", buf);
			fgets(input, MAXLENOFCOMMAND, stdin);
			if(strcmp(input, "\n") != 0){
				input[strlen(input) - 1] = '\0';
				char **token = TokenInput(input);
				Perform(token);
			}
		}
	}
#if 0
	printf("\nPress Enter to execute ls...");
	while(getchar() != '\n');
	pid_t child_pid;
	if(!(child_pid = fork())){	
		char *arglist[] = {"ls", NULL};
		execvp(*arglist, arglist);
	}
	else{
		wait(NULL);
	}
#endif

#if 0
	glob_f globbuf;

	globbuf.gl_offs = 1;

	glob("*.c", GLOB_DOOFFS | GLOB_NOCHECK, NULL, &globbuf);
	glob("*.txt", GLOB_DOOFFS | GLOB_NOCHECK | GLOB_APPEND, NULL, &globbuf);
	glob("*.java", GLOB_DOOFFS | GLOB_NOCHECK | GLOB_APPEND, NULL, &globbuf);
	globbuf.gl_pathv[0] = "ls";

	evecvp(globbuf.gl_pathv[0], globbuf.gl_pathv);
#endif
	return 0;
}

