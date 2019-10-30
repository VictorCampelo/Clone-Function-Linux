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
#include <errno.h>
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

static int child(void* arg) {
	valuesToClone* v1 = (valuesToClone*)arg;
	ftruncate(v1->fd, 0);
	strcpy(v1->text_msg, "Filho: Obrigado por me emprestar o dinheiro");
	strcpy(v1->text_write, "saldo: 0.0");

	write(v1->fd, v1->text_write, strlen(v1->text_write));

	/* posicionando-se no inicio do arquivo... */
	if (lseek (v1->fd, 0, SEEK_SET) < 0)
	{
		perror ("Erro de posicionamento");
		exit (errno);
	}

	read(v1->fd, v1->text_read, strlen(v1->text_write));

	close(v1->fd);
	_exit(0);
}

int main(int argc, char** argv) {
  // Aloca fila para o filho.
	const int STACK_SIZE = 65536;
	char* stack = malloc(STACK_SIZE);
	ssize_t numRead;

	printf("My process ID : %d\n", getpid());
	
	valuesToClone values;

	values.fd = open ("bank.txt", O_RDWR | O_CREAT | O_SYNC | O_TRUNC | O_CLOEXEC);
	if (values.fd == -1) {
		perror("Failed to open file\n");
		exit(1);
	}

	if (!stack) {
		perror("Failed to alocatte memory\n");
		exit(1);
	}

	int status = 0;
	unsigned long flags = 0;
	if (argc > 1 && !strcmp(argv[1], "y")) {
		flags |= CLONE_VM;
		flags |= CLONE_FILES;
		flags |= CLONE_VFORK;
		status = 1;
	}

	strcpy(values.text_msg, "Pai: Nao vou emprestar dinheiro");
	strcpy(values.text_write, "saldo: 100.0");

	if (clone(child, stack + STACK_SIZE, flags | SIGCHLD, (void*)(&values)) == -1) {
		perror("clone");
		exit(1);
	}
	
	if (status == 0)
	{
		printf("Flag CLONE_VFORK não ativada. Precisa do Wait()\n");
		if (wait(&status) == -1) {
			perror("wait");
			exit(1);
		}
	}
	else{
		printf("Flag CLONE_VFORK ativada, Pai está suspenso ate filho terminar\n");
	}	

	char aux[MAX_READ];
	numRead = read(values.fd, aux, MAX_READ);
	if (numRead == -1) {
		perror("Read Failed");
		printf("%s\n", values.text_msg);
		printf("-> %s\n", values.text_read);
		exit(1);
	}
	
	if (lseek (values.fd, 0, SEEK_SET) < 0)
	{
		perror (argv[0]);
		exit (errno);
	}

	write (values.fd, values.text_write, strlen(values.text_write));

	if (lseek (values.fd, 0, SEEK_SET) < 0)
	{
		perror (argv[0]);
		exit (errno);
	}

	read(values.fd, values.text_read, strlen(values.text_write));

	printf("%s\n", values.text_msg);
	printf("-> %s\n", (char*)values.text_read);

	close(values.fd);
	return 0;
}
