#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(){
	printf("My pid is %d\n", getpid());
	signal(SIGINT, SIG_IGN);
	char test[10];
	while(1){
		printf("Before fgets...");
		fgets(test, 10, stdin);
		printf("After fgets: %s", test);
	}
	return 0;
}
