//BUG FIXED, DON'T OVERWRITE THE VALUE OF RAX HENCE CAREFULLY WRITE THE ORDER OF STATEMENTS. ALWAYS RESTORE RAX AT THE END OF THE FUNCTION.

#include <context.h>
#include <memory.h>
#include <schedule.h>
static u64 numticks;
enum type{TIMER, SLEEPING, DO_EXIT}; 

static struct exec_context *pick_next_context(int type)
{
   	struct exec_context *current = get_current_ctx();	
   	int curr_pid = current->pid;
   	int is_wait=0;

	for(int i=curr_pid+1;i<=15;i++){
		struct exec_context *a = get_ctx_by_pid(i);
   	 	if(a->state==READY)
   	 		return a;
   	 	if(a->state==WAITING)
   	 		is_wait=1;
   	}
   	for(int i=1;i<=curr_pid-1;i++){
   		struct exec_context *a = get_ctx_by_pid(i);
   	 	if(a->state==READY)
   	 		return a;
   	 	if(a->state==WAITING)
   	 		is_wait=1;
   	}

   	if(type==TIMER){
	   	if(current->pid)
	   		return current;
	   	if(is_wait==0){
	   		printf("cleaning up\n");
	   		do_cleanup();
	   	}
	 	struct exec_context *swapper = get_ctx_by_pid(0);
		return swapper;
	}	
	else if(type==SLEEPING){
	 	struct exec_context *swapper = get_ctx_by_pid(0);
		return swapper;
	}
	else if(type==DO_EXIT){
		if(is_wait==0){
			printf("cleaning up\n");
			do_cleanup();	
		}
		struct exec_context *swapper = get_ctx_by_pid(0);
		return swapper;	
	}
}

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
	// asm volatile ("movq %%rbp, %0;" : "=r"(rbp_pointer));
	u64 ustackp,urip;
	asm volatile ("movq 32(%%rbp), %0;" : "=r"(ustackp));
	asm volatile ("movq 8(%%rbp), %0;" : "=r"(urip));


	struct exec_context* current = get_current_ctx();
	printf("\nGot a tick. #ticks = %u\n", ++numticks);   /*XXX Do not modify this line*/ 
	printf("current pid %d \n",current->pid);

	//alarm will be tested only for the init process.
	struct exec_context* init = get_ctx_by_pid(1);	
	if(init->ticks_to_alarm>=1){
		init->ticks_to_alarm = init->ticks_to_alarm - 1;
		if(init->ticks_to_alarm == 0){
			invoke_sync_signal(SIGALRM,&ustackp,&urip);
		}
	}

	for(int i=0;i<=15;i++){
		struct exec_context* itr = get_ctx_by_pid(i);
		if(itr->ticks_to_sleep>=1){
			itr->ticks_to_sleep = itr->ticks_to_sleep-1;
			if(itr->ticks_to_sleep==0){
				itr->state=READY;
			}
		}
	}
	current->state = READY;
	struct exec_context* new = 	pick_next_context(TIMER);
	new->state = RUNNING;

	if(new->pid!=current->pid){
		printf("schedluing in timer: old pid = %d  new pid  = %d\n", current->pid, new->pid); /*XXX: Don't remove*/
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

		asm volatile ("movq 40(%%rbp), %0;" : "=r"(current->regs.entry_ss));
		asm volatile ("movq 32(%%rbp), %0;" : "=r"(current->regs.entry_rsp));
		asm volatile ("movq 24(%%rbp), %0;" : "=r"(current->regs.entry_rflags));
		asm volatile ("movq 16(%%rbp), %0;" : "=r"(current->regs.entry_cs));
		asm volatile ("movq 8(%%rbp), %0;" : "=r"(current->regs.entry_rip));
		asm volatile ("movq (%%rbp), %0;" : "=r"(current->regs.rbp));
		

		// *(rbp_pointer)=new->regs.rbp;
		// *(rbp_pointer+1)=new->regs.entry_rip;
		// *(rbp_pointer+2)=new->regs.entry_cs;
		// *(rbp_pointer+3)=new->regs.entry_rflags;
		// *(rbp_pointer+4)=new->regs.entry_rsp;
		// *(rbp_pointer+5)=new->regs.entry_ss;
		asm volatile ("movq %0, (%%rbp);"
                :: "r" (new->regs.rbp));
		asm volatile ("movq %0, 8(%%rbp);"
	                :: "r" (new->regs.entry_rip));
		asm volatile ("movq %0, 16(%%rbp);"
	                :: "r" (new->regs.entry_cs));
		asm volatile ("movq %0, 24(%%rbp);"
	                :: "r" (new->regs.entry_rflags));
		asm volatile ("movq %0, 32(%%rbp);"
	                :: "r" (new->regs.entry_rsp));
		asm volatile ("movq %0, 40(%%rbp);"
	                :: "r" (new->regs.entry_ss));
	
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
		asm volatile ("movq %0, %%rbx;" :: "r"(new->regs.rbx));
		asm volatile ("movq %0, %%rcx;" :: "r"(new->regs.rcx));
		asm volatile ("movq %0, %%rdx;" :: "r"(new->regs.rdx));
		asm volatile ("movq %0, %%rdi;" :: "r"(new->regs.rdi));
		asm volatile ("movq %0, %%rsi;" :: "r"(new->regs.rsi));
		asm volatile ("movq %0, %%rax;" :: "r"(new->regs.rax));
		asm volatile("mov %%rbp, %%rsp;"
             	  "pop %%rbp;"
               	  "iretq;"
               	   :::"memory");
	}
	
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

}

void do_exit()
{
  /*You may need to invoke the scheduler from here if there are
    other processes except swapper in the system. Make sure you make 
    the status of the current process to UNUSED before scheduling 
    the next process. If the only process alive in system is swapper, 
    invoke do_cleanup() to shutdown gem5 (by crashing it, huh!)
    */
	struct exec_context *current = get_current_ctx(); 
	current->state = UNUSED;
	struct exec_context* new = 	pick_next_context(DO_EXIT);
	new->state = RUNNING;
	printf("exiting process pid %d\n", current->pid);
	printf("schedluing after do_exit: old pid = %d  new pid  = %d\n", current->pid, new->pid); /*XXX: Don't remove*/

	set_tss_stack_ptr(new);
	set_current_ctx(new);

	asm volatile ("movq %0, (%%rbp);"
                :: "r" (new->regs.rbp));
	asm volatile ("movq %0, 8(%%rbp);"
                :: "r" (new->regs.entry_rip));
	asm volatile ("movq %0, 16(%%rbp);"
                :: "r" (new->regs.entry_cs));
	asm volatile ("movq %0, 24(%%rbp);"
                :: "r" (new->regs.entry_rflags));
	asm volatile ("movq %0, 32(%%rbp);"
                :: "r" (new->regs.entry_rsp));
	asm volatile ("movq %0, 40(%%rbp);"
                :: "r" (new->regs.entry_ss));

	asm volatile ("movq %0, %%r8;" :: "r"(new->regs.r8));
	asm volatile ("movq %0, %%r9;" :: "r"(new->regs.r9));
	asm volatile ("movq %0, %%r10;" :: "r"(new->regs.r10));
	asm volatile ("movq %0, %%r11;" :: "r"(new->regs.r11));
	asm volatile ("movq %0, %%r12;" :: "r"(new->regs.r12));
	asm volatile ("movq %0, %%r13;" :: "r"(new->regs.r13));
	asm volatile ("movq %0, %%r14;" :: "r"(new->regs.r14));
	asm volatile ("movq %0, %%r15;" :: "r"(new->regs.r15));
	asm volatile ("movq %0, %%rbx;" :: "r"(new->regs.rbx));
	asm volatile ("movq %0, %%rcx;" :: "r"(new->regs.rcx));
	asm volatile ("movq %0, %%rdx;" :: "r"(new->regs.rdx));
	asm volatile ("movq %0, %%rdi;" :: "r"(new->regs.rdi));
	asm volatile ("movq %0, %%rsi;" :: "r"(new->regs.rsi));
	asm volatile ("movq %0, %%rax;" :: "r"(new->regs.rax));
	asm volatile("mov %%rbp, %%rsp;"
             	  "pop %%rbp;"
               	  "iretq;"
               	   :::"memory");

}

/*system call handler for sleep*/
long do_sleep(u32 ticks)
{

	printf("syscall handler for sleep called\n");
	struct exec_context *current = get_current_ctx(); 
	current->ticks_to_sleep = ticks;
	current->state  = WAITING;
	
	// u64* save_rbp;
	// asm volatile("mov %%cs, %0" : "=r" ((current->regs).entry_cs));
 //    asm volatile("mov %%ss, %0" : "=r" ((current->regs).entry_cs));
    // asm volatile("mov %%rbp, %0" : "=r"(save_rbp));
    u64* temp = (u64*)((((u64)current->os_stack_pfn+1)<<12)-8);
    current->regs.rbp = *(temp-10);
    current->regs.entry_rip = *(temp-4);
    current->regs.entry_rflags = *(temp-2);
    current->regs.entry_rsp = *(temp-1);
    current->regs.entry_cs = 0x23;
    current->regs.entry_ss = 0x2b;

	struct exec_context* new = 	pick_next_context(SLEEPING);
	printf("schedluing due to sleep: old pid = %d  new pid  = %d\n", current->pid, new->pid); /*XXX: Don't remove*/
	new->state = RUNNING; 

	// *(save_rbp)=new->regs.rbp;
	// *(save_rbp+1)=new->regs.entry_rip;
	// *(save_rbp+2)=new->regs.entry_cs;
	// *(save_rbp+3)=new->regs.entry_rflags;
	// *(save_rbp+4)=new->regs.entry_rsp;
	// *(save_rbp+5)=new->regs.entry_ss;
	asm volatile ("movq %0, (%%rbp);"
                :: "r" (new->regs.rbp));
	asm volatile ("movq %0, 8(%%rbp);"
                :: "r" (new->regs.entry_rip));
	asm volatile ("movq %0, 16(%%rbp);"
                :: "r" (new->regs.entry_cs));
	asm volatile ("movq %0, 24(%%rbp);"
                :: "r" (new->regs.entry_rflags));
	asm volatile ("movq %0, 32(%%rbp);"
                :: "r" (new->regs.entry_rsp));
	asm volatile ("movq %0, 40(%%rbp);"
                :: "r" (new->regs.entry_ss));

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
	asm volatile ("movq %0, %%rbx;" :: "r"(new->regs.rbx));
	asm volatile ("movq %0, %%rcx;" :: "r"(new->regs.rcx));
	asm volatile ("movq %0, %%rdx;" :: "r"(new->regs.rdx));
	asm volatile ("movq %0, %%rdi;" :: "r"(new->regs.rdi));
	asm volatile ("movq %0, %%rsi;" :: "r"(new->regs.rsi));
	asm volatile ("movq %0, %%rax;" :: "r"(new->regs.rax));

	asm volatile ("movq %rbp, %rsp;");
	asm volatile ("pop %rbp;");
    asm volatile ("iretq");
    
}

/*
  system call handler for clone, create thread like 
  execution contexts
*/
long do_clone(void *th_func, void *user_stack)
{
 
	struct exec_context *new = get_new_ctx();
	struct exec_context* init = get_ctx_by_pid(1);
	new->type = init->type;
	new->state = READY;
	new->used_mem = init->used_mem;
	new->pgd = init->pgd;
	new->os_stack_pfn = os_pfn_alloc(OS_PT_REG);
	new->os_rsp = init->os_rsp;
	//DOUBT
	new->ticks_to_alarm = 0;
	new->ticks_to_sleep = 0;
	new->pending_signal_bitmap = 0x0;
	for(int i=0;i<MAX_SIGNALS;i++){
		new->sighandlers[i] = init->sighandlers[i];

	}
	char str[20];
	char temp[CNAME_MAX];

	int i=0;
	while(init->name[i]!='\0'){
		temp[i] = init->name[i];
		i++;
	}
    int rem, len = 0, n,num;
    n = new->pid;
    num = n;
    while (n != 0){
        len++;
        n /= 10;
    }
    for (int i = 0; i < len; i++){
        rem = num % 10;
        num = num / 10;
        str[len - (i + 1)] = rem + '0';
    }

    str[len] = '\0';
    for(int j=0;j<=len;j++){
    	temp[i++] = str[j];
    }
    for(int i=0;i<CNAME_MAX;i++)
    	new->name[i] = temp[i];

	for(int i=0;i<MAX_MM_SEGS;i++){
		new->mms[i].start = init->mms[i].start;
		new->mms[i].end = init->mms[i].end;
		new->mms[i].next_free = init->mms[i].next_free;
		new->mms[i].access_flags = init->mms[i].access_flags;
	}

	new->regs = init->regs;
	new->regs.entry_cs = 0x23;
	new->regs.entry_ss = 0x2b;
	new->regs.entry_rip = (u64)(th_func);
	new->regs.entry_rsp = (u64)(user_stack);
	new->regs.rbp = new->regs.entry_rsp;
	new->alarm_config_time =0 ;

	u64* address = (u64*)((((u64)init->os_stack_pfn+1)<<12)-8);
  	new->regs.entry_rflags = *(address-2);
  	printf("process CLONED and created pid %d, name %s\n",new->pid,new->name);

	return 0; 
}

long invoke_sync_signal(int signo, u64 *ustackp, u64 *urip)
{
   /*If signal handler is registered, manipulate user stack and RIP to execute signal handler*/
   /*ustackp and urip are pointers to user RSP and user RIP in the exception/interrupt stack*/
   printf("Called signal with ustackp=%x urip=%x\n", *ustackp, *urip);
   /*Default behavior is exit( ) if sighandler is not registered for SIGFPE or SIGSEGV.
    Ignore for SIGALRM*/
 	struct exec_context *current = get_current_ctx();  
 	// if(signo==SIGALRM)
 	// 	printf("alarm\n");
 	// if(signo==SIGSEGV)
 	// 	printf("segmentation fault here\n");
 	// if(signo==SIGFPE)
 	// 	printf("floating point\n");

   	if(current->sighandlers[signo]==NULL){
		if(signo != SIGALRM)
    		do_exit();
   	}
   	else{
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

