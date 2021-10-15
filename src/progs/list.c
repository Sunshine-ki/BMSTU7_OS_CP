#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>  

#define VALUE_SIZE 64
#define OK 0
#define GET_RAND_NUMBER(min, max) (rand() % (max - min + 1) + min)

typedef struct Node {
    int *value; // VALUE_SIZE
    struct Node *next; // 8 byte
} Node;

Node *create()
{
	Node *node = (Node*)malloc(sizeof(Node));
    node->value = (int*)malloc(VALUE_SIZE * sizeof(int)); // VALUE_SIZE * 4 (при 64 == 256)
	node->next = NULL;
	return node;
}

void add_to_end(Node *node)
{
	Node *new_node = create();
	node->next = new_node;
}

void output_data(int *data)
{
	for (int i = 0; i < VALUE_SIZE; i++)
	{
		printf("%d ", data[i]);
	}
	printf("\n");
}

void output(Node *node)
{
	while(node != NULL)
	{
		output_data(node->value);
		node = node->next;
	}
	printf("\n\n");
}

void generate_random_values(Node *node)
{
	for (int i = 0; i < VALUE_SIZE; i++)
	{
		node->value[i] = GET_RAND_NUMBER(0, 100);
	}
}

void update_list(Node *node)
{
	while(node != NULL)
	{	
		generate_random_values(node);
		node = node->next;
	}
}

int msleep(long msec)
{
    struct timespec ts;
    int res;

    if (msec < 0)
    {
        errno = EINVAL;
        return -1;
    }

    ts.tv_sec = msec / 1000;
    ts.tv_nsec = (msec % 1000) * 1000000;

    do {
        res = nanosleep(&ts, &ts);
    } while (res && errno == EINTR);

    return res;
}

void process()
{
	Node *first = create();
	Node *current_node = first;


	// int count = 1155072;
	// char *buffer = (char*) malloc(count);
	// char *buffer2 = (char*) malloc(count);

	// Сделать 3 потока и посмотреть сколько будет выделяться памяти
	// На каждый поток свой список 

	long long int i = 0;
	while (1)
	{
		update_list(first);
		// output(first);

		add_to_end(current_node);
		
		current_node = current_node->next;
		
		i++;
		if (!(i % 16))
		{
			printf("1 kilobytes\n");
		}

		long long int byte = (256 + 8) * i;
		printf("byte = %lld kilobyte = %lld ", byte, byte / 1024);
		printf("pages = %lld \n", byte / 1024 / 4);
	 	msleep(100);
		// sleep(1);
	}
}

int main()
{
	srand(time(NULL));
	setbuf(stdout, NULL);
	process();
	return OK;
}