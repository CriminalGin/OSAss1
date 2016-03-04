#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <glob.h>
#include <stdlib.h>

int main(int argc, char *argv[]){
	char test[] = "*.c";
	int i;
	for(i = 0; i < strlen(test); ++i){
		if(test[i] == '*'){printf("I have found *\n");	return 0;}
	}
	printf("I have not found *\n");
	return -1;
}

#if 0
int cmpfunc(const void *a, const void *b){
	printf("the result of %s and %s is %d\n", (char*)a, (char *)b, (*(char *)a - *(char *)b) );
	return ( *(char *)a - *(char *)b );
}

int main(int argc, char *argv[]){
	char test[] = "Hello world";
	char key = 'e';
	printf("key is %c\n", key);
	char *item = (char *)bsearch(&key, test, strlen(test), sizeof(char), cmpfunc);
	if(item != NULL){printf("I have found the item\n");}
	else{printf("I have not found the item\n");}
	return 0;
}
#endif
