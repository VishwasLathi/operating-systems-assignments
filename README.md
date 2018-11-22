# operating-systems-assignemts
Assignment of CS330- Operating systems

Build To Learn (BTL) (2018/2019 - I)

We will gradually build an Operating System (gemOS) during the course. This OS under construction is for 64-bit X86 architecture which will be simulated by Gem5 architectural simulator. The model we follow for the course will be as follows,
Instructor will build a part of the OS, provide the binary for the same. You can download the OS binary and boot it using gem5.
In assignments, one OS feature or functionality will be implemented by you.
The process will repeat.
Gem5: A small HOWTO document regarding Gem5 setup, boot gemOS using Gem5 etc. can be found here. Please go through the document, create your own setup and test run as soon as possible. 

Assignment-1 
At this stage, gemOS is in 64-bit mode executing itself as the first context (say the boot context). The boot context sets up the page table, stack, segement regsiters for itself. Further, it implements basic input output to a serial console. The boot context puts the OS into a basic shell. In this assignment, you need to create another context and execute a function before returning back to the basic shell. You need to setup the paging structures for the new context as per its virtual address layout. Details of the assignment and required files are uploaded in moodle. 

You need to submit only one file (named context_[rollno].c) containing implementations for the following functions.
                                                   void prepare_context_mm (struct exec_context *ctx)
                                                    {
                                                           /*Your code goes in here*/
                                                           .....
                                                           .....

                                                           return;
                                                    }

                                                    void cleanup_context_mm(struct exec_context *ctx)
                                                    {
                                                           /*Your code goes in here*/
                                                           .....
                                                           .....
                                                            
                                                           return;
                                                    }

                                           
Assignment and related files can be found here.

Group information: Individual 
Submission deadline: Sunday, 26 August 2018, 11:59 PM 

Assignment-2 
At this stage, GemOS implements a command called init which creates the first user process (named as the init process with PID = 1). Source code for init process can be found in user/init.c file. The current init process invokes two system calls---getpid() and exit(). The objective of the assignment is to implement new system calls and exception handlers and enable support for lazy memory allocation. To handle the system calls, GemOS installs an IDT entry at offset 0x80. The system call handlers in in GemOS are defined in entry.c. 

As part of the assignment, you are required to implement three system calls listed below. 
int write(char *buf, int length) 
void *expand(u32 size, int flags) 
void *shrink (u32 size, flags) 

In the next part of the assignment, you are required to implement two exception handlers. They are, 
Divide-by-zero exception handler 
Page fault exception handler 

Details of assignment can be found in Piazza and Moodle.
Group information: Individual 
Submission deadline: Wednesday, 12 September 2018, 11:59 PM 

Assignment-3 
At this stage, GemOS implements a command called init which creates the first user process (named as the init process with PID = 1). Source code for init process can be found in user/init.c file. The current init process supports five system calls---getpid(), exit(), write(), expand() and shrink(). gemOS also implements lazy memory allocation by handling page fault exception. Further, divide-by-zero exception is also handled by the gemOS. Now we are ready to take the next step.

Objectives of the assignment are to implement new system calls (signal(), alarm(), sleep() and clone()) along with basic signal handling and multitasking using a round robin (RR) scheduler. To enable multi-tasking design of gemOS, a periodic timer interrupt is initialized with a registered handler function handle_timer_tick, defined in schedule.c. Every invocation of the periodic timer interrupt handler is counted as one tick. Note that, the timer interrupt handler has the same semantic of a div-by-zero fault handler, albeit with a separate interrupt stack.

For this assignment, a list of contexts is maintained in gemOSâ€”accessed using get_ctx_list(), which returns the pointer to the first process (PID = 0). You can iterate the list as an array of pointers using PID as an index. Currently, the maximum number of contexts is defined by a macro MAX_PROCESSES which is 16 (PID=0,1...15). For more details, please refer to the definitions of struct exec_context and process states in include/context.h and include/schedule.h, respectively. A template of the required implementation is provided in schedule.c file. Details of assignment can be found in Piazza and Moodle.
Group information: Individual 
Submission deadline: Wednesday, 24 October 2018, 11:59 PM 

