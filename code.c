/*
Created by Gin, Feb 29, 2016
*/

#include <stdio.h>
#include <string.h>	// Needed by strtok()
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
//		printf("tmp = %s %d\n", tmp, i);
		token[i] = (char *)malloc(sizeof(char) * strlen(tmp));
		strcpy(token[i], tmp);
		tmp = strtok(NULL, " ");
		++i;
	}
	return token;
}

int PerformBuiltIn(char **token){
	if(strcmp(token[0], "exit") && token[1] == NULL){
		exit(0);	// 0 is the return value when the program exits
	}
	else if(token[0] == "cd"){
		if(chdir(getenv("HOME")) == 0){ return 0; }
		else{  return -1; }
	}
	return 0;
}

void HandleSig(){
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGTERM, SIG_IGN);
	signal(SIGTSTP, SIG_IGN);
	if(!fork()){
		signal(SIGINT, SIG_DFL);
		signal(SIGQUIT, SIG_DFL);
		signal(SIGTERM, SIG_DFL);
		signal(SIGTSTP, SIG_DFL);
		printf("[%d]I am Child..\n", getpid());
		while(1){}
	}
	else{
		wait(NULL);
		printf("[%d]I am super parent, kill me if you can\n", getpid());
		while(1){}
	}
}

int main(int argc, char *argv[])
{
	// HandleSig();
	char input[MAXLENOFCOMMAND];
	while(1)
	{
		fgets(input, MAXLENOFCOMMAND, stdin);
		char **token = TokenInput(input);
		PerformBuiltIn(token);	
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

#if 0
	char cwd[PATH_MAX + 1];
	char input[255];
	if(getcwd(cwd, PATH_MAX + 1) != NULL){
		printf("Current Working Dir: %s\n", cwd);
		printf("Where do you want to go?:");
		fgets(input, 255, stdin);
		input[strlen(input) - 1] = '\0';
		if(chdir(input) != -1){
			getcwd(cwd, PATH_MAX + 1);
			printf("Current Working Dir: %s\n", cwd);
		}
	}
	else{
		printf("Error Occured!\n");
	}
#endif

	return 0;
}

