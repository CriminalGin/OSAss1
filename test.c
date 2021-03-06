#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/times.h>
#include <unistd.h>
#include <wait.h>

pid_t pid;
void alarmHandlerFIFO(int signal){
	kill(pid, SIGTERM);
}

int main(int argc, char *argv[])
{
	clock_t startTime, endTime;
	struct tms cpuTime;
	double ticks_per_sec = (double)sysconf(_SC_CLK_TCK);
	signal(SIGALRM, alarmHandlerFIFO);
	startTime = times(&cpuTime);
	int i;
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
}


#if 0
typedef struct stu{
	int age;
	char name[10];
};

int main()
{
	extern stu *stu1;
	stu1->age = 1;
	printf("%d", stu1->age);
	return 0;
}
#endif

#if 0
int main(){
	FILE *fp = fopen("filename.txt", "r");
	char *line = NULL;
	size_t len = 0;
	ssize_t read;

	if(fp == NULL){ exit(EXIT_FAILURE); }
	while((read = fgets(&line, &len, fp)) != -1)
	{
		printf("Retrieved line of length %zu :\n", read);
		printf("%s", line);
	}

	free(line);
	fclose(fp);
	exit(EXIT_SUCCESS);
	return 0;
}
#endif
