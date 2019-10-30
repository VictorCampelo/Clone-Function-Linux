#define _GNU_SOURCE
#include <sched.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <locale.h>

// CLONE_FILES - CLONE_VM - CLONE_VFORK
#define MAX_READ 30
#define MSG 45

typedef struct
{
	int fd;
	char text_write[MAX_READ+1];
	char text_read[MAX_READ+1];
	char text_msg[MSG];
} valuesToClone;


int main(int argc, char** argv) {
  // Aloca fila para o filho.
	const int STACK_SIZE = 65536;
	char* stack = malloc(STACK_SIZE);
	ssize_t numRead;

	printf("My process ID : %d\n", getpid());
	
	valuesToClone values;

	values.fd = open ("bank.txt", O_RDWR);
	if (values.fd == -1) {
		perror("Failed to open file\n");
		exit(1);
	}

	if (!stack) {
		perror("Failed to alocatte memory\n");
		exit(1);
	}
	//ftruncate(values.fd, 0);
	char buf[100];
	write(values.fd, (char*)"tudo bem", 8);
	close(values.fd);
	open ("bank.txt", O_RDWR);
	ssize_t nr = read(values.fd, buf, 100);
	printf("-> %s\n", buf);
	close(values.fd);
	return 0;
}
