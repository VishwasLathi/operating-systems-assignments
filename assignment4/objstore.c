#include "lib.h"
#include <pthread.h>
#define malloc_4k(x,y) do{\
                         (x) = mmap(NULL, y, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);\
                         if((x) == MAP_FAILED)\
                              (x)=NULL;\
                     }while(0); 


#define free_4k(x,y) munmap((x), y)
                     
#define MAX_OBJS 1000000 
#define OBJ_SIZE 88                     
#define INODE_BLOCKS 22528
#define INODE_BITMAP_SIZE 31
#define DATA_BITMAP_SIZE 256
#define BLOCK_SIZE 4096
#define inode_block_offset 287
#define data_block_offset 22528+287
#define data_bitmap_offset 31
#define INODES_PER_BLOCK 46
#define disk_address_bits 32
#define data_offset  22528+287
#define cache_blks  32768                   
/*          
Returns the object ID.  -1 (invalid), 0, 1 - reserved
*/
//inode bitmap for dirty bitmap 
pthread_mutex_t d_lock, i_lock,cache_lock;

int* cache_index;
int* cache_dirty;

struct object{
     long id;
     long size;
     int cache_index;
     int dirty;
     int direct[4];
     int indirect[4];
     char key[32];
};

struct object* a;
struct object* backup;
unsigned int* inode_bitmap;
unsigned int* data_bitmap;
unsigned int* dirty_array;
char *cache_ptr;

int free_data_block(struct objfs_state *objfs){
	int ctr=0;
	unsigned int b = 1<<31;
    //lock
    	pthread_mutex_lock(&d_lock);

	for(int i=0;i<DATA_BITMAP_SIZE*BLOCK_SIZE/4;i++){
		b = 1<<31;
		for(int j=0;j<32;j++){
			if(!(data_bitmap[i]&b)){
				data_bitmap[i] = data_bitmap[i] | b;
        			pthread_mutex_unlock(&d_lock);
				return (data_block_offset + ctr);
			}
			b = b>>1;
			ctr++;
		}
	}
	
 	 pthread_mutex_unlock(&d_lock);
	// dprintf("can't find free block for writing\n");
	return -1;

}

long cache_blk_read(struct objfs_state *objfs,int blk,char *temp){
	//directly read from cache
	#ifdef CACHE
  	pthread_mutex_lock(&cache_lock);
	char* start = cache_ptr +  BLOCK_SIZE * (blk%cache_blks);

	if(cache_index[blk%cache_blks] == -1){
		cache_index[blk%cache_blks] = blk/cache_blks;
		read_block(objfs,blk,start);
	}

	else if(cache_index[blk%cache_blks] != blk/cache_blks){
		// read_block(objfs,blk,start);
		// dprintf("read cache hit\n");
		
		//check dirty of old blk
		if(cache_dirty[blk%cache_blks]==1){
			int old_blk = cache_blks*cache_index[blk%cache_blks] + blk%cache_blks;
			int new_blk = blk;

			//write the old block to disk
			write_block(objfs,old_blk,start);
			read_block(objfs,new_blk,start);
		}
		else{
			//no need to write back the old blk
			read_block(objfs,blk,start);
		}
		cache_index[blk%cache_blks] = blk/cache_blks;
	}
	for(int i=0;i<BLOCK_SIZE;i++)
			*(temp+i) = *(start+i);
    
   	pthread_mutex_unlock(&cache_lock);
   	return 0;

	#else
	read_block(objfs,blk,(char*)temp);
  	return 0;

	#endif	
	return 0;
}

long cache_blk_write(struct objfs_state *objfs,int blk,char *temp){
	#ifdef CACHE
  	pthread_mutex_lock(&cache_lock);
	char* start = cache_ptr + BLOCK_SIZE * (blk%cache_blks);

	if(cache_index[blk%cache_blks]==-1 || cache_index[blk%cache_blks] == blk/cache_blks){
		for(int i=0;i<BLOCK_SIZE;i++)
			*(start+i) = *(temp+i);	 
	}

	else if(cache_dirty[blk%cache_blks] == 1){
		//first write back the old block to the disk
		int old_blk = cache_blks*cache_index[blk%cache_blks] + blk%cache_blks;
		write_block(objfs,old_blk,start);
		for(int i=0;i<BLOCK_SIZE;i++)
			*(start+i) = *(temp+i);
	}
	else{
		for(int i=0;i<BLOCK_SIZE;i++)
			*(start+i) = *(temp+i);
	}
	cache_index[blk%cache_blks] = blk/cache_blks;
	cache_dirty[blk%cache_blks] = 1;
  	pthread_mutex_unlock(&cache_lock);
  	return 0;

	#else
	write_block(objfs,blk,temp);
    	return 0;
	#endif
	
	return 0;	
}


struct object* find_target(struct objfs_state *objfs,int objid,int set_dirty){
	// struct object* target = NULL;
	struct object* obj = a;
	unsigned int b = 1<<31;
  	int ctr=0;
	for(int i=0;i<INODE_BITMAP_SIZE*BLOCK_SIZE/4;i++){
		b = 1<<31;
		for(int j=0;j<32;j++){
			if((inode_bitmap[i]&b) && obj->id==objid){
        		if(set_dirty){
          			dirty_array[ctr/46] = 1;
        		}
        		// dprintf("object found by target\n");
				return obj;
			}
			   b = b>>1;
      		ctr++;
      		obj= a + ctr;
			}
		}

	
	// dprintf("couldn't find object for writing");
	return NULL;	

}


long find_object_id( const char *key,struct objfs_state *objfs)
{
	
	struct object* obj = a;
  	int ctr=0;
	unsigned int b = 1<<31;
	for(int i=0;i<INODE_BITMAP_SIZE*BLOCK_SIZE/4;i++){
		b = 1<<31;
		for(int j=0;j<32;j++){
			if((inode_bitmap[i]&b) && obj->id && !strcmp(obj->key,key)){
				backup = obj;
				// dprintf("found\n");
				// dprintf("returning objid %d\n",obj->id);
				return obj->id;
			}
			b = b>>1;
      		ctr++;
			obj = a + ctr;
		}
	}
	// dprintf("didn't found\n");
	return -1;
}

/*
  Creates a new object with obj.key=key. Object ID must be >=2.
  Must check for duplicates.

  Return value: Success --> object ID of the newly created object
                Failure --> -1
*/
//set id  +=2
long create_object(const char *key, struct objfs_state *objfs)
{

    struct object *free = NULL;
    struct object* obj = a;
    unsigned int b = 1<<31;
    int ctr=0;
    //lock
    pthread_mutex_lock(&i_lock);
    for(int i=0;i<INODE_BITMAP_SIZE*BLOCK_SIZE/4;i++){
    	b = 1<<31;
    	for(int j=0;j<32;j++){
    		if(!(b & inode_bitmap[i]) && !free){
    			inode_bitmap[i] = inode_bitmap[i] | b;
          //unlock
              
          		dirty_array[ctr/46] = 1;
    			// dprintf("found free object whith id %d, bit value %ud,i %d\n",ctr+2,inode_bitmap[i],i);
    			free = obj;
    			obj->id = ctr+2;
		        pthread_mutex_unlock(&i_lock);
          
    		}
    		b = b>>1;
    		ctr++;
    		obj = a + ctr;
    	}
    }	

    if(!free){
      pthread_mutex_unlock(&i_lock);
        // dprintf("%s: objstore full\n", __func__);
        return -1;
    } 
    strcpy(free->key, key);
    return free->id;	
}
/* 
  One of the users of the object has dropped a reference
  Can be useful to implement caching.
  Return value: Success --> 0
                Failure --> -1
*/
long release_object(int objid, struct objfs_state *objfs)
{
  //no use
    return 0;
}

/*
  Destroys an object with obj.key=key. Object ID is ensured to be >=2.

  Return value: Success --> 0
                Failure --> -1
*/
// long set_dirty(con)

long destroy_object(const char *key, struct objfs_state *objfs)
{

  struct object* target=NULL;
  int find=0;
  struct object* obj = a;
  int ctr=0;
  unsigned int b = 1<<31;

  unsigned bit_temp;
  int bit_index;

  pthread_mutex_lock(&i_lock);
  for(int i=0;i<INODE_BITMAP_SIZE*BLOCK_SIZE/4;i++){
    if(find)
      break;
    b = 1<<31;
    for(int j=0;j<32;j++){  
      if((inode_bitmap[i]&b) && obj->id && !strcmp(obj->key,key)){
        target = obj;
        inode_bitmap[i]=  inode_bitmap[i]^b;
        pthread_mutex_unlock(&i_lock);
        dirty_array[ctr/46] = 1;
        find  = 1;
        break;
      }
      b = b>>1;
      ctr++;
      obj = a + ctr;
    }
  }
   
  // if(!target)
  // 	return -1;
    // dprintf("didn't found object\n");
  target->id = 0;
  target->size = 0;

 // pthread_mutex_lock(&d_lock);
  for(int i=0;i<4;i++){
    int dir_add = target->direct[i];
    if(dir_add){
      int bit_pos = dir_add - data_block_offset;
      int idx = bit_pos/32;
      //check this line
      if(cache_index[dir_add%cache_blks] == dir_add/cache_blks){
      	cache_index[dir_add%cache_blks] = -1;
      	cache_dirty[dir_add%cache_blks] = 0;
      }
      data_bitmap[idx] = data_bitmap[idx] ^ (1<<(31-bit_pos%32));
    }
  }

  
  for(int i=0;i<4;i++){
    int indir_add = target->indirect[i];

    int bit_pos1 = indir_add - data_block_offset;
    int idx1 = bit_pos1/32;
    data_bitmap[idx1] = data_bitmap[idx1] ^ (1<<(31-bit_pos1%32));
  
    if(cache_index[indir_add%cache_blks] == indir_add/cache_blks){
      	cache_index[indir_add%cache_blks] = -1;
      	cache_dirty[indir_add%cache_blks] = 0;
      }

    if(indir_add){
      int* indir_block;
      malloc_4k(indir_block,BLOCK_SIZE);
      read_block(objfs,indir_add,(char*)indir_block);
      for(int i=0;i<1024;i++){
        if(indir_block[i]){
          int bit_pos = indir_block[i] - data_block_offset;
          int idx = bit_pos/32;
          //check this line
          data_bitmap[idx] = data_bitmap[idx] ^ (1<<(31-bit_pos%32));
          if(cache_index[indir_block[i]%cache_blks] == indir_block[i]/cache_blks){
	      	cache_index[indir_block[i]%cache_blks] = -1;
	      	cache_dirty[indir_block[i]%cache_blks] = 0;
	      }
                    // data_bitmap[idx] = data_bitmap[idx] ^ (1<<(31-bit_pos%32));
        }
      }
      free_4k(indir_block,BLOCK_SIZE);
    }
  }

  // pthread_mutex_unlock(&d_lock);
  for(int i=0;i<4;i++){
    target->direct[i]=0;
    target->indirect[i]=0;
  } 
    return 0;
}

/*
  Renames a new object with obj.key=key. Object ID must be >=2.
  Must check for duplicates.  
  Return value: Success --> object ID of the newly created object
                Failure --> -1
*/

long rename_object(const char *key, const char *newname, struct objfs_state *objfs)
{
 
 	int ctr;
    struct object * obj = a;
    struct object * actual;
    int find=0;
    unsigned int b= 1<<31;
    for(int i=0;i<INODE_BITMAP_SIZE*BLOCK_SIZE/4;i++){
    	b = 1<<31;
    	for(int j=0;j<32;j++){
    		if( (inode_bitmap[i]&b) && !find && !strcmp(obj->key,key)){
          	 dirty_array[ctr/46] = 1;
    			   actual = obj;
    			   strcpy(obj->key,newname);
    			   find = 1;
    		}
    		else if((inode_bitmap[i]&b)&&!strcmp(obj->key,key)){
    			return -1;
    		}
        	b = b>>1;
    		ctr++;
    		obj = a + ctr ;
    	}
    }
    return actual->id;
}


// OLD WRITE
long objstore_write(int objid, const char *buff, int size, struct objfs_state *objfs,off_t offset)
{
  struct object* target = find_target( objfs,objid,1);
  if(target==NULL)
  	return -1;
  int a = offset/BLOCK_SIZE;
  int data_1 = free_data_block(objfs);

  char* temp;
  malloc_4k(temp,size);
  for(int i=0;i<size;i++)
    *(temp+i) = *(buff+i);


  //if single direct pointer.
    //MAJOR CHANGE CONFIRM IF IT's BLOCK SIZE OR NOT.
  // target->size = target->size + BLOCK_SIZE;
  target->size = target->size + size;
  if(a<=3){
    // dprintf("direct data block %d\n",a);
    cache_blk_write(objfs,data_1,(char*)temp);
    target->direct[a] = data_1; 
    // dprintf("target id %d\n",target->id);
  }
  //if uses indirect pointer.
  else{
    
    //find first free data block from the data bitmap and set it's bit to 1.
    int data_2 ;
    int* indirect_block;
    malloc_4k(indirect_block,BLOCK_SIZE);

    if(a>=4 && a<1028){
      // dprintf("indirect data block 1\n");  
      if(a==4){
        data_2 = free_data_block(objfs);
        // dprintf("creating first indirect block %d, data block %d\n",data_1,data_2);
        //need to write the block address of the current allocated block.
        indirect_block[0] = data_2;
        target->indirect[0]= data_1;
        cache_blk_write(objfs,data_1,(char*)indirect_block);
        cache_blk_write(objfs,data_2,temp);
      }
      else{
        // dprintf("first indirect block %d, data block %d\n",target->indirect[0],data_1);
        cache_blk_read(objfs,target->indirect[0],(char*)indirect_block);
        // int block_add = indirect_block[a-4];
        indirect_block[a-4] = data_1;
        cache_blk_write(objfs,target->indirect[0],(char*)indirect_block);
        cache_blk_write(objfs,data_1,temp);
      }
      
    }
    else if(a>=1028&&a<2052){
      // dprintf("indirect data block 2\n");
      if(a==1028){
        data_2 = free_data_block(objfs);  
        indirect_block[0] = data_2;
        target->indirect[1]= data_1;
        cache_blk_write(objfs,data_1,(char*)indirect_block);
        cache_blk_write(objfs,data_2,temp);
      }
      else{
        cache_blk_read(objfs,target->indirect[1],(char*)indirect_block);
        indirect_block[a-1028] = data_1;
        cache_blk_write(objfs,target->indirect[1],(char*)indirect_block);
        cache_blk_write(objfs,data_1,temp);
      }
    
    }
    else if(a>=2052 && a< 3076){
      // dprintf("indirect data block 3\n");
        if(a==2052){
        //need to write the block address of the current allocated block.
        data_2 = free_data_block(objfs);  
        indirect_block[0] = data_2;
        target->indirect[2]= data_1;
        cache_blk_write(objfs,data_1,(char*)indirect_block);
        cache_blk_write(objfs,data_2,temp);
      }
      else{
        cache_blk_read(objfs,target->indirect[2],(char*)indirect_block);
        indirect_block[a-2052] = data_1;
        cache_blk_write(objfs,target->indirect[2],(char*)indirect_block);
        cache_blk_write(objfs,data_1,temp);
      }
    
    }
    else if(a>=3076 && a<4100){
      // dprintf("indirect data block 4\n");
        if(a==3076){
        data_2 = free_data_block(objfs);    
        //need to write the block address of the current allocated block.
        indirect_block[0] = data_2;
        target->indirect[3]= data_1;
        cache_blk_write(objfs,data_1,(char*)indirect_block);
        cache_blk_write(objfs,data_2,temp);
      }
      else{
        cache_blk_read(objfs,target->indirect[3],(char*)indirect_block);
        indirect_block[a-3076] = data_1;
        cache_blk_write(objfs,target->indirect[3],(char*)indirect_block);
        cache_blk_write(objfs,data_1,temp);
      }
        
    }
    free_4k(indirect_block,BLOCK_SIZE);

  }
  free_4k(temp,size);  
  return size;

  
}

//size and offset are 4K alinged.
//bytes left to read.
//DIRECTLY READ INTO THE BUFFER , DON'T MAKE ANY TEMPORARY ARRAY.
long objstore_read(int objid, char *buff, int size, struct objfs_state *objfs,off_t offset )
{

  // dprintf("offset in read %d\n",offset);
  int curr = (long)offset;
  struct object* target = find_target(objfs,objid,0);

  int blocks=(size+BLOCK_SIZE-1)/BLOCK_SIZE;
  char* temp;
  malloc_4k(temp,blocks*BLOCK_SIZE);
  int temp_off=0;

	while(blocks){
		int a = curr/BLOCK_SIZE;
		if(a<=3){
      // dprintf("reading from direct data %d\n",a);
			cache_blk_read(objfs,target->direct[a],(char*)(temp+temp_off));
		}
		else{
			int* indirect_block;
			malloc_4k(indirect_block,BLOCK_SIZE);
      int block_add;

			if(a>=4 && a<1028){
				cache_blk_read(objfs,target->indirect[0],(char*)indirect_block);
				block_add = indirect_block[a-4];
        // dprintf("reading from indirect data 1 %d, direct data %d\n",target->indirect[0],indirect_block[a-4]);
				cache_blk_read(objfs,block_add,(char*)(temp+temp_off));
			}
			else if(a>=1028&&a<2052){
        // dprintf("reading from indirect data 2\n");
				cache_blk_read(objfs,target->indirect[1],(char*)indirect_block);
				block_add = indirect_block[a-1028];
				cache_blk_read(objfs,block_add,(char*)(temp+temp_off));
			}
			else if(a>=2052 && a< 3076){
        // dprintf("reading from indirect data 3\n");
				cache_blk_read(objfs,target->indirect[2],(char*)indirect_block);
				block_add = indirect_block[a-2052];
				cache_blk_read(objfs,block_add,(char*)(temp+temp_off));
			}
			else if(a>=3076 && a<4100){
        // dprintf("reading from indirect data 4\n");
				cache_blk_read(objfs,target->indirect[3],(char*)indirect_block);
				block_add = indirect_block[a-3076];
				cache_blk_read(objfs,block_add,(char*)(temp+temp_off));	
			}
	      else{
	        // dprintf("file limit exceeded\n");
	        break;
	      }
      		free_4k(indirect_block,BLOCK_SIZE);
		}
    blocks--;
    curr += BLOCK_SIZE;
    temp_off += BLOCK_SIZE;
  }
 
    for(int i=0;i<size;i++){
      *(buff+i) = *(temp+i);
    }
    free_4k(temp,blocks*BLOCK_SIZE);
    // dprintf("temp count %d, buff count %d\n",count1,count2);
    return size;
}

/*
  Reads the object metadata for obj->id = buf->st_ino
  Fillup buf->st_size and buf->st_blocks correctly
  See man 2 stat 
*/
int fillup_size_details(struct stat *buf,struct objfs_state *objfs)
{
	// // dprintf("entered fillup_size_details\n");
   struct object *obj = backup;
   if(buf->st_ino < 2 || obj->id != buf->st_ino)
       return -1;
  //st_size is the total size in bytes 
   buf->st_size = obj->size;
   buf->st_blocks = obj->size >> 9;
   if(((obj->size >> 9) << 9) != obj->size)
       buf->st_blocks++;
   return 0;	
   // return -1;
}

/*
   Set your private pointeri, anyway you like.
*/
int objstore_init(struct objfs_state *objfs)
{
	// dprintf("reached here 0\n");
	malloc_4k(a,INODE_BLOCKS*BLOCK_SIZE);
	malloc_4k(inode_bitmap,INODE_BITMAP_SIZE*BLOCK_SIZE);
	malloc_4k(data_bitmap,DATA_BITMAP_SIZE*BLOCK_SIZE);
  	malloc_4k(dirty_array,INODE_BLOCKS*4);
	malloc_4k(backup,BLOCK_SIZE);
	malloc_4k(cache_index,4*cache_blks);
	malloc_4k(cache_dirty,4*cache_blks);

	cache_ptr = objfs->cache;

	for(int i=0;i<cache_blks;i++)
		cache_index[i]=-1;
	for(int i=0;i<cache_blks;i++)
		cache_dirty[i]=0;	

  	pthread_mutex_init(&d_lock,NULL);
  	pthread_mutex_init(&i_lock,NULL);
        pthread_mutex_init(&cache_lock,NULL);

	// // dprintf("reached here 1\n");
	for(int i=0;i<INODE_BITMAP_SIZE;i++){
		read_block(objfs,i,(char*)((char*)inode_bitmap + i*BLOCK_SIZE));
	}
	// // dprintf("reached here 2\n");
	for(int i=0;i<DATA_BITMAP_SIZE;i++){
		read_block(objfs,i+data_bitmap_offset,(char*)((char*)data_bitmap + i*BLOCK_SIZE));
	}
	// // dprintf("reached here 3\n");
	char* temp;
	malloc_4k(temp,BLOCK_SIZE);

	char* b = (char*)a;
	for(int i=0;i<INODE_BLOCKS;i++){
    	dirty_array[i] = 0;
		read_block(objfs,i+inode_block_offset,temp);
    	for(int k=0;k<4048;k++)
    		*(b+k+i*4048) = *(temp+k);
	}

	free_4k(temp,BLOCK_SIZE);
   // // dprintf("Done objstore init\n");

   return 0;
}

/*
   Cleanup private data. FS is being unmounted
*/
int objstore_destroy(struct objfs_state *objfs)
{
   // // dprintf("starting objstore destroy\n");

   for(int i=0;i<INODE_BITMAP_SIZE;i++){
   	write_block(objfs,i,(char*)((char*)inode_bitmap+i*BLOCK_SIZE));
   }
   for(int i=0;i<DATA_BITMAP_SIZE;i++){
   	write_block(objfs,data_bitmap_offset+i,(char*)((char*)data_bitmap+i*BLOCK_SIZE));
   }

   char* temp;
   malloc_4k(temp,BLOCK_SIZE);

   char* b = (char*)a;

   for(int i=0;i<INODE_BLOCKS;i++){
    if(dirty_array[i]){
      // // dprintf("dirty index %d\n",i);	
      for(int j=0;j<4096;j++)
      	*(temp+j) = *(b+j+i*4048);	
   	  write_block(objfs,inode_block_offset+i,temp);
    }
   }

   for(int i=0;i<cache_blks;i++){
   	if(cache_dirty[i]==1){
   		char* start = cache_ptr + BLOCK_SIZE*i;
   		int blk = cache_blks*cache_index[i] + i;
   		write_block(objfs,blk,start);
   	}
   }

   // // dprintf("Done write back");
   free_4k(inode_bitmap,INODE_BITMAP_SIZE*BLOCK_SIZE);
   free_4k(data_bitmap,BLOCK_SIZE*DATA_BITMAP_SIZE);
   free_4k(a,INODE_BLOCKS*BLOCK_SIZE);
   free_4k(dirty_array,4*INODE_BLOCKS);
   free_4k(temp,BLOCK_SIZE);
   free_4k(backup,BLOCK_SIZE);
   free_4k(cache_dirty,cache_blks*4);
   free_4k(cache_index,cache_blks*4);

   return 0;
}
