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

glob_t globbuf;

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

int Count(char **token){
	int num = 0;
	while(token[num] != NULL){
		++num;
	}
	return num;
}

int FindElement(char key, char **target){
	int i = 0, j;
	while(target[i] != NULL){
		for(j = 0; j < strlen(target[i]); ++j){
			if(target[i][j] == key){ return i; }
		}
		++i;
	}
	return -1;
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
}

int PerformCommand(char **token){
	if(!fork()){
		printf("I am a child and my pid is %d\n", getpid());
		setenv("PATH", "/bin:/usr/bin:.", 1);
		signal(SIGINT, SIG_DFL);signal(SIGQUIT, SIG_DFL);signal(SIGTERM, SIG_DFL);signal(SIGTSTP, SIG_DFL);
		int starPosition = FindElement('*', token);
		if(starPosition == -1){
			if(execvp(token[0], token) == -1){ 
				int errsv = errno;
				printf("error number is %d\n", errsv);
				if(errsv == 2){
					printf("[%s]: command not found\n", token[0]); 
				}
				else{ printf("[%s]: unknown error\n", token[0]); }
				exit(errsv); 
			}
		}
		else{
			int num = Count(token);	
			globbuf.gl_offs = num - 1;

			glob(token[starPosition], GLOB_DOOFFS | GLOB_NOCHECK, NULL, &globbuf);
			int i;
			for(i = 0; i < num; ++i){
				globbuf.gl_pathv[i] = token[i];
			}
			if(execvp(globbuf.gl_pathv[0], globbuf.gl_pathv) == -1){
				int errsv = errno;
				printf("error number is %d\n", errsv);
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
	return 0;
}

