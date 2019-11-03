#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/syscall.h>
#include <semaphore.h>
#include <fcntl.h>
#define TEXT_SZ 1024
#define producerNum 3
#define gettid() syscall(__NR_gettid)

//缓冲池结构
struct shared_use_st  
{  
    int Index[20];    //20个缓冲池,为0表示对应的缓冲区未被生产者使用，可分配但不可消费；为1表示对应的缓冲区已被生产者使用，不可分配但可消费
    int Buffer[20];    //20个字符串缓冲区
    sem_t sem;        //信号量，同步功能
};

double expntl(double x)
{ /*负指数分布随机数*/
  double z;
  do{
  z=((double)rand()/RAND_MAX);
  }while((z == 0)||(z == 1));
  return(-x*log(z));
}

//producer
void * producer(struct shared_use_st* a){
  struct shared_use_st *shared;
	shared = a;
  while(1)        //制造一个循环
  {
	  if(sem_wait(&(shared->sem)) == -1)        //sem_wait为P操作，减少信号量的值
        {
            printf("P操作 ERROR!\n");
            exit(EXIT_FAILURE);
        }
        int i;
        for(i = 0; i < 20 && shared->Index[i] == 1; i++)
        ;
        if(i == 20)        //Index为1表示缓冲池被消费者占用
        {  
            //当五个空间都被消费者占用时输出“waiting...”
            sem_post(&shared->sem);        //sem_post为V操作，用来增加信号量的值
            sleep(1);        //sleep一段时间，再次进入循环
            printf("PID:%ld, TID:%ld,Waiting...\n",getpid(),gettid());
        }
        else
        {
            sem_post(&shared->sem);        //V 操作增加信号量
               int num = rand() % 100 + 1;
               shared->Buffer[i%20] = num;        //读取stdin字符流最多BUFSIZ-1个，并存在buffer数组中 其中stdin是键盘输入到缓冲区的字符
               printf("PID:%ld, TID:%ld, produce:%d\n",getpid(),gettid(),num);
               //strncpy(shared->Buffer[i%20], buffer,TEXT_SZ);        //读取的字符串存入缓冲区shared->Buffer中
               shared->Index[i%20] = 1;        //表示该缓冲区被生产者使用了
    			  //running=0
        }
        double x = 1,t;
        t = expntl(x);  ////x为λp 
        usleep(t);
   }
}


int main()
{
    int running = 1;
    int i = 0;  
    void *shm = NULL;        //共享存储段连接的实际地址
    struct shared_use_st *shared = NULL;
    int buffer[BUFSIZ + 1];        //缓冲区存放字符
    int shmid;        //共享内存标识符
    //获得或创建一个共享内存标识符
    shmid = shmget((key_t)1121, sizeof(struct shared_use_st), 0666|IPC_CREAT);
    if(shmid == -1)        //获取或创建一个共享内存标识符失败
    {   
      exit(EXIT_FAILURE);
    }
    shm = shmat(shmid, (void*)0, 0);        //返回共享存储段连接的实际地址
    if(shm == (void*)-1)
    {
        exit(EXIT_FAILURE);
    }
    printf("Memory attached at %ld\n", (intptr_t)shm);
    shared = (struct shared_use_st*)shm;        //缓冲池为共享存储段连接地址
    for( ; i < 20; i++ )
    {
        shared->Index[i] = 0;        //对缓冲池初始化，Index为0表示可以生产
    }
    sem_init(&(shared->sem),1,1);        //信号量化初始化，且信号量初始值为第二个1
    i = 0;
    pthread_t threadPool[producerNum];
   
    //创建生产者进程放入线程池
	int j;
    for(j = 0; j < producerNum; j++){
        pthread_t temp;
        if(pthread_create(&temp, NULL, producer, shared) == -1){
            printf("ERROR, fail to create producer%d\n", j);
            exit(1);
        }
        threadPool[i] = temp;
    }
	//运行线程池
    void * result;
    for(j = 0; j < producerNum; j++){
        if(pthread_join(threadPool[j], &result) == -1){
            printf("fail to recollect\n");
            exit(1);
        }
    }
    
     //将共享内存从当前进程中分离
    if(shmdt(shm) == -1)        //失败
    {
        exit(EXIT_FAILURE);
    }
    /*查看及修改共享内存段的shmid_ds结构，删除该结构以及相连的共享存储段标识
    struct shmid_ds  
    {  
        uid_t shm_perm.uid;  
        uid_t shm_perm.gid;  
        mode_t shm_perm.mode;  
    };
    */
    if(shmctl(shmid, IPC_RMID, 0) == -1)        //失败
    {
        exit(EXIT_FAILURE);
    }  
    exit(EXIT_SUCCESS);
}
