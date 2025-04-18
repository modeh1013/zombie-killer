// these are the header files that will be used in the project
#include <linux/module.h>
#include <linux/kernel.h>
// the two following ones are new to allow us to create threads and prevent deadlocks
#include <linux/kthread.h>
#include <linux/semaphore.h>
#include <linux/sched.h>
#include <linux/delay.h>


// initialization of the Number of producer threads
static int prod = 1; 
// doing the same thing for the consumer threads as well as the size of the semaphore
static int size = 5; 
static int cons = 1; 

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Laila");
MODULE_DESCRIPTION("Producer-Consumer kernel module that allows us to create and deal with threads using semaphores");

// makes the producer thread a module parameter
module_param(prod, int, 0);
// used this for debugging and checking purposes
MODULE_PARM_DESC(prod, "Number of producer threads that were made:");
// makes the consumer thread a module parameter just like the previous one
module_param(cons, int, 0);
// again used for debugging
MODULE_PARM_DESC(cons, "Number of consumer threads that were made:");
module_param(size, int, 0);

MODULE_PARM_DESC(size, "Size of the semaphore used:");

// this first semaphore will track the filled up slots found in the buffer
static struct semaphore full;
// this second semaphore does the opposite so tracks the empty ones
static struct semaphore empty;

// these two are used as pointer arrays for the thread functions that will be made
static struct task_struct **producers;
static struct task_struct **consumers;

// Producer thread function
static int producer_thread(void *arg) {
// this will get the thread number from the argument
    int id = *(int *)arg;
    // got this line from the internet and it's used as a buffer
    char thread_name[TASK_COMM_LEN] = {};
// used a while loop so the tasks continue working until the thread has to stop
    while (!kthread_should_stop()) {
    // got this from the manual and it acquires the empty semaphore
        if (down_interruptible(&empty)) {
        // if there was an interruption case then it will be released
            break;
        }
// created a print statement to show how many items were in fact produced or in other words threads

printk(KERN_INFO "An item has been produced by Producer-%d\n", id);
// release of the full semaphore
        up(&full);
// this will create simulation of the work by sleeping and then continuing like talked about in lectures
        msleep(100); 
        }

    return 0;
}

// made a consumer thread functions
static int consumer_thread(void *arg) {
// same thing as previously 
    int id = *(int *)arg;
    // internet buffer 
    char thread_name[TASK_COMM_LEN] = {};
// loop until the thread stops, this was used for both consumers and producer threads
    while (!kthread_should_stop()) {
// again will acquire the full semaphore
        if (down_interruptible(&full)) {
        // exit if interrupted
            break;
        }
// seeing how many threads were consumed by the consumers
        printk(KERN_INFO "An item has been consumed by Consumer-%d\n", id);
        up(&empty);
// we will also put it to sleep, the number 100 I got from a website 
      msleep(100); 
    }

    return 0;
}

// initialization of the modules
static int __init producer_consumer_init(void) {
// an index for counting the loop
    int i;
// arrays to store the threads specific id
    int *producer_ids, *consumer_ids;

// Initialize semaphores
    sema_init(&empty, size);
    sema_init(&full, 0);

// Allocating a memory for producer thread task and IDs to be used
    producers = kmalloc_array(prod, sizeof(struct task_struct *), GFP_KERNEL);
// we will also allocate memory for the consumer threads
    consumers = kmalloc_array(cons, sizeof(struct task_struct *), GFP_KERNEL);
// same thing but for the IDs
    producer_ids = kmalloc_array(prod, sizeof(int), GFP_KERNEL);
    consumer_ids = kmalloc_array(cons, sizeof(int), GFP_KERNEL);
// used as a buffer because I struggled with coding to check if allocation failed


// Creating producer threads
    for (i = 0; i < prod; i++) {
// used to assign a unique ID
        producer_ids[i] = i + 1;
// creating and starting a producer thread
        producers[i] = kthread_run(producer_thread, &producer_ids[i], "Producer-%d", producer_ids[i]);
    
    }

// Creating the consumer threads
    for (i = 0; i < cons; i++) {
  // same thing assigning a unique ID
        consumer_ids[i] = i + 1;
        consumers[i] = kthread_run(consumer_thread, &consumer_ids[i], "Consumer-%d", consumer_ids[i]);
        
    }
// a message showing that it has successfully loaded 
    printk(KERN_INFO "Producer-Consumer module loaded\n");
    return 0;
}

// Module cleanup function
static void __exit producer_consumer_exit(void) {
// initializing a variable i
    int i;

// Stops the producer threads
    for (i = 0; i < prod; i++) {
        if (producers[i]) {
// function used same as manual
            kthread_stop(producers[i]);
        }
    }

// Stops consumer threads
    for (i = 0; i < cons; i++) {
        if (consumers[i]) {
            kthread_stop(consumers[i]);
        }
    }

  // the two lines use to free allocated memory
    kfree(producers);
    kfree(consumers);

    printk(KERN_INFO "Producer-Consumer module unloaded\n");
}

module_init(producer_consumer_init);
module_exit(producer_consumer_exit);
	

