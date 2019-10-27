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

// CLONE_FILES - CLONE_VM - CLONE_VFORK
typedef struct
{
	char text_str[500];
	int fd;
} valuesToClone;

static int child_func(void* arg) {
	valuesToClone* v1 = (valuesToClone*)arg;
	printf("%d\n", v1->fd);
	read(v1->fd, v1->text_str, 500);
	printf("Pegou os dados =  %s\n", v1->text_str);
	write (v1->fd, "dados roubados", 500);
	read(v1->fd, v1->text_str, 500);
	close(v1->fd);
	return 0;
}

int main(int argc, char** argv) {
  // Aloca fila para o filho.
	const int STACK_SIZE = 65536;
	char* stack = malloc(STACK_SIZE);
	
	valuesToClone values;

	values.fd = open ("fathersPassword.txt", O_RDWR);
	if (values.fd == -1) {
		perror("Failed to open file\n");
		exit(1);
	}

	if (!stack) {
		perror("Failed to alocatte memory\n");
		exit(1);
	}

  // Chamado quando usado o argumento "vm", altera CLONE_VM para on.
	unsigned long flags = 0;
	if (argc > 1 && !strcmp(argv[1], "senha123")) {
		flags |= CLONE_VM;
		flags |= CLONE_FILES;
	}

	strcpy(values.text_str, "Dados estão seguros");
	if (clone(child_func, stack + STACK_SIZE, flags | SIGCHLD, (void*)(&values)) == -1) {
		perror("clone");
		exit(1);
	}

	int status;
	if (wait(&status) == -1) {
		perror("wait");
		exit(1);
	}
	char aux[500];
	status = read(values.fd, aux, 500);
	printf("%s\n", aux);
	if (status < 0) {
		perror("Read Failed\n");
		printf("hacker alterou seus dados: \"%s\"\n e fechou arquivo", values.text_str);
		exit(1);
	}
	close(values.fd);
	printf("\n\ntext_str = \"%s\"\n", values.text_str);
	return 0;
}
