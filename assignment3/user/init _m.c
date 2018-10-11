#include<init.h>
#include<memory.h>
static void exit(int);
static int main(void);
void init_start()
{
  int retval = main();
  exit(0);
}

/*Invoke system call with no additional arguments*/
static long _syscall0(int syscall_num)
{
  asm volatile ( "int $0x80;"
                 "leaveq;"
                 "retq;"
                :::"memory");
  return 0;   /*gcc shutup!*/
}

/*Invoke system call with one argument*/

static long _syscall1(int syscall_num, int exit_code)
{
  asm volatile ( "int $0x80;"
                 "leaveq;"
                 "retq;"
                :::"memory");
  return 0;   /*gcc shutup!*/
}
/*Invoke system call with two arguments*/

static long _syscall2(int syscall_num, u64 arg1, u64 arg2)
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

static long getpid()
{
  return(_syscall0(SYSCALL_GETPID));
}

static long write(char *ptr, int size)
{
   return(_syscall2(SYSCALL_WRITE, (u64)ptr, size));
}

static void signal(u64 signum, void* addr)
{
  _syscall2(SYSCALL_SIGNAL, signum,(long)addr) ;
}
static void alarm(u32 num)
{
  _syscall1(SYSCALL_ALARM, num) ;
}
static void sleep(u32 ticks)
{
  _syscall1(SYSCALL_SLEEP, ticks) ;
}
static void clone(void* func_addr,void* st_addr)
{
  _syscall2(SYSCALL_CLONE, (long)func_addr,(long)st_addr) ;
}
static char* expand(u64 size,u64 flags)
{
  return (char*)_syscall2(SYSCALL_EXPAND,size,flags);
}

static void fun(){
	write("Hello Hello\n",15);
    exit(0);
}
static void fu(){
	write("Hll\n",15);
//    exit(0);
}
static int main()
{
    write("aaaaaa\n",12);
    sleep(10);
    write("aaaaaa\n",12);
    sleep(5);
    write("dddd\n",10);
  signal(SIGALRM,&fu);
    alarm(5);
    while(1);
//  exit(0);
  
  //char *ptr = (char *)expand(20, MAP_WR);
  //clone(&fun,ptr);
  //write("a\n",2);
  //return 0;
  /*unsigned long i, j;
  unsigned long buff[4096];
  i = getpid();

  for(i=0; i<4096; ++i){
      j = buff[i];
  }
  i=0x100034;
  j = i / (i-0x100034);
  exit(-5);*/
}
