#include <stdio.h>
#include <unistd.h>

int main(void)
{
	printf("PID %d: calling execl(\"/bin/ls\", \"ls\", NULL)...\n", getpid());
	fflush(stdout);

	execl("/bin/ls", "ls", (char *)NULL);
	perror("execl failed");
	return 1;
}
