#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define PAGE_SIZE 4 * 1024 // 4 kilobyte
#define OK 0

void create(int page_count)
{
	void* mem = (void*)malloc(PAGE_SIZE * page_count);
	printf("malloc (%d pages)\n", page_count);
}

void process(void)
{
	int page_count = 1;
	while (page_count)
	{
		create(page_count);
		printf("Input page count (0 to exit) : ");
		scanf("%d", &page_count);
	}
}

int main()
{
	srand(time(NULL));
	setbuf(stdout, NULL);
	process();
	return OK;
}