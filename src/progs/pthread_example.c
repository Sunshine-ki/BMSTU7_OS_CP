// Compiling with key -pthread
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>  
#include <pthread.h>

#define SUCCESS 0

#define ERROR_CREATE_THREAD -11
#define ERROR_JOIN_THREAD   -12

#define VALUE_SIZE 64
#define PTHREAD_COUNT 3
#define SLEEP_TIME 3

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

void process(char* name)
{
	Node *first = create();
	Node *current_node = first;

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
		printf(" %s byte = %lld kilobyte = %lld ",name, byte, byte / 1024);
		printf("pages = %lld \n", byte / 1024 / 4);
	 	msleep(SLEEP_TIME);
	}
}
 
void* do_pthread(void *args) 
{
	process((char*)args);
	return SUCCESS;
}

// gcc pthread_example.c -pthread -o my_mem_prog
int main() 
{
	printf("Start program");
    srand(time(NULL));
	setbuf(stdout, NULL);
	
	pthread_t threads[PTHREAD_COUNT];
	char* names[5] = {"1", "2", "3", "4", "5"};

    int status;
    int status_addr;

	for (int i = 0; i < PTHREAD_COUNT; i++)
	{
		status = pthread_create(&threads[i], NULL, do_pthread, names[i]);
		if (status != 0) {
			printf("main error: can't create thread, status = %d\n", status);
			exit(ERROR_CREATE_THREAD);
		}
	}

	do_pthread("Main pthread"); 

 	for (int i = 0; i < PTHREAD_COUNT; i++)
	{
		status = pthread_join(threads[i], (void**)&status_addr);
		if (status != SUCCESS) {
			printf("main error: can't join thread, status = %d\n", status);
			exit(ERROR_JOIN_THREAD);
		}
	    printf("joined with address %d\n", status_addr);
	}
 
    return 0;
}