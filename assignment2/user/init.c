#include<init.h>
static void exit(int);
static int main(void);
#include<memory.h>

void init_start()
{
  int retval = main();
  exit(0);
}

/*Invoke system call with no additional arguments*/
static u64 _syscall0(int syscall_num)
{
  asm volatile ( "int $0x80;"
                 "leaveq;"
                 "retq;"
                :::"memory");
  return 0;   /*gcc shutup!*/
}

/*Invoke system call with one argument*/

static u64 _syscall1(int syscall_num, int exit_code)
{
  asm volatile ( "int $0x80;"
                 "leaveq;"
                 "retq;"
                :::"memory");
  return 0;   /*gcc shutup!*/
}

//for writing
static u64 _syscall2(int syscall_num, char* buff, int length)
{
  asm volatile ( "int $0x80;"
                 "leaveq;"
                 "retq;"
                :::"memory");
  return 0;   /*gcc shutup!*/
}

//for expanding
static u64 _syscall3(int syscall_num, u32 size, int flags)
{
  asm volatile ( "int $0x80;"
                 "leaveq;"
                 "retq;"
                :::"memory");
  return 0;   /*gcc shutup!*/
}

//for shrinking
static u64 _syscall4(int syscall_num, u32 size, int flags)
{
  asm volatile ( "int $0x80;"
                 "leaveq;"
                 "retq;"
                :::"memory");
  return 0;   /*gcc shutup!*/
}

static void exit(int code)
{
  _syscall1(SYSCALL_EXIT, code); 
}

static u64 getpid()
{
  return(_syscall0(SYSCALL_GETPID));
}
static u64 write(char *buff,int length){
  return _syscall2(SYSCALL_WRITE,buff,length);
}

static u64 expand(u32 size,int flags){
  return _syscall3(SYSCALL_EXPAND,size,flags);
}
static u64 shrink(u32 size,int flags){
  return _syscall3(SYSCALL_SHRINK,size,flags);
}

static int main()
{
   void *ptr1;
  char *ptr = (char *) expand(8, MAP_WR);
  
  if(ptr == NULL)
    write("FAILED\n", 7);
  
  *(ptr + 8192) = 'A';   /*Page fault will occur and handled successfully*/
  ptr1 = (char *) shrink(7, MAP_WR);
  *ptr = 'A';  /*Page fault will occur and handled successfully*/     
  
  *(ptr + 4096) = 'A';   /*Page fault will occur and PF handler should termminate 
                 the process (gemOS shell should be back) by printing an error message*/
  exit(0);
  return 0;

  /*
  write("aaaaaaa",1024);
  write("bbbbbbb",7);
  unsigned long i;
  
  int a = 2/0;
  int b=2/0;
  
#if 0
  unsigned long *ptr = (unsigned long *)0x100032;
  i = *ptr;

#endif
  i = getpid();
  exit(-5);
  return 0;
  */
}
