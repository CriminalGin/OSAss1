#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
	if(!(strcmp(argv[1], "test"))){
		printf("argv[1] = %s\n", argv[1]);
	}
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
