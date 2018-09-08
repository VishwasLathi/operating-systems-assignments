#include<init.h>
#include<lib.h>
#include<context.h>
#include<memory.h>

/*System Call handler*/
u64 do_syscall(int syscall, u64 param1, u64 param2, u64 param3, u64 param4)
{
    struct exec_context *current = get_current_ctx();
    printf("[GemOS] System call invoked. syscall no  = %d\n", syscall);
    switch(syscall)
    {
          case SYSCALL_EXIT:
                              printf("[GemOS] exit code = %d\n", (int) param1);
                              do_exit();
                              break;
          case SYSCALL_GETPID:
                              printf("[GemOS] getpid called for process %s, with pid = %d\n", current->name, current->id);
                              return current->id;      
          case SYSCALL_WRITE:
                            {  
                              printf("I will be writing: ");
                              char *buff = (char*)param1;
                              u32 length = (u32)param2;
                              if(length<0||length>1024)
                                  return -1;
                              u32 l1_index, l2_index, l3_index,l4_index;
                              u64 *l1,*l2,*l3,*l4;
                              u64 virtual_add;
        
                              for(int i=0;i<2;i++){
                                if(i==0)
                                  virtual_add = param1;
                                else{
                                  if((param1 + length)/4096 > (param1)/4096)
                                    //should it be param1 + length -1 ?,
                                    //doing param1 + length because '\0' is getting stored in param1+length. 
                                    virtual_add  = param1+length ;
                                  else
                                    continue;  
                                }
                                      
                                l1_index = (virtual_add)>>12 & 0x1FF;
                                l2_index = (virtual_add)>>21 & 0x1FF;
                                l3_index=(virtual_add)>>30 & 0x1FF;
                                l4_index=(virtual_add)>>39;

                                l4=(u64*)osmap(current->pgd);

                                if((l4[l4_index]&0x1) == 0)
                                  return -1;  
                                l3=(u64*)osmap((u32)(l4[l4_index]>>12));

                                if((l3[l3_index]&0x1)==0)
                                  return -1;
                    
                                l2=(u64*)osmap((u32)(l3[l3_index]>>12));

                                if((l2[l2_index]&0x1)==0)
                                  return -1;
                                
                                l1=(u64*)osmap((u32)(l2[l2_index]>>12));

                                if((l1[l1_index]&0x1)==0)
                                  return -1;
                              }
                              int i=0;
                              while(buff[i]!='\0')
                                i++;
                              if(i!=length){
                                printf("incorrect length\n");
                                return -1;    
                              }
                              for(u32 i=0;i<length;i++){
                                printf("%c",buff[i]);
                              }

                              printf("\n");
                                

                            }
          case SYSCALL_EXPAND:
                            {  
                              struct mm_segment m;
                              u32 size = param1;
                              int flags = param2;
                              if(size>512)
                                return 0;
                              if(flags==MAP_RD)
                                m=current->mms[ MM_SEG_RODATA];
                              else
                                m=current->mms[MM_SEG_DATA];
                              u64 next_free = m.next_free;
                              u64 start=m.start;
                              u64 end = m.end;
                              
                              if(next_free+size*4096-1> end){
                                    printf("size moved out of boundary while expanding\n");
                                    return 0;
                                }
                              if(flags==MAP_RD)
                                current->mms[MM_SEG_RODATA].next_free = next_free+size*4096;
                              else
                                current->mms[MM_SEG_DATA].next_free = next_free+size*4096;
                              return next_free;

                            }
          case SYSCALL_SHRINK:
                            {  
                              struct mm_segment m;
                              u32 size = param1;
                              int flags = param2;

                              if(flags==MAP_RD)
                                m=current->mms[ MM_SEG_RODATA];
                              else
                                m=current->mms[MM_SEG_DATA];

                              u64 next_free = m.next_free;
                              u64 start=m.start;
                              u64 end = m.end;
                              u32 l1_index, l2_index, l3_index,l4_index;
                              u64 *l1,*l2,*l3,*l4;

                              if(next_free-size*4096<start){
                                    printf("size moved out of boundary while contracting\n");
                                    return 0; 
                                }
                              for(int i=1;i<=size;i++){
                                u64 virtual_add = next_free-i*4096;
                                
                    
                                l1_index = (virtual_add)>>12 & 0x1FF;
                                l2_index = (virtual_add)>>21 & 0x1FF;
                                l3_index=(virtual_add)>>30 & 0x1FF;
                                l4_index=(virtual_add)>>39;

                                l4=(u64*)osmap(current->pgd);
                                if((l4[l4_index]&0x1) == 0)
                                    continue;
                                l3=(u64*)osmap((u32)(l4[l4_index]>>12));
                                if((l3[l3_index]&0x1) == 0)
                                    continue;   
                                l2=(u64*)osmap((u32)(l3[l3_index]>>12));
                                if((l2[l2_index]&0x1) == 0)
                                    continue;
                                l1=(u64*)osmap((u32)(l2[l2_index]>>12));
                                if((l1[l1_index]&0x1)==1){
                                    os_pfn_free(USER_REG,l1[l1_index]>>12);
                                    l1[l1_index] &= 0x0; 
                                }

                                if(flags==MAP_RD)
                                  current->mms[MM_SEG_RODATA].next_free = next_free-size*4096;
                                else
                                  current->mms[MM_SEG_DATA].next_free = next_free-size*4096;
                                return next_free-size*4096;
                              }
                                   
                            }
                             
          default:
                              return -1;
                                
    }
    return 0;   /*GCC shut up!*/
}

extern u64 handle_div_by_zero(void)
{
    register u64* os_stack asm("rbp");
  
    printf("Div-by-zero handler detected at %x\n",*(os_stack+1));
    //printf("+8 Div-by-zero handler detected at %x\n",*(os_stack+8));
    do_exit();
    return 0;
}


//allocates memory
void allocate(u64 virtual_add,struct mm_segment m){
    
    struct exec_context *current = get_current_ctx();
    u64 *l4 = (u64*)osmap(current->pgd);
    u32 flag = m.access_flags;
    
    u32 l1_index, l2_index, l3_index,l4_index;
    
    l1_index = (virtual_add)>>12 & 0x1FF;
    l2_index = (virtual_add)>>21 & 0x1FF;
    l3_index=(virtual_add)>>30 & 0x1FF;
    l4_index=(virtual_add)>>39;
    
    //u1 contain the virtual address of L1 page table.
    u64 *l2,*l3,*l1;
  
    for(int level=4;level>=1;level--){
        if(level==1){
            if((l1[l1_index] & 0x1)==1){
                printf("memory already allocated\n");
                return;    
            }
            u32 a=os_pfn_alloc(USER_REG);
            l1[l1_index] |= a<<12;
            l1[l1_index] |=0x5;           
            
            if((flag & 0x2) == 2)
                l1[l1_index]|=0x2;     
        }
        else if(level==2){
            if((l2[l2_index] & 0x1)==0){
                u32 a=os_pfn_alloc(OS_PT_REG);
                l1=(u64*)osmap(a);
                for(u32 i=0;i<512;i++)
                    l1[i]=0;
                l2[l2_index]|=a<<12;
                l2[l2_index]|=0x5;
            }
            else{
                l1=(u64*)osmap((u32)(l2[l2_index]>>12));
            }
            if((flag & 0x2) == 2)
                l2[l2_index] |=0x2;
        }
        else if(level==3){
            if((l3[l3_index] & 0x1)==0){
                u32 a=os_pfn_alloc(OS_PT_REG);
                l2=(u64*)osmap(a);
                for(u32 i=0;i<512;i++)
                    l2[i]=0;
                l3[l3_index]|= a<<12;
                l3[l3_index]|= 0x5;
            }
            else{
                l2=(u64*)osmap((u32)(l3[l3_index]>>12));     
            }
            if((flag & 0x2) == 2)
                l3[l3_index] |= 0x2;     
                
        }
        else if(level==4){
            if((l4[l4_index] & 0x1)==0){
                u32 a=os_pfn_alloc(OS_PT_REG);
                l3=(u64*)osmap(a);
                for(u32 i=0;i<512;i++)
                    l3[i]=0;

                l4[l4_index] |= a<<12;
                l4[l4_index] |= 0x5;
            }
            else{
                l3=(u64*)osmap((u32)(l4[l4_index]>>12));
            }
            if((flag & 0x2) == 2)
                l4[l4_index] |= 0x2;
        }
    }
}


//checks if the virtual_add is present in any of the memory segment or not
int check_present(u64 virtual_add){
  struct exec_context *current = get_current_ctx();
  struct mm_segment m;
  u64 start,next_free,end;
  
  m = current->mms[MM_SEG_DATA];
  next_free = m.next_free;
  start = m.start;
 
  if(virtual_add<next_free&&virtual_add>=start){
    return 1; 
  }
  m = current->mms[MM_SEG_RODATA];
  next_free = m.next_free;
  start = m.start;
  if(virtual_add<next_free&&virtual_add>=start){
    return 2; 
  }
  m = current->mms[MM_SEG_STACK];
  end = m.end;
  start = m.start;
  if(virtual_add<=end&&virtual_add>=start){
    return 3; 
  }
  return -1;
}

extern u64 handle_page_fault(void)
{ 
  u64 virtual_add,error_code,RIP;
    //saving general purpose registers
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

    u64 save;

    asm volatile("movq 8(%%rbp), %0;" : "=r" (error_code));
    asm volatile("movq 16(%%rbp), %0;" : "=r" (RIP));
    asm volatile("movq %%cr2, %0;" : "=r" (virtual_add));
    asm volatile ("movq %%rsp, %0;" : "=r"(save));
    printf("values of RIP %x, virtual address %x, error code %x\n",RIP,virtual_add,error_code);
    
    //check protection violation
    if((error_code&0x1)==1){
        printf("error code : protecion violation at RIP: %x, virtual address: %x, error code: %x\n",RIP,virtual_add,error_code);
        printf("****************\n");
        do_exit();
        return 0;
    }
    struct exec_context *current = get_current_ctx();
    int region = check_present(virtual_add);

    if(region==-1){
        printf("out of region RIP %x, virtual address %x, error code %x\n",RIP,virtual_add,error_code);
        printf("****************\n");
        do_exit();
        return 0;
    }
    struct mm_segment m;
    if(region==1){
        m = current->mms[MM_SEG_DATA];
        printf("next_free %x\n",m.next_free);
        printf("allocating in MM_SEG_DATA\n");
        allocate(virtual_add,m);
    }
    else if(region==2){
        if((error_code&0x2)==2){
            printf("write error at RIP %x, virtual address %x, error code %x\n",RIP,virtual_add,error_code);
            printf("****************\n");
            do_exit();
            return 0;
        }
        printf("allocating in MM_SEG_RODATA\n");
        m = current->mms[MM_SEG_RODATA];
        allocate(virtual_add,m);
    }
    else{ 
        printf("allocating in MM_SEG_STACK\n");
        m = current->mms[MM_SEG_STACK];
        allocate(virtual_add,m);
    }
    //printf("page fault handler: unimplemented!\n");
    //error in iretq.
    printf("****************\n");
    asm volatile ("movq %0, %%rsp;"
                    :: "r" (save));
    //restoring general purpose registers
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
    asm volatile ("movq %rbp, %rsp;");
    asm volatile ("pop %rbp;");
    asm volatile ("add $8, %rsp;");
    asm volatile ("iretq");
    
    
    return 0;
}

