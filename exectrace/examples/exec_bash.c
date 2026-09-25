#include <stdio.h>
#include <unistd.h>

int main(void)
{
	printf("PID %d: press any key to continue...\n", getpid());
	fflush(stdout);

	getchar();

	printf("PID %d: calling execl(\"./exec_bash.sh\", ...)...\n", getpid());
	fflush(stdout);

	execl("./exec_bash.sh", "exec_bash.sh", (char *)NULL);
	perror("execl failed");
	return 1;
}