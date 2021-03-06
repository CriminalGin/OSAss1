#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/times.h>	// Needed by times(), clock_t
#include <string.h>
#include <glob.h>

#define	MAXNUMOFJOB	10
#define MAXLENOFJOB	10000
#define MAXLENOFCOMMAND 255
#define CHARTOINT	48
pid_t pid;
pid_t pids[MAXNUMOFJOB][2];

#if 0
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
#endif

void alarmHandlerFIFO(int signal){
	kill(pid, SIGTERM);
}

void alarmHandlerPARA(int signal){
	pid_t myPid = getpid();
	int i;
	for(i = 0; i < MAXNUMOFJOB; ++i){
		if(pids[i][0] == myPid){
			//int i = FindMonitor(myPid, pids);
			//printf("The monitor position is %d\n", i);
			kill(pids[i][1], SIGTERM);
			break;
		}
	}
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

int main(int argc, char *argv[]){
	int i, j;
	char *mode = argv[1];
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
				int *starPosition = FindElement('*',cmdToken[i]);				
				if(starPosition == NULL){				
					execvp(cmdToken[i][0], cmdToken[i]);
					exit(0);}
				else{
					int tokenNum = Count2d(cmdToken[i]), starNum = starPosition[0];
					glob_t globbuf;
					globbuf.gl_offs = tokenNum - starNum;
					glob(cmdToken[i][starPosition[1]], GLOB_DOOFFS | GLOB_NOCHECK, NULL, &globbuf);
					for(j = 2; j < (starNum - 1 + 2); ++j){
						glob(cmdToken[i][starPosition[j]],GLOB_DOOFFS | GLOB_NOCHECK | GLOB_APPEND, NULL, &globbuf);
					}
					for(j = 0; j < globbuf.gl_offs; ++j)			{
						globbuf.gl_pathv[j] = cmdToken[i][j];
					}
					execvp(globbuf.gl_pathv[0], globbuf.gl_pathv);
					exit(0);
				}
			}
			else{
				alarm(time[i]);
				waitpid(pid, NULL, 0);	
			}	
	}
}

	else if(!strcmp(mode, "PARA")){
		clock_t startTimes[10], endTimes[10];
		glob_t globbufs[MAXNUMOFJOB];
		struct tms cpuTimes[10];
		printf("I have gone to the PARA\n");
		for(i = 0; i < num; ++i){			
			if(!(pids[i][0] = fork())){
				signal(SIGALRM, alarmHandlerPARA);
				pids[i][0] = getpid();
				startTime = times(&cpuTime);
	
				if(!(pids[i][1] = fork())){
					
					pids[i][1] = getpid();
					printf("I am the job, my pid is %d, my parent pid is %d\n", getpid(), getppid());
					//printf("<<Process %d>>\n", getpid());
					setenv("PATH", ":/bin:/usr/bin:.", 1);
					int *starPosition = FindElement('*',cmdToken[i]);				
					if(starPosition == NULL){				
						execvp(cmdToken[i][0], cmdToken[i]);
						exit(0);}
					else{
						int tokenNum = Count2d(cmdToken[i]), starNum = starPosition[0];
						globbufs[i].gl_offs = tokenNum - starNum;
						glob(cmdToken[i][starPosition[1]], GLOB_DOOFFS | GLOB_NOCHECK, NULL, &globbufs[i]);
						for(j = 2; j < (starNum - 1 + 2); ++j){
							glob(cmdToken[i][starPosition[j]],GLOB_DOOFFS | GLOB_NOCHECK | GLOB_APPEND, NULL, &globbufs[i]);
						}
						for(j = 0; j < globbufs[i].gl_offs; ++j)			{
							globbufs[i].gl_pathv[j] = cmdToken[i][j];
						}
						execvp(globbufs[i].gl_pathv[0], globbufs[i].gl_pathv);
						
						exit(0);
					}							
								
				}	
				else{
					printf("I am the monitor, my pid is %d\n", getpid());
					// int k = FindMonitor(getpid(), pids);
					// printf("The monitor position is %d\n", k);
					alarm(time[i]);
					// waitpid(pids[i][1], NULL, 1);
					wait(NULL);
					endTime = times(&cpuTime);
					printf("time elapsed: %.4f\n", (endTime - startTime)/ticks_per_sec);
					printf("user time: %.4f\n", cpuTime.tms_utime/ticks_per_sec);
					printf("system time: %.4f\n", cpuTime.tms_stime/ticks_per_sec);	
				}
			}
			
			else{
				for(i = 0; i < num; ++i){
					waitpid(pids[i][0], NULL, 0);
				}
			}
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
