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

//����ؽṹ
struct shared_use_st  
{  
    int Index[20];    //20�������,Ϊ0��ʾ��Ӧ�Ļ�����δ��������ʹ�ã��ɷ��䵫�������ѣ�Ϊ1��ʾ��Ӧ�Ļ�������������ʹ�ã����ɷ��䵫������
    int Buffer[20];    //20���ַ���������
    sem_t sem;        //�ź�����ͬ������
};

double expntl(double x)
{ /*��ָ���ֲ������*/
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
  	if(sem_wait(&(shared->sem)) == -1)        //sem_waitΪP�����������ź�����ֵ
        {
            printf("P���� ERROR!\n");
            exit(EXIT_FAILURE);
        }
        int i;
        for(i = 0; i < 20 && shared->Index[i] == 0; i++)
           ;
           //��ʮ����������û�ж���������ռ��
        if(i != 20)
        {   
            printf("PID:%ld, TID:%ld, consume: %d\n",getpid(),gettid(),shared->Buffer[i%20]);        //��ӡ��������д����ַ�
            shared->Index[i%20] = 0;        //Ϊ0ʱ����ʾ�ѱ�������ʹ��
            sem_post(&shared->sem);        //sem_postΪV����
            sleep(1); 
        }
        //��ʮ�����ռ䶼��ռ�ã����waiting...  
          else
          {
             sem_post(&shared->sem);        //V����
             sleep(1);  
            printf("PID:%ld, TID:%ld,Waiting...\n",getpid(),gettid());
         }
        double x = 1,t;  //xΪ��c 
        t = expntl(x);
        usleep(t);
    }
   }
    
int main()
{
    int i = 0; 
    void *shm = NULL;        //����洢�����ӵ�ʵ�ʵ�ַ
    struct shared_use_st *shared = NULL;    //�����
    int shmid;        //���������ڴ��ʶ��
    
    shmid = shmget((key_t)1121, sizeof(struct shared_use_st), 0666|IPC_CREAT); //��û򴴽�һ�������ڴ��ʶ��
    if(shmid == -1)        //��ȡ�򴴽�һ�������ڴ��ʶ��ʧ��
    {  
        exit(EXIT_FAILURE);
    }
    
    //�������ڴ�����ӵ�һ�����̵ĵ�ַ�ռ��У�����void *ָ��
    shm = shmat(shmid, 0, 0);    //���ع���洢�����ӵ�ʵ�ʵ�ַ    
    if(shm == (void*)-1)        //ʧ��
    {
        exit(EXIT_FAILURE);
    }
    printf("Memory attached at %ld\n", (intptr_t)shm);
    shared = (struct shared_use_st*)shm;        //�����Ϊ����洢�����ӵ�ַ
    
    for( ; i < 20; i++ )
    {
        shared->Index[i] = 0;        //�Ի���س�ʼ����IndexΪ0��ʾ��������
    }
    sem_init(&(shared->sem),1,1);        //�ź�������ʼ�������ź�����ʼֵΪ�ڶ���1
    i = 0;
    
    //���������߽��̷����̳߳�
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
    
    //�����̳߳�
    void * result;
    for(j = 0; j < consumerNum; j++){
        if(pthread_join(threadPool[j], &result) == -1){
            printf("fail to recollect\n");
            exit(1);
        }
    }

    //�������ڴ�ӵ�ǰ�����з���
    if(shmdt(shm) == -1)        //����ʧ��
    {
        exit(EXIT_FAILURE);
    }
    if(shmctl(shmid, IPC_RMID, 0) == -1)    //ʧ��
    {
        exit(EXIT_FAILURE);
    }  
    exit(EXIT_SUCCESS);
}
