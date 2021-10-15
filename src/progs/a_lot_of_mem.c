#include <stdio.h>
#include <stdlib.h>

int static_uninitialized[1024 * 1024]; // 
int static_initialized[1024 * 1024] = { 0 }; // 


int main()
{
	int uninitialized[1024 * 1024]; // 

	int a;
	printf("Ввод (после будет malloc): ");
	scanf("%d", &a);

	int count = 1024 * 1024 * 1024;
	char *buffer = (char*) malloc(count);
	char *buffer2 = (char*) malloc(count);

	printf("Ввод (после будет обращение к памяти): ");
	scanf("%d", &a);

	// while (1)
	// {
		for (int i = 0; i <  1024; i++)
		{
			buffer[i] = 'a';
			printf("i = %d buffer[i] = %c\n", i, buffer[i]);
		}
		printf("Буфер заполнился. ");
		for (int i = 0; i <  1024; i++)
		{
			buffer2[i] = 'a';
			printf("i = %d buffer[i] = %c\n", i, buffer[i]);
		}
		printf("Буфер заполнился. ");
	// }

	printf("Ввод (после будет free): ");
	scanf("%d", &a);
	free(buffer);
	free(buffer2);

	printf("Ввод (после будет завершение программы): ");
	scanf("%d", &a);

		

	return 0;
}