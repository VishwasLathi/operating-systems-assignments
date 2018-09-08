#include<context.h>
#include<memory.h>
#include<lib.h>

void allocate(char c,u64 *l4,struct mm_segment m,u32 arg_pfn){
    u64 virutal_add;
    u32 flag = m.access_flags;
    if(c=='c'||c=='d')
        virutal_add=m.start;
    else
        virutal_add=m.end-0x1000;
   
    l1_index = (virutal_add)>>12 & 0x1FF;
    l2_index = (virutal_add)>>21 & 0x1FF;
    l3_index=(virutal_add)>>30 & 0x1FF;
    l4_index=(virutal_add)>>39;
    
    u64 *l2,*l3,*l1;
    
    for(int level=4;level>=1;level--){
        if(level==1){
            if((l1[l1_index] & 0x1)==1){
               return;
            }
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
                for(u64 i=0;i<512;i++)
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
                for(u64 i=0;i<512;i++)
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

void prepare_context_mm(struct exec_context *ctx)
{
	u32 a=os_pfn_alloc(OS_PT_REG);
	u64* l4=(u64*)osmap(a);
	u32 arg_pfn=ctx->arg_pfn;
	ctx->pgd=a;
	for(u64 i=0;i<512;i++)
	    l4[i]=0;
        
    	allocate('c',l4,ctx->mms[MM_SEG_CODE],arg_pfn); 
    	allocate('d',l4,ctx->mms[MM_SEG_DATA],arg_pfn);   
    	allocate('s',l4,ctx->mms[MM_SEG_STACK],arg_pfn); 
	
	
   	return;
}
void cleanup_context_mm(struct exec_context *ctx)
{
       
   return;
}

