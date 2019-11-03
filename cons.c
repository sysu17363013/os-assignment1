#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <fcntl.h>
#define TEXT_SZ 1024
#define consumerNum 3
#define gettid() syscall(__NR_gettid)

//缓冲池结构
struct shared_use_st  
{  
    int Index[20];    //20个缓冲池,为0表示对应的缓冲区未被生产者使用，可分配但不可消费；为1表示对应的缓冲区被生产者使用，不可分配但可消费
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

//consumer
void * consumer(struct shared_use_st* b){
  struct shared_use_st *shared;
	shared = b;
  while(1)
  {
  	if(sem_wait(&(shared->sem)) == -1)        //sem_wait为P操作，减少信号量的值
        {
            printf("P操作 ERROR!\n");
            exit(EXIT_FAILURE);
        }
        int i;
        for(i = 0; i < 20 && shared->Index[i] == 0; i++)
           ;
           //二十个个缓冲区没有都被生产者占用
        if(i != 20)
        {   
            printf("PID:%ld, TID:%ld, consume: %d\n",getpid(),gettid(),shared->Buffer[i%20]);        //打印出生产者写入的字符
            shared->Index[i%20] = 0;        //为0时，表示已被消费者使用
            sem_post(&shared->sem);        //sem_post为V操作
            sleep(1); 
        }
        //二十个个空间都被占用，输出waiting...  
          else
          {
             sem_post(&shared->sem);        //V操作
             sleep(1);  
            printf("PID:%ld, TID:%ld,Waiting...\n",getpid(),gettid());
         }
        double x = 1,t;  //x为λc 
        t = expntl(x);
        usleep(t);
    }
   }
    
int main()
{
    int i = 0; 
    void *shm = NULL;        //共享存储段连接的实际地址
    struct shared_use_st *shared = NULL;    //缓冲池
    int shmid;        //声明共享内存标识符
    
    shmid = shmget((key_t)1121, sizeof(struct shared_use_st), 0666|IPC_CREAT); //获得或创建一个共享内存标识符
    if(shmid == -1)        //获取或创建一个共享内存标识符失败
    {  
        exit(EXIT_FAILURE);
    }
    
    //将共享内存段连接到一个进程的地址空间中，返回void *指针
    shm = shmat(shmid, 0, 0);    //返回共享存储段连接的实际地址    
    if(shm == (void*)-1)        //失败
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
    
    //创建消费者进程放入线程池
	  pthread_t threadPool[consumerNum];
    int j;
    for(j = 0; j < consumerNum; j++){
        pthread_t temp;
        if(pthread_create(&temp, NULL, consumer, shared) == -1){
            printf("ERROR, fail to create consumer%d\n", j);
            exit(1);
        }
        threadPool[i] = temp;
    }
    
    //运行线程池
    void * result;
    for(j = 0; j < consumerNum; j++){
        if(pthread_join(threadPool[j], &result) == -1){
            printf("fail to recollect\n");
            exit(1);
        }
    }

    //将共享内存从当前进程中分离
    if(shmdt(shm) == -1)        //分离失败
    {
        exit(EXIT_FAILURE);
    }
    if(shmctl(shmid, IPC_RMID, 0) == -1)    //失败
    {
        exit(EXIT_FAILURE);
    }  
    exit(EXIT_SUCCESS);
}
