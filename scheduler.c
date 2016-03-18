#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/times.h>	// Needed by times(), clock_t
#include <string.h>
#include <glob.h>
#include <errno.h>

#define	MAXNUMOFJOB	10
#define MAXLENOFJOB	10000
#define MAXLENOFCOMMAND 255
#define CHARTOINT	48
pid_t pid;
pid_t pids[MAXNUMOFJOB][2];

int FindMonitor(pid_t _pid, pid_t **_pids){
	int len = MAXNUMOFJOB;
	int width = 2;
	int i, j;
	for(i = 0; i < len; ++i){
		for(j = 0; j < width; ++j){
			if(_pids[i][j] == _pid)			
				{printf("The monitor position is %d\n", i);return i;}		
		}	
	}
	return -1;
}

void alarmHandlerFIFO(int signal){
	kill(pid, SIGTERM);
}

void alarmHandlerPARA(int signal){
	pid_t myPid = getpid();
	int i = FindMonitor(myPid, pids);
	printf("The monitor position is %d\n", i);
	kill(pids[i][1], SIGTERM);
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

int Seperate(int num, char **jobToken, char **cmdToken, int *times)
{
	int i;
	for(i = 0; i < (num - 2); ++i){
		cmdToken[i] = (char *)malloc(sizeof(char) * strlen(jobToken[i]));
		while(strcpy(cmdToken[i], jobToken[i]) == NULL);
	}
	*times = atoi(jobToken[num - 2]);
	return 0;
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
	char ***cmdToken = (char ***)malloc(num * sizeof(char**));
	int *numOfToken = (int *)malloc(num * sizeof(int));
	int *time = (int *)malloc(num * sizeof(int));
	for(i = 0; i < num; ++i){
		jobToken[i] = (char **)malloc( (MAXLENOFCOMMAND + 2) * sizeof(char *));
		cmdToken[i] = (char **)malloc( (MAXLENOFCOMMAND + 1) * sizeof(char *));
		numOfToken[i] = TokenInput(job[i], " \t", jobToken[i]);
		while(Seperate(numOfToken[i], jobToken[i], cmdToken[i], (time + i)) != 0);
}
#if 1
	// printf("num is %d\n", num);
	clock_t startTime, endTime;
	struct tms cpuTime;
	double ticks_per_sec = (double)sysconf(_SC_CLK_TCK);
	if(!strcmp(mode, "FIFO")){
		signal(SIGALRM, alarmHandlerFIFO);
		for(i = 0; i < num; ++i){
		
		if(!(pid = fork())){
			setenv("PATH", ":/bin:/usr/bin:.", 1);
			execvp(cmdToken[i][0], cmdToken[i]);
			exit(0);
		}
		else{
			alarm(time[i]);
			waitpid(pid, NULL, 0);	
		}
		
		
	}
}

	else if(!strcmp(mode, "PARA")){
		clock_t startTimes[10], endTimes[10];
		struct tms cpuTimes[10];
		printf("I have gone to the PARA\n");
		for(i = 0; i < num; ++i){
			
			signal(SIGALRM, alarmHandlerPARA);
			if(!(pids[i][0] = fork())){
				
				pids[i][0] = getpid();
				startTimes[i] = times(&cpuTimes[i]);
				if(!(pids[i][1] = fork())){
					pids[i][1] = getpid();
					
					printf("<<Process %d>>\n", getpid());
					setenv("PATH", ":/bin:/usr/bin:.", 1);
					
					execvp(cmdToken[i][0], cmdToken[i]);								
				}	
				else{
					alarm(time[i]);
					int k = FindMonitor(getpid(), pids);
					printf("The monitor position is %d\n", k);
					// waitpid(pids[i][1], NULL, 1);
					wait(NULL);	
				}
				endTimes[i] = times(&cpuTimes[i]);
				printf("time elapsed: %.4f\n", (endTimes[i] - startTimes[i])/ticks_per_sec);
				printf("user time: %.4f\n", cpuTimes[i].tms_utime/ticks_per_sec);
				printf("system time: %.4f\n", cpuTimes[i].tms_stime/ticks_per_sec);
			}
			
		}
		for(j = 0; j < num; ++j){
			waitpid(pids[j][0], NULL, 0);
		}
	}
#endif

#if 0
	signal(SIGALRM, alarmHandlerFIFO);
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
	// printf("child user time: %.4f\n", cpuTime.tms_cutime/ticks_per_sec);
	// printf("child system time: %.4f\n", cpuTime.tms_cstime/ticks_per_sec);
#endif

	return 0;

}
