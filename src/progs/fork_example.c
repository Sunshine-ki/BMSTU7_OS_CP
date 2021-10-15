#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>

#define GET_RAND_NUMBER(min, max) (rand() % (max - min + 1) + min)

int main(void)
{
	srand(time(NULL));
	setbuf(stdout, NULL);

	int status;
	pid_t pid = fork();

	int common_variable = 123;
	
	if (pid == -1 )
	{
		perror("fork");
		return -1;
	}
	else if(!pid) // если это потомок
	{
		printf("Child start\n");
		while (1)
		{
			printf("Child say: %d\n", common_variable);

			// int a = GET_RAND_NUMBER(0, 100);
		}
	}
	else // если это родитель
	{
		printf("Parent start\n");
		while (1)
		{
			printf("Parent say: %d\n", common_variable);
			// int b = GET_RAND_NUMBER(0, 100);
		}
	}

	return 0;
}


// switch(pid=fork())
// {
// 	case -1:
// 			perror("fork"); /* произошла ошибка */
// 			exit(1); /*выход из родительского процесса*/
// 	case 0:
// 			printf(" CHILD: Это процесс-потомок!\n");
// 			printf(" CHILD: Мой PID -- %d\n", getpid());
// 			printf(" CHILD: PID моего родителя -- %d\n", getppid());
// 			printf(" CHILD: Введите мой код возврата:");
// 			scanf(" %d", &rv);
// 			printf(" CHILD: Выход!\n");
// 			exit(rv);
// 	default:
// 			printf("PARENT: Это процесс-родитель!\n");
// 			printf("PARENT: Мой PID -- %d\n", getpid());
// 			printf("PARENT: PID моего потомка %d\n", pid);
// 			printf("PARENT: Я жду, пока потомок вызовет exit()...\n");
// 			wait(&status);
// 			printf("PARENT: Код возврата потомка:%d\n", WEXITSTATUS(status));
// 			printf("PARENT: Выход!\n");
// }