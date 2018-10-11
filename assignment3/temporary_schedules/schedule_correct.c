// #include <stdio.h>
#include <context.h>
#include <memory.h>
#include <schedule.h>
// #include <stdlib.h>
static u64 numticks;

//don't use it.
static void save_current_context()
{
  /*Your code goes in here*/ 
} 

static void schedule_context(struct exec_context *next)
{
  /*Your code goes in here. get_current_ctx() still returns the old context*/
 struct exec_context *current = get_current_ctx();
 printf("schedluing: old pid = %d  new pid  = %d\n", current->pid, next->pid); /*XXX: Don't remove*/
/*These two lines must be executed*/
 set_tss_stack_ptr(next);
 set_current_ctx(next);
 return;
}

static struct exec_context *pick_next_context(struct exec_context *list)
{
  /*Your code goes in here*/
  
   return NULL;
}
static void schedule()
{
/* 
 struct exec_context *next;
 struct exec_context *current = get_current_ctx(); 
 struct exec_context *list = get_ctx_list();
 next = pick_next_context(list);
 schedule_context(next);
*/     
}

// static void do_sleep_and_alarm_account()
// {
//  All processes in sleep() must decrement their sleep count 

// }

/*The five functions above are just a template. You may change the signatures as you wish*/
void handle_timer_tick()
{
 /*
   This is the timer interrupt handler. 
   You should account timer ticks for alarm and sleep
   and invoke schedule
 */
	u64 save_rsp;
	u64* rsp_pointer;
	u64* rbp_pointer;
	u64 ustackp,urip;
	asm volatile ("movq %%rbp, %0;" : "=r"(rbp_pointer));
	asm volatile ("push %r8;");
	asm volatile ("push %r9;");
	asm volatile ("push %r10;");
	asm volatile ("push %r11;");
	asm volatile ("push %r12;");
	asm volatile ("push %r13;");
	asm volatile ("push %r14;");
	asm volatile ("push %r15;");
	asm volatile ("push %rax;");
	asm volatile ("push %rbx;");
	asm volatile ("push %rcx;");
	asm volatile ("push %rdx;");
	asm volatile ("push %rdi;");
	asm volatile ("push %rsi;");
	asm volatile ("movq %%rsp, %0;" : "=r"(save_rsp));
	asm volatile ("movq %%rsp, %0;" : "=r"(rsp_pointer));
	
	asm volatile ("movq 32(%%rbp), %0;" : "=r"(ustackp));
	asm volatile ("movq 8(%%rbp), %0;" : "=r"(urip));
	// asm volatile ("movq %%rbp, %0;" : "=r"(save_rbp));
	// if(rsp_pointer==save_rsp)
	// 	printf("thanks god\n");
	// printf("timer tick handler invoked\n");


	//PART A
	//ticks to alarm decrement.
	// u64* a=(u64*)(save_rbp+1);
	// u64* b=(u64*)(save_rbp+4);

	struct exec_context* current = get_current_ctx();
	printf("current pid %d \n",current->pid);
	if(current->ticks_to_alarm>=1){
		current->ticks_to_alarm = current->ticks_to_alarm - 1;
		if(current->ticks_to_alarm == 0){
			invoke_sync_signal(SIGALRM,&ustackp,&urip);
		}
	}


	//PART B
	//if ticks to sleep = 0, flag=1. 

	//save values from stack to PCB of the get current context.
	//take special values from rbp offsets.
	//load init process.
	//invoke scheduler.
	//
	// int flag=0;
	// struct exec_context* current = get_ctx_by_pid(1);
	// if(current->ticks_to_sleep>=1){
	// 	current->ticks_to_sleep = current->ticks_to_sleep-1;
	// 	if(current->ticks_to_sleep==0)
	// 		flag=1; 
	// }

	int flag=0;
	struct exec_context* init = get_ctx_by_pid(1);
	if(init->ticks_to_sleep>=1){
		init->ticks_to_sleep = init->ticks_to_sleep-1;
		if(init->ticks_to_sleep==0)
			flag=1; 
	}

	if(flag==1){
		// printf("stop sleeping !!!\n");
		
		// asm volatile ("movq -8(%%rbp), %0;" : "=r"(current->regs.r8));
		// asm volatile ("movq -16(%%rbp), %0;" : "=r"(current->regs.r9));
		// asm volatile ("movq -24(%%rbp), %0;" : "=r"(current->regs.r10));
		// asm volatile ("movq -32(%%rbp), %0;" : "=r"(current->regs.r11));
		// asm volatile ("movq -40(%%rbp), %0;" : "=r"(current->regs.r12));
		// asm volatile ("movq -48(%%rbp), %0;" : "=r"(current->regs.r13));
		// asm volatile ("movq -56(%%rbp), %0;" : "=r"(current->regs.r14));
		// asm volatile ("movq -64(%%rbp), %0;" : "=r"(current->regs.r15));
		// asm volatile ("movq -72(%%rbp), %0;" : "=r"(current->regs.rax));
		// asm volatile ("movq -80(%%rbp), %0;" : "=r"(current->regs.rbx));
		// asm volatile ("movq -88(%%rbp), %0;" : "=r"(current->regs.rcx));
		// asm volatile ("movq -96(%%rbp), %0;" : "=r"(current->regs.rdx));
		// asm volatile ("movq -104(%%rbp), %0;" : "=r"(current->regs.rdi));
		// asm volatile ("movq -112(%%rbp), %0;" : "=r"(current->regs.rsi)); 
		current->regs.rsi = *(rsp_pointer);
		current->regs.rdi = *(rsp_pointer+1);
		current->regs.rdx = *(rsp_pointer+2);
		current->regs.rcx = *(rsp_pointer+3);
		current->regs.rbx = *(rsp_pointer+4);
		current->regs.rax = *(rsp_pointer+5);
		current->regs.r15 = *(rsp_pointer+6);
		current->regs.r14 = *(rsp_pointer+7);
		current->regs.r13 = *(rsp_pointer+8);
		current->regs.r12 = *(rsp_pointer+9);
		current->regs.r11 = *(rsp_pointer+10);
		current->regs.r10 = *(rsp_pointer+11);
		current->regs.r9 = *(rsp_pointer+12);
		current->regs.r8 = *(rsp_pointer+13);

		//check if the offsets are correct.
		asm volatile ("movq 40(%%rbp), %0;" : "=r"(current->regs.entry_ss));
		asm volatile ("movq 32(%%rbp), %0;" : "=r"(current->regs.entry_rsp));
		asm volatile ("movq 24(%%rbp), %0;" : "=r"(current->regs.entry_rflags));
		asm volatile ("movq 16(%%rbp), %0;" : "=r"(current->regs.entry_cs));
		asm volatile ("movq 8(%%rbp), %0;" : "=r"(current->regs.entry_rip));
		asm volatile ("movq (%%rbp), %0;" : "=r"(current->regs.rbp));
		
		struct exec_context* swapper = get_ctx_by_pid(0);

		// if(get_current_ctx()->pid==0){
		// 	printf("it's swapper\n");
		// }
		struct exec_context* new = get_ctx_by_pid(1);
		swapper->state = WAITING;
		new->state = RUNNING;
		*(rbp_pointer)=new->regs.rbp;
		*(rbp_pointer+1)=new->regs.entry_rip;
		*(rbp_pointer+2)=new->regs.entry_cs;
		*(rbp_pointer+3)=new->regs.entry_rflags;
		*(rbp_pointer+4)=new->regs.entry_rsp;
		*(rbp_pointer+5)=new->regs.entry_ss;
		// asm volatile ("movq %0, (%%rbp);"
  //                   :: "r" (new->regs.rbp));
		// asm volatile ("movq %0, 8(%%rbp);"
  //                   :: "r" (new->regs.entry_rip));
		// asm volatile ("movq %0, 16(%%rbp);"
  //                   :: "r" (new->regs.entry_cs));
		// asm volatile ("movq %0, 24(%%rbp);"
  //                   :: "r" (new->regs.entry_rflags));
		// asm volatile ("movq %0, 32(%%rbp);"
  //                   :: "r" (new->regs.entry_rsp));
		// asm volatile ("movq %0, 40(%%rbp);"
  //                   :: "r" (new->regs.entry_ss));
		// printf("check 1\n");
		ack_irq();
		set_tss_stack_ptr(new);
		set_current_ctx(new);

		asm volatile ("movq %0, %%r8;" :: "r"(new->regs.r8));
		asm volatile ("movq %0, %%r9;" :: "r"(new->regs.r9));
		asm volatile ("movq %0, %%r10;" :: "r"(new->regs.r10));
		asm volatile ("movq %0, %%r11;" :: "r"(new->regs.r11));
		asm volatile ("movq %0, %%r12;" :: "r"(new->regs.r12));
		asm volatile ("movq %0, %%r13;" :: "r"(new->regs.r13));
		asm volatile ("movq %0, %%r14;" :: "r"(new->regs.r14));
		asm volatile ("movq %0, %%r15;" :: "r"(new->regs.r15));
		asm volatile ("movq %0, %%rax;" :: "r"(new->regs.rax));
		asm volatile ("movq %0, %%rbx;" :: "r"(new->regs.rbx));
		asm volatile ("movq %0, %%rcx;" :: "r"(new->regs.rcx));
		asm volatile ("movq %0, %%rdx;" :: "r"(new->regs.rdx));
		asm volatile ("movq %0, %%rdi;" :: "r"(new->regs.rdi));
		asm volatile ("movq %0, %%rsi;" :: "r"(new->regs.rsi));
		// printf("check 2\n");
		asm volatile("mov %%rbp, %%rsp;"
             	  "pop %%rbp;"
               	  "iretq;"
               	   :::"memory");
		//invoke scheduler.
	}
	
	printf("Got a tick. #ticks = %u\n", ++numticks);   /*XXX Do not modify this line*/ 
  	ack_irq();  /*acknowledge the interrupt, next interrupt */
	asm volatile ("movq %0, 8(%%rbp);"
                    :: "r" (urip));
	asm volatile ("movq %0, 32(%%rbp);"
                    :: "r" (ustackp));
	asm volatile ("movq %0, %%rsp;"
                    :: "r" (save_rsp));

    
	asm volatile ("pop %rsi;");
    asm volatile ("pop %rdi;");
    asm volatile ("pop %rdx;");
    asm volatile ("pop %rcx;");
    asm volatile ("pop %rbx;");
    asm volatile ("pop %rax;");
    asm volatile ("pop %r15;");
    asm volatile ("pop %r14;");
    asm volatile ("pop %r13;");
    asm volatile ("pop %r12;");
    asm volatile ("pop %r11;");
    asm volatile ("pop %r10;");
    asm volatile ("pop %r9;");
    asm volatile ("pop %r8;");
  	asm volatile("mov %%rbp, %%rsp;"
             	  "pop %%rbp;"
               	  "iretq;"
               	   :::"memory");

  //decrement the timer for all the processes, save and restore the PCB. (GPR's save and restore).

}

void do_exit()
{
  /*You may need to invoke the scheduler from here if there are
    other processes except swapper in the system. Make sure you make 
    the status of the current process to UNUSED before scheduling 
    the next process. If the only process alive in system is swapper, 
    invoke do_cleanup() to shutdown gem5 (by crashing it, huh!)
    */
    do_cleanup();  /*Call this conditionally, see comments above*/
}

/*system call handler for sleep*/
long do_sleep(u32 ticks)
{

	printf("syscall handler for sleep called\n");
	struct exec_context *current = get_current_ctx(); 
	current->ticks_to_sleep = ticks;
	current->state  = WAITING;

	//can't understand.
	// printf("reached stage 1\n");
	u64* save_rbp;
	asm volatile("mov %%cs, %0" : "=r" ((current->regs).entry_cs));
    asm volatile("mov %%ss, %0" : "=r" ((current->regs).entry_cs));
    asm volatile("mov %%rbp, %0" : "=r"(save_rbp));
    u64* temp = (u64*)((((u64)current->os_stack_pfn+1)<<12)-8);
    current->regs.rbp = *(temp-10);
    current->regs.entry_rip = *(temp-4);
    current->regs.entry_rflags = *(temp-2);
    current->regs.entry_rsp = *(temp-1);
    current->regs.entry_cs = 0x23;
    current->regs.entry_ss = 0x2b;

    

 //    struct exec_context** ctx_list = get_ctx_list();
	// struct exec_context* current = ctx_list[0];
	// printf("reached stage 2\n");
	struct exec_context* swapper=get_ctx_by_pid(0);
	swapper->state = RUNNING; 



    //!!!!fill the extra values on the stack from the PCB of swapper.
	//check the offsets from rbp.
	// asm volatile("sub $0x30, %rsp");
	// u64* save_rsp;
	// asm volatile ("movq %%rsp, %0;" : "=r"(save_rsp));
	*(save_rbp)=swapper->regs.rbp;
	*(save_rbp+1)=swapper->regs.entry_rip;
	*(save_rbp+2)=swapper->regs.entry_cs;
	*(save_rbp+3)=swapper->regs.entry_rflags;
	*(save_rbp+4)=swapper->regs.entry_rsp;
	*(save_rbp+5)=swapper->regs.entry_ss;


 //    asm volatile ("movq %0, 40(%%rbp);" :: "r"(c));
	// asm volatile ("movq %0, 32(%%rbp);" :: "r"(swapper->regs.entry_rsp));
	// asm volatile ("movq %0, 24(%%rbp);" :: "r"(swapper->regs.entry_rflags));
	// asm volatile ("movq %0, 16(%%rbp);" :: "r"(swapper->regs.entry_cs));
	// asm volatile ("movq %0, 8(%%rbp);" :: "r"(swapper->regs.entry_rip));
	//!!!!set_tss_...

	set_tss_stack_ptr(swapper);
	set_current_ctx(swapper);

	// printf("reached stage 3\n");
	//!!!!load values from PCB of swapper.
	asm volatile ("movq %0, %%r8;" :: "r"(swapper->regs.r8));
	asm volatile ("movq %0, %%r9;" :: "r"(swapper->regs.r9));
	asm volatile ("movq %0, %%r10;" :: "r"(swapper->regs.r10));
	asm volatile ("movq %0, %%r11;" :: "r"(swapper->regs.r11));
	asm volatile ("movq %0, %%r12;" :: "r"(swapper->regs.r12));
	asm volatile ("movq %0, %%r13;" :: "r"(swapper->regs.r13));
	asm volatile ("movq %0, %%r14;" :: "r"(swapper->regs.r14));
	asm volatile ("movq %0, %%r15;" :: "r"(swapper->regs.r15));
	asm volatile ("movq %0, %%rax;" :: "r"(swapper->regs.rax));
	asm volatile ("movq %0, %%rbx;" :: "r"(swapper->regs.rbx));
	asm volatile ("movq %0, %%rcx;" :: "r"(swapper->regs.rcx));
	asm volatile ("movq %0, %%rdx;" :: "r"(swapper->regs.rdx));
	asm volatile ("movq %0, %%rdi;" :: "r"(swapper->regs.rdi));
	asm volatile ("movq %0, %%rsi;" :: "r"(swapper->regs.rsi));

	// printf("reached stage 4\n");
	asm volatile ("movq %rbp, %rsp;");
	// asm volatile ("movq %0, (%%rsp);"
 //                    :: "r" (save_rsp));

    asm volatile ("pop %rbp;");
    asm volatile ("iretq");
    
}

/*
  system call handler for clone, create thread like 
  execution contexts
*/
long do_clone(void *th_func, void *user_stack)
{
 
	// struct exec_context{
//              u32 pid;
//              u8 type;
//              u8 state;     /*Can be any of the states mentioned in schedule.h*/
//              u16 used_mem;
//              u32 pgd;     /*CR3 should point to this PFN when schedulled*/
//              u32 os_stack_pfn;  /*Must be unique for every context*/
//              u64 os_rsp;
//              struct mm_segment mms[MAX_MM_SEGS];
//              char name[CNAME_MAX];
//              struct user_regs regs;          /*Saved copy of user registers*/
//              u32 pending_signal_bitmap;      /*Pending signal bitmap*/
//              void* sighandlers[MAX_SIGNALS]; /*Signal handler pointers to functions (in user space)*/
//              u32 ticks_to_sleep;    /*Remaining ticks before sleep expires*/
//              u32 ticks_to_alarm;   /*Remaining ticks before raising SIGALRM*/
// };

// struct user_regs{
//                     u64 r15;
//                     u64 r14;
//                     u64 r13;
//                     u64 r12;
//                     u64 r11;
//                     u64 r10;
//                     u64 r9;
//                     u64 r8;
//                     u64 rbp;
//                     u64 rdi;
//                     u64 rsi;
//                     u64 rdx;
//                     u64 rcx;
//                     u64 rbx;
//                     u64 rax;
//                     u64 entry_rip;  /*Error code for PF*/
//                     u64 entry_cs;   RIP for PF
//                     u64 entry_rflags;
//                     u64 entry_rsp;
//                     u64 entry_ss;
// };


// 	/*my code*/
// 	struct exec_context *new = get_new_ctx();
// 	struct exec_context* ctx_list[16] = get_ctx_list();
// 	struct exec_context* init = ctx_list[1];
	
// 	new.type = init.type;
// 	//dobut regarding the state.
// 	new.state = READY;
// 	new.used_mem = init.used_mem;
// 	new.pgd = init.pgd;
// 	new.os_stack_pfn = os_pfn_alloc(OS_PT_REG);
// 	// new.os_rsp = ??
// 	new.ticks_to_alarm = init.ticks_to_alarm;
// 	new.ticks_to_sleep = init.ticks_to_sleep;
// 	new.pending_signal_bitmap = 0x0;
// 	new.sighandlers = init.sighandlers;

// 	char buff[20];
// 	sprtinf(buff,"%d",new.pid);
// 	char temp[CNAME_MAX];
// 	strcpy(temp,init.name);
// 	strcat(temp,buff);
// 	new.name = temp;
// 	//new.mm_segment = ???









// 
}

long invoke_sync_signal(int signo, u64 *ustackp, u64 *urip)
{
   /*If signal handler is registered, manipulate user stack and RIP to execute signal handler*/
   /*ustackp and urip are pointers to user RSP and user RIP in the exception/interrupt stack*/
   printf("Called signal with ustackp=%x urip=%x\n", *ustackp, *urip);
   // printf("oh shit\n");
   /*Default behavior is exit( ) if sighandler is not registered for SIGFPE or SIGSEGV.
    Ignore for SIGALRM*/
 	struct exec_context *current = get_current_ctx();  
 	if(signo==SIGALRM)
 		printf("alarm\n");
 	if(signo==SIGSEGV)
 		printf("segmentation fault here\n");
 	if(signo==SIGFPE)
 		printf("floating point\n");

   	if(current->sighandlers[signo]==NULL){
		if(signo != SIGALRM)
    		do_exit();
   	}
   	else{
   		
   		// *ustackp = *ustackp - 1;
   		// *((u64*) *ustackp) = *urip;
   		// *urip = (u64) current->sighandlers[signo];
   		
   		u64 **a = (u64**) ustackp;
   		*a = *a - 1;
   		**a = *urip;
   		*urip = (u64) current->sighandlers[signo];
   		
   	}
   	return 0;

}
/*system call handler for signal, to register a handler*/
long do_signal(int signo, unsigned long handler)
{
	struct exec_context *current = get_current_ctx();
	current->sighandlers[signo] = (void*)handler;
	return 0;
}

/*system call handler for alarm*/
long do_alarm(u32 ticks)
{
	printf("Setting alarm\n");
	struct exec_context *current = get_current_ctx();
	current->ticks_to_alarm = ticks;
	return 0;
}



// #ifndef __CONTEXT_H_
// #define __CONTEXT_H_

// #include<types.h>
// #include<schedule.h>
// #define CNAME_MAX 64
// #define MAX_PROCESSES 16

// enum{
//            EXEC_CTX_BOOT,
//            EXEC_CTX_OS,
//            EXEC_CTX_USER,
//            MAX_CTX
// };
// enum{
//           MM_SEG_CODE,
//           MM_SEG_RODATA,
//           MM_SEG_DATA,
//           MM_SEG_STACK,
//           MAX_MM_SEGS
// };

// #define MM_RD 0x1
// #define MM_WR 0x2
// #define MM_EX 0x4
// #define MM_SH 0x8


// #define CODE_START       0x100000000  
// #define RODATA_START     0x140000000 
// #define DATA_START       0x180000000
// #define STACK_START      0x800000000 

// #define CODE_PAGES       0x8

// #define MAX_STACK_SIZE   0x1000000


// struct mm_segment{
//                    unsigned long start;
//                    unsigned long end;
//                    unsigned long next_free; 
//                    u32 access_flags;   /*R=1, W=2, X=4, S=8*/ 
// };


// struct user_regs{
//                     u64 r15;
//                     u64 r14;
//                     u64 r13;
//                     u64 r12;
//                     u64 r11;
//                     u64 r10;
//                     u64 r9;
//                     u64 r8;
//                     u64 rbp;
//                     u64 rdi;
//                     u64 rsi;
//                     u64 rdx;
//                     u64 rcx;
//                     u64 rbx;
//                     u64 rax;
//                     u64 entry_rip;  /*Error code for PF*/
//                     u64 entry_cs;   RIP for PF
//                     u64 entry_rflags;
//                     u64 entry_rsp;
//                     u64 entry_ss;
// };

// struct exec_context{
//              u32 pid;
//              u8 type;
//              u8 state;     /*Can be any of the states mentioned in schedule.h*/
//              u16 used_mem;
//              u32 pgd;     /*CR3 should point to this PFN when schedulled*/
//              u32 os_stack_pfn;  /*Must be unique for every context*/
//              u64 os_rsp;
//              struct mm_segment mms[MAX_MM_SEGS];
//              char name[CNAME_MAX];
//              struct user_regs regs;          /*Saved copy of user registers*/
//              u32 pending_signal_bitmap;      /*Pending signal bitmap*/
//              void* sighandlers[MAX_SIGNALS]; /*Signal handler pointers to functions (in user space)*/
//              u32 ticks_to_sleep;    /*Remaining ticks before sleep expires*/
//              u32 ticks_to_alarm;   /*Remaining ticks before raising SIGALRM*/
// };


// extern struct exec_context* create_context(char *, u8);
// extern int exec_init (struct exec_context *);

// extern struct exec_context *get_current_ctx(void);
// extern void set_current_ctx(struct exec_context *);
// extern struct exec_context *get_ctx_by_pid(u32 pid);
// extern void load_swapper();
// extern void init_swapper();
// extern void handle_timer_tick();
// extern struct exec_context *get_ctx_list();
// extern struct exec_context *get_new_ctx(); 
// extern void do_cleanup(void);
// #endif
