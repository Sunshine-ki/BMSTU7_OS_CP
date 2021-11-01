#include <linux/module.h> // MODULE_LICENSE, MODULE_AUTHOR
#include <linux/kernel.h> // KERN_INFO
#include <linux/init.h>	  // ​Макросы __init и ​__exit
#include <linux/init_task.h> // init_task
#include <linux/interrupt.h> // request_irq
#include <linux/string.h> // strcmp
#include <asm/page.h>
#include <linux/workqueue.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alice");
MODULE_DESCRIPTION("Coursework BMSTU");

#define KBD_DATA_REG 0x60 /* I/O port for keyboard data */
#define KBD_SCANCODE_MASK 0x7f
#define KBD_STATUS_MASK 0x80

#define ANALYSIS_PROGRAM_NAME "my_mem_prog"

#define MONITORING_SCANCODE 1 // "[ESC]"

unsigned int my_irq = 1; // Прерывание от клавиатуры.

static struct workqueue_struct *my_wq; //очередь работ
struct task_struct *task = NULL;
char current_scancode;

bool find_user_task_struct(char* prog_name);
static void my_wq_function(struct work_struct *work);
irqreturn_t irq_handler(int irq, void *dev, struct pt_regs *regs);

// Статическая инициализация структуры.
// Создает (работу), переменную my_work, с типом struct work_struct *
DECLARE_WORK(my_work, my_wq_function);

void info_about_mm(void)
{
	struct mm_struct *info_about_mem; 
	atomic_t mm_users; 
	int counter;

	if (find_user_task_struct(ANALYSIS_PROGRAM_NAME) == false)
	{
		printk(KERN_INFO "+Module: find_user_task_struct is false");
		return;
	}

	// if (task == NULL || task->mm == NULL)
	// {
	// 	printk(KERN_INFO "+Module: task or task->mm is NULL");
	// 	if (find_user_task_struct(ANALYSIS_PROGRAM_NAME) == false)
	// 	{
	// 		printk(KERN_INFO "+Module: find_user_task_struct is false");
	// 		return;
	// 	}
	// }

	info_about_mem = task->mm;
	
	// В поле mm_users хранится количество процессов, в которых используется данное адресное пространство.
	mm_users = info_about_mem->mm_users; /* Счетчик использования адресного пространства */
	counter = mm_users.counter;

	printk(KERN_INFO "+Module: mm_users: %d", counter);
	
	printk(KERN_INFO "+Module: start_code = %lu end_code = %lu", info_about_mem->start_code, info_about_mem->end_code);
	printk(KERN_INFO "+Module: end_code - start_code = %lu ", info_about_mem->end_code - info_about_mem->start_code );
	printk(KERN_INFO "+Module: task_size = %lu highest_vm_end = %lu", info_about_mem->task_size, info_about_mem->highest_vm_end);
	printk(KERN_INFO "+number of VMAs = %d ", info_about_mem->map_count);
	printk(KERN_INFO "+Total pages mapped (всего отображенных страниц) = %lu ", info_about_mem->total_vm);

	printk(KERN_INFO "+start_brk = %lu ", info_about_mem->start_brk);
	printk(KERN_INFO "+brk = %lu ", info_about_mem->brk);
	printk(KERN_INFO "+brk - start_brk = %lu ", info_about_mem->brk - info_about_mem->start_brk);
}


static void my_wq_function(struct work_struct *work) // вызываемая функция
{
	int scan_normal;

	// printk("Scan Code %d %s\n",
	// 	current_scancode & KBD_SCANCODE_MASK,
	// 	current_scancode & KBD_STATUS_MASK ? "Released" : "Pressed");

	if (!(current_scancode & KBD_STATUS_MASK))
	{
		scan_normal = current_scancode & KBD_SCANCODE_MASK;

		if (scan_normal == MONITORING_SCANCODE)
		{
			printk("This is esc!");
			info_about_mm();
		}
	}
}

irqreturn_t irq_handler(int irq, void *dev, struct pt_regs *regs)
{
	if (irq == my_irq)
	{		
		// Получаем скан-код нажатой клавиши.
		current_scancode = inb(KBD_DATA_REG);

		// Помещаем структуру в очередь работ.
		// queue_work назначает работу текущему процессору.
		queue_work(my_wq, &my_work);

		return IRQ_HANDLED; // прерывание обработано
	}
	else
		return IRQ_NONE; // прерывание не обработано
}

bool find_user_task_struct(char* prog_name)
{
	struct task_struct *current_task = &init_task;

	do {
		if (!strcmp(current_task->comm, prog_name))
		{
			task = current_task;
			return true;
		}
	} while ((current_task = next_task(current_task)) != &init_task);

	return false;
}

static int __init md_init(void)
{
	// регистрация обработчика прерывания
	if (request_irq(
			my_irq,						/* номер irq */
			(irq_handler_t)irq_handler, /* наш обработчик */
			IRQF_SHARED,				/* линия может быть раздедена, IRQ
										(разрешено совместное использование)*/
			"my_irq_handler",			/* имя устройства (можно потом посмотреть в /proc/interrupts)*/
			(void *)(irq_handler)))		/* Последний параметр (идентификатор устройства) irq_handler нужен
										для того, чтобы можно отключить с помощью free_irq  */
	{
		printk(" + Error request_irq");
		return -1;
	}

	my_wq = create_workqueue("my_queue"); //создание очереди работ
	if (my_wq)
	{
		printk(KERN_INFO "Module: workqueue created!\n");
	}
	else
	{
		free_irq(my_irq, irq_handler); // Отключение обработчика прерывания.
		printk(KERN_INFO "Module: error create_workqueue()!\n");
		return -1;
	}

	find_user_task_struct(ANALYSIS_PROGRAM_NAME);

	printk(KERN_INFO "Module: module loaded!\n");
	return 0;
}

static void __exit md_exit(void)
{
	// Принудительно завершаем все работы в очереди.
	// Вызывающий блок блокируется до тех пор, пока операция не будет завершена.
	flush_workqueue(my_wq);
	destroy_workqueue(my_wq);

	// my_irq - номер прерывания.
	// irq_handler - идентификатор устройства.
	free_irq(my_irq, irq_handler); // Отключение обработчика прерывания.
	
	printk(KERN_INFO "Module: unloaded!\n");
}

module_init(md_init);
module_exit(md_exit);




// struct task_struct *task = &init_task;
// struct mm_struct *info_about_mem; 

// // list_of_VMAs
// // struct vm_area_struct *text, *data_prog, *bss, *heap, *memory_mapping, *stack;
// // struct file* current_file;

// printk(KERN_INFO "Module begin info");

// do {
// 	if (!strcmp(task->comm, "my_mem_prog"))
// 	{
// 		info_about_mem = task->mm;
// 		if (info_about_mem == NULL)
// 		{
// 			continue;
// 		}

// 		// printk(KERN_INFO "+Module: start_code = %lu end_code = %lu", info_about_mem->start_code, info_about_mem->end_code);
// 		// printk(KERN_INFO "+Module: end_code - start_code = %lu ", info_about_mem->end_code - info_about_mem->start_code );
// 		// printk(KERN_INFO "+Module: task_size = %lu highest_vm_end = %lu", info_about_mem->task_size, info_about_mem->highest_vm_end);
// 		printk(KERN_INFO "+number of VMAs = %d ", info_about_mem->map_count);
// 		printk(KERN_INFO "+Total pages mapped (всего отображенных страниц) = %lu ", info_about_mem->total_vm);

// 		printk(KERN_INFO "+start_brk = %lu ", info_about_mem->start_brk);
// 		printk(KERN_INFO "+brk = %lu ", info_about_mem->brk);
// 		printk(KERN_INFO "+brk - start_brk = %lu ", info_about_mem->brk - info_about_mem->start_brk);


// 		// text = info_about_mem->mmap;

// 		// if (text == NULL)
// 		// {
// 		// 	continue;
// 		// }

// 		// // printk(KERN_INFO "+Module: text vm_start = %lu", text->vm_start);
// 		// // printk(KERN_INFO "+Module: text vm_end = %lu", text->vm_end);
// 		// printk(KERN_INFO "+Module: text vm_end - vm_start = %lu byte (%lu kilobytes)", text->vm_end - text->vm_start, (text->vm_end - text->vm_start) / 1024);
// 		// printk(KERN_INFO "+Module: vm_flags %lu byte", text->vm_flags);
		
		
// 		// // current_file = vm_file
// 		// // if (current_file != NULL)
// 		// // {
			
// 		// // }

// 		// data_prog = text->vm_next;

// 		// if (data_prog == NULL)
// 		// {
// 		// 	printk("+Module: data is NULL");
// 		// 	continue;
// 		// }


// 		// // printk(KERN_INFO "+Module: data vm_start = %lu", data_prog->vm_start);
// 		// // printk(KERN_INFO "+Module: data vm_end = %lu", data_prog->vm_end);
// 		// printk(KERN_INFO "+Module: data vm_end - vm_start = %lu byte (%lu kilobytes)", data_prog->vm_end - data_prog->vm_start, (data_prog->vm_end - data_prog->vm_start) / 1024);

// 		// bss = data_prog->vm_next;

// 		// if (bss == NULL)
// 		// {
// 		// 	printk("+Module: bss is NULL");
// 		// 	continue;
// 		// }

// 		// // printk(KERN_INFO "+Module: bss vm_start = %lu", bss->vm_start);
// 		// // printk(KERN_INFO "+Module: bss vm_end = %lu", bss->vm_end);
// 		// printk(KERN_INFO "+Module: bss vm_end - vm_start = %lu byte (%lu kilobytes)", bss->vm_end - bss->vm_start, (bss->vm_end - bss->vm_start) / 1024);


// 		// heap = bss->vm_next;

// 		// if (heap == NULL)
// 		// {
// 		// 	printk("+Module: heap is NULL");
// 		// 	continue;
// 		// }

// 		// // printk(KERN_INFO "+Module: heap vm_start = %lu", heap->vm_start);
// 		// // printk(KERN_INFO "+Module: heap vm_end = %lu", heap->vm_end);
// 		// printk(KERN_INFO "+Module: heap vm_end - vm_start = %lu byte (%lu kilobytes)", heap->vm_end - heap->vm_start, (heap->vm_end - heap->vm_start) / 1024);

// 		// memory_mapping = heap->vm_next;

// 		// if (memory_mapping == NULL)
// 		// {
// 		// 	printk("+Module: heap is NULL");
// 		// 	continue;
// 		// }

// 		// // printk(KERN_INFO "+Module: memory_mapping vm_start = %lu", memory_mapping->vm_start);
// 		// // printk(KERN_INFO "+Module: memory_mapping vm_end = %lu", memory_mapping->vm_end);
// 		// printk(KERN_INFO "+Module: memory_mapping vm_end - vm_start = %lu byte (%lu kilobytes)", memory_mapping->vm_end - memory_mapping->vm_start, (memory_mapping->vm_end - memory_mapping->vm_start) / 1024);
		
// 		// stack = memory_mapping->vm_next;

// 		// if (stack == NULL)
// 		// {
// 		// 	printk("+Module: heap is NULL");
// 		// 	continue;
// 		// }

// 		// // printk(KERN_INFO "+Module: stack vm_start = %lu", stack->vm_start);
// 		// // printk(KERN_INFO "+Module: stack vm_end = %lu", stack->vm_end);
// 		// printk(KERN_INFO "+Module: stack vm_end - vm_start = %lu byte (%lu kilobytes)", stack->vm_end - stack->vm_start, (stack->vm_end - stack->vm_start) / 1024);

// 		// printk(KERN_INFO "+ pid = %d", task->pid);
// 	}
// } while ((task = next_task(task)) != &init_task);

// printk(KERN_INFO "Module end info!\n");
