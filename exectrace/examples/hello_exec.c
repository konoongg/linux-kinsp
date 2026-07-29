#include <stdio.h>
#include <unistd.h>

int main(void)
{
	printf("PID %d: press any key to continue...\n", getpid());
	fflush(stdout);

	getchar();

	printf("PID %d: calling execl(\"/proc/self/exe\", ...)...\n", getpid());
	fflush(stdout);

	execl("/proc/self/exe", "hello_exec", (char *)NULL);
	perror("execl failed");
	return 1;
}
