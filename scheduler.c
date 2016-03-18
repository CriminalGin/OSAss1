#include <sys/times.h>	// Needed by times(), clock_t
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <glob.h>
#include <errno.h>

#define	MAXNUMOFJOB	10
#define MAXLENOFJOB	10000
#define MAXLENOFCOMMAND 255
#define CHARTOINT	48

pid_t pid;
void alarmHandler(int signal){
	kill(pid, SIGTERM);
}


int readTxt(char *filename, char **job)
{
	FILE *fp = fopen(filename, "r");
	char *line = NULL;
	size_t len = 0;
	ssize_t read;
	int i = 0;
	if(fp == NULL){ exit(EXIT_FAILURE); }
	while((read = getline(&line, &len, fp) != -1)){
		job[i] = (char *)malloc((read + 1) * sizeof(char));
		strcpy(job[i], line);
		++i;
	}
	int num = i;
	fclose(fp);
	return num;
}

int TokenInput(char *input, char *delim, char **token){
	char *tmp = (char *)malloc(sizeof(char) * (MAXLENOFCOMMAND + 1));
	int i = 0;
	tmp = strtok(input, delim);
	while(tmp != NULL){
		token[i] = (char *)malloc(sizeof(char) * strlen(tmp));
		strcpy(token[i], tmp);
		tmp = strtok(NULL, delim);
		++i;
	}
	return i;
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

 

int PerformCommand(char **token){
	setenv("PATH", "/bin:/usr/bin:.", 1);
	int *starPosition = FindElement('*', token);
	if(starPosition == NULL){
		if(execvp(token[0], token) == -1){
			int errsv = errno;
			if(errsv == 2){
				printf("[%s]: command not found\n", token[0]); 
				}
			else if(errsv != 0){ printf("[%s]: unknown error\n", token[0]); }
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

int main(int argc, char *argv[]){
	int i, j;
	char *mode = argv[1];
	printf("mode is %s\n", mode);
	char **job = (char **)malloc(MAXNUMOFJOB * sizeof(char *));
	int num = readTxt(argv[2], job);
	char ***jobToken = (char ***)malloc(num * sizeof(char**));
	int *numOfToken = (int *)malloc(num * sizeof(int));
	int *times = (int *)malloc(num * sizeof(int));
	for(i = 0; i < num; ++i){
		jobToken[i] = (char **)malloc( (MAXLENOFCOMMAND + 1) * sizeof(char *));
		numOfToken[i] = TokenInput(job[i], " \t", jobToken[i]);
		times[i] = atoi(jobToken[i][numOfToken[i] - 2]);
	}
#if 1
printf("num is %d\n", num);
if(!strcmp(mode, "FIFO")){
	for(i = 0; i < num; ++i){
		pid = fork();
		if(pid != 0){	waitpid(pid, NULL, 1);	}
		if(pid == 0){
			setenv("PATH", ":/bin:/usr/bin:.", 1);
			printf("I am child, my pid is %d, my parent pid is %d\n", getpid(), getppid());
		}
	}
}
else if(!strcmp(mode, "PARA")){
	printf("I have gone to the PARA\n");
	int **pids = (int **)malloc(num * sizeof(int *));
	for(i = 0; i < num; ++i){
		pids[i] = (int *)malloc(2 * sizeof(int));
		if(!(pids[i][0] != fork())){
			printf("I am child, my pid is%d, my parent pid is %d\n", getpid(), getppid());
			signal(SIGALRM, alarmHandler);
			pids[i][0] = getpid();
			if(!(pids[i][1] = fork())){
				pids[i][1] = getpid();
			}
		}	
		else{
			alarm(1);
			wait(NULL);
		}
	}
	for(i = 0; i < num; ++i){
		waitpid(pids[j][0], NULL, 0);
	}
}



#endif

#if 0
	signal(SIGALRM, alarmHandler);
	clock_t startTime, endTime;
	struct tms cpuTime;
	double ticks_per_sec = (double)sysconf(_SC_CLK_TCK);
	startTime = times(&cpuTime);
	if(!(pid = fork())){	// the return value of pid = fork() is 0 in the child, 
		for(i = 0; i < 100000000; ++i);
		exit(0);
	}
	else{
		alarm(2);
		wait(NULL);
	}
	endTime = times(&cpuTime);
	printf("Times Elapsed: %.4f\n", (endTime - startTime)/ticks_per_sec);
	printf("user time: %.4f\n", cpuTime.tms_utime/ticks_per_sec);
	printf("system time: %.4f\n", cpuTime.tms_stime/ticks_per_sec);
	printf("child user time: %.4f\n", cpuTime.tms_cutime/ticks_per_sec);
	printf("child system time: %.4f\n", cpuTime.tms_cstime/ticks_per_sec);
#endif

	return 0;
}
