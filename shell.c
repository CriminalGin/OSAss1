/*
Created by Gin, Feb 29, 2016, CUHK
*/

#include <stdio.h>
#include <string.h>	// Needed by strtok(), strcmp()
#include <limits.h>	// Needed by PATH_MAX
#include <signal.h>	// Needed by signal()
#include <unistd.h>	// Needed by getcwd(), chdir(), exec*()
#include <stdlib.h>	// Needed by setenv()
#include <sys/types.h>
#include <sys/wait.h>	// Needed by wait()
#include <errno.h>	// Needed by errno
#include <glob.h>	// Needed by glob(), glob_f

#define MAXLENOFCOMMAND 255

char **TokenInput(char *input){
	char **token = (char **)malloc( (MAXLENOFCOMMAND + 1) * sizeof(char *));
	char *tmp = (char *)malloc(sizeof(char) * (MAXLENOFCOMMAND + 1));
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

int Count2d(char **token){
	int num = 0;
	while(token[num] != NULL){
		++num;
	}
	return num;
}

int *FindElement(char key, char **target){
	int i = 0, j, k = 1;
	int num = 0;
	for(i = 0; i < Count2d(target); ++i){
		for(j = 0; j < strlen(target[i]); ++j){
			if(target[i][j] == key){ ++num; }
		}
	}
	if(num == 0){ return NULL; }
	else{
		int *positionArray = (int *)malloc(sizeof(int) * (num + 1) );
		for(i = 0; i < Count2d(target); ++i){
			for(j = 0; j < strlen(target[i]); ++j){
				if(target[i][j] == key){ positionArray[k] = i; ++k; }
			}
		}
		positionArray[0] = num; 
		return positionArray;
	}
}
 
int PerformBuiltIn(char **token){
	if(strcmp(token[0], "exit") == 0){ 
		if(token[1] == NULL){ 
			exit(0);	// 0 is the return value when the program exits
		}
		else{
			printf("exit: wrong number of arguments\n");
			return -1;
		}
	}
	else if(strcmp(token[0], "cd") == 0){
		if(token[1] == NULL){
			if(chdir(getenv("HOME")) != -1){ return 0; }
			else{  return -1; }
		}
		else{
			if(token[2] != NULL){
				printf("cd: wrong number of arguments\n");
			}
			else{
				if(strcmp(token[1], "~") == 0){
					if(chdir(getenv("HOME")) != -1){ return 0; }
					else{  return -1; }
				}
				else{
					if(chdir(token[1]) != -1){ return 0; }
					else{ 
						printf("[%s]: cannot change directory \n", token[1]);
						return -1;
					}
				}
			}
		}
	}
}
#if 1
int PerformCommand(char **token){
	if(!fork()){
		setenv("PATH", "/bin:/usr/bin:.", 1);
		signal(SIGINT, SIG_DFL);signal(SIGQUIT, SIG_DFL);signal(SIGTERM, SIG_DFL);signal(SIGTSTP, SIG_DFL);
		int *starPosition = FindElement('*', token);
		if(starPosition == NULL){
			if(execvp(token[0], token) == -1){ 
				int errsv = errno;
				if(errsv == 2){
					printf("[%s]: command not found\n", token[0]); 
				}
				else{ printf("[%s]: unknown error\n", token[0]); }
				exit(errsv); 
			}
		}
		else{
			int tokenNum = Count2d(token), starNum = starPosition[0];
			glob_t globbuf;
			globbuf.gl_offs = tokenNum - starNum;
			int i;
			glob(token[starPosition[1]], GLOB_DOOFFS | GLOB_NOCHECK, NULL, &globbuf);
			for(i = 2; i <= starNum; ++i){
				glob(token[starPosition[i]], GLOB_DOOFFS | GLOB_NOCHECK | GLOB_APPEND, NULL, &globbuf);
			}
			for(i = 0; i < globbuf.gl_offs; ++i){
				globbuf.gl_pathv[i] = token[i];
			}
			if(execvp(globbuf.gl_pathv[0], globbuf.gl_pathv) == -1){
				int errsv = errno;
				if(errsv == 2){
					printf("[%s]: command not found\n", token[0]); 
				}
				else{ printf("[%s]: unknown error\n", token[0]); }
				exit(errsv); 
			}
		}

	}
	else{
		wait(NULL);
	}
}
#endif
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
	signal(SIGINT, SIG_IGN);signal(SIGQUIT, SIG_IGN);signal(SIGTERM, SIG_IGN);signal(SIGTSTP, SIG_IGN);
	signal(SIGSTOP, SIG_DFL); signal(SIGKILL, SIG_DFL);
}

int main(int argc, char *argv[])
{
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
	return 0;
}
