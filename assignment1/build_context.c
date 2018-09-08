#include<context.h>
#include<memory.h>
#include<lib.h>


u64 createMask(u64 a, u64 b)
{
   u64 r = 0;
   for (u64 i=a; i<=b; i++)
       r |= 1 << i;
   return r;
}

u32 bitExtracted(u64 number, int k, int p)
{
    return (u32)((((1 << k) - 1) & (number >> (p - 1))));
}

void allocate(char c,u64 *l4,struct mm_segment m,u32 arg_pfn){
    u64 virutal_add;
    //printf("*******\n");
    u32 flag = m.access_flags;
    //printf("type %c\n",c);
    if(c=='c'||c=='d')
        virutal_add=m.start;
    else
        virutal_add=m.end-0x1000;
    u32 l1_index, l2_index, l3_index,l4_index;
    
    l1_index = (virutal_add)>>12 & 0x1FF;
    l2_index = (virutal_add)>>21 & 0x1FF;
    l3_index=(virutal_add)>>30 & 0x1FF;
    l4_index=(virutal_add)>>39;
    
/*
    u32 l4_index=virutal_add & createMask(39,47);
    u32 l3_index=virutal_add & createMask(30,38);
    u32 l2_index=virutal_add & createMask(21,29);
    u32 l1_index=virutal_add & createMask(12,20);
    u32 offset=virutal_add & createMask(0,11);*/
    
    /*
    u32 l1_index = bitExtracted(virutal_add,13,9);
    u32 l2_index = bitExtracted(virutal_add,22,9);
    u32 l3_index = bitExtracted(virutal_add,31,9);
    u32 l4_index = bitExtracted(virutal_add,40,9);
    u32 offset = bitExtracted(virutal_add,1,12);
    */

    //u1 contain the virtual address of L1 page table.
    u64 *l2,*l3,*l1;
    
    for(int level=4;level>=1;level--){
       // printf("curr_level %d\n",level);
        if(level==1){
            if((l1[l1_index] & 0x1)==1){
               // printf("Error: memory already occupied");
                return;    
            }
            //printf("allocating memory\n");
            if(c=='d'){
                l1[l1_index]|=arg_pfn<<12;
                l1[l1_index]|=0x5;  
            }
            else{
                u32 a=os_pfn_alloc(USER_REG);
                l1[l1_index] |= a<<12;
                l1[l1_index] |=0x5;   
            }
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
                //printf("invalid l2\n");
            }
            else{
                l1=(u64*)osmap((u32)(l2[l2_index]>>12));
                //printf("valid l2\n");
            }
            if((flag & 0x2) == 2)
                l2[l2_index] |=0x2;
        }
        else if(level==3){
            if((l3[l3_index] & 0x1)==0){
                //printf("invalid l3\n");
                u32 a=os_pfn_alloc(OS_PT_REG);
                l2=(u64*)osmap(a);
                for(u32 i=0;i<512;i++)
                    l2[i]=0;
                l3[l3_index]|= a<<12;
                l3[l3_index]|= 0x5;
            }
            else{
                //printf("valid l3\n");
                //l2=(u64*)osmap(l3[l3_index]>>12);
                l2=(u64*)osmap((u32)(l3[l3_index]>>12));     
            }
            if((flag & 0x2) == 2)
               l3[l3_index] |= 0x2;     
                
        }
        else if(level==4){
            if((l4[l4_index] & 0x1)==0){
                //printf("invalid l4\n");
                u32 a=os_pfn_alloc(OS_PT_REG);
                l3=(u64*)osmap(a);
                for(u32 i=0;i<512;i++)
                    l3[i]=0;
                //set the 12-51 bits of l4[l4_index] by the frame no of the aligned address of next page.
                l4[l4_index] |= a<<12;
                l4[l4_index] |= 0x5;
            }
            else{
               // printf("valid l4\n");
                //printf("%d %lld l4 valid 1\n",l4_index,l4[l4_index]);
                //l3=(u64*)osmap(l4[l4_index]>>12);
                l3=(u64*)osmap((u32)(l4[l4_index]>>12));
                //case when the entry is already valid.
            }
            if((flag & 0x2) == 2)
                l4[l4_index] |= 0x2;
        }
    }
    // printf("*******\n");

}

void prepare_context_mm(struct exec_context *ctx)
{
  u32 a=os_pfn_alloc(OS_PT_REG);
  u64* l4=(u64*)osmap(a);
  u32 arg_pfn=ctx->arg_pfn;
  ctx->pgd=a;
  for(u64 i=0;i<512;i++)
      l4[i]=0;
  //allocates memory to stack, code and heap iteratively.
        
  allocate('c',l4,ctx->mms[MM_SEG_CODE],arg_pfn); 
    allocate('d',l4,ctx->mms[MM_SEG_DATA],arg_pfn);   
    allocate('s',l4,ctx->mms[MM_SEG_STACK],arg_pfn); 
  
  
    return;
}
void cleanup_context_mm(struct exec_context *ctx)
{
       
   return;
}
