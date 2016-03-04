#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <glob.h>


int main(int argc, char *argv[]){
	glob_t globbuf;

	globbuf.gl_offs = 1;

	glob("*.c", GLOB_DOOFFS | GLOB_NOCHECK, NULL, &globbuf);
	globbuf.gl_pathv[0] = "ls";

	execvp(globbuf.gl_pathv[0], globbuf.gl_pathv);
	return 0;
}
