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

//����ؽṹ
struct shared_use_st  
{  
    int Index[20];    //20�������,Ϊ0��ʾ��Ӧ�Ļ�����δ��������ʹ�ã��ɷ��䵫�������ѣ�Ϊ1��ʾ��Ӧ�Ļ������ѱ�������ʹ�ã����ɷ��䵫������
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

//producer
void * producer(struct shared_use_st* a){
  struct shared_use_st *shared;
	shared = a;
  while(1)        //����һ��ѭ��
  {
	  if(sem_wait(&(shared->sem)) == -1)        //sem_waitΪP�����������ź�����ֵ
        {
            printf("P���� ERROR!\n");
            exit(EXIT_FAILURE);
        }
        int i;
        for(i = 0; i < 20 && shared->Index[i] == 1; i++)
        ;
        if(i == 20)        //IndexΪ1��ʾ����ر�������ռ��
        {  
            //������ռ䶼��������ռ��ʱ�����waiting...��
            sem_post(&shared->sem);        //sem_postΪV���������������ź�����ֵ
            sleep(1);        //sleepһ��ʱ�䣬�ٴν���ѭ��
            printf("PID:%ld, TID:%ld,Waiting...\n",getpid(),gettid());
        }
        else
        {
            sem_post(&shared->sem);        //V ���������ź���
               int num = rand() % 100 + 1;
               shared->Buffer[i%20] = num;        //��ȡstdin�ַ������BUFSIZ-1����������buffer������ ����stdin�Ǽ������뵽���������ַ�
               printf("PID:%ld, TID:%ld, produce:%d\n",getpid(),gettid(),num);
               //strncpy(shared->Buffer[i%20], buffer,TEXT_SZ);        //��ȡ���ַ������뻺����shared->Buffer��
               shared->Index[i%20] = 1;        //��ʾ�û�������������ʹ����
    			  //running=0
        }
        double x = 1,t;
        t = expntl(x);  ////xΪ��p 
        usleep(t);
   }
}


int main()
{
    int running = 1;
    int i = 0;  
    void *shm = NULL;        //����洢�����ӵ�ʵ�ʵ�ַ
    struct shared_use_st *shared = NULL;
    int buffer[BUFSIZ + 1];        //����������ַ�
    int shmid;        //�����ڴ��ʶ��
    //��û򴴽�һ�������ڴ��ʶ��
    shmid = shmget((key_t)1121, sizeof(struct shared_use_st), 0666|IPC_CREAT);
    if(shmid == -1)        //��ȡ�򴴽�һ�������ڴ��ʶ��ʧ��
    {   
      exit(EXIT_FAILURE);
    }
    shm = shmat(shmid, (void*)0, 0);        //���ع���洢�����ӵ�ʵ�ʵ�ַ
    if(shm == (void*)-1)
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
    pthread_t threadPool[producerNum];
   
    //���������߽��̷����̳߳�
	int j;
    for(j = 0; j < producerNum; j++){
        pthread_t temp;
        if(pthread_create(&temp, NULL, producer, shared) == -1){
            printf("ERROR, fail to create producer%d\n", j);
            exit(1);
        }
        threadPool[i] = temp;
    }
	//�����̳߳�
    void * result;
    for(j = 0; j < producerNum; j++){
        if(pthread_join(threadPool[j], &result) == -1){
            printf("fail to recollect\n");
            exit(1);
        }
    }
    
     //�������ڴ�ӵ�ǰ�����з���
    if(shmdt(shm) == -1)        //ʧ��
    {
        exit(EXIT_FAILURE);
    }
    /*�鿴���޸Ĺ����ڴ�ε�shmid_ds�ṹ��ɾ���ýṹ�Լ������Ĺ���洢�α�ʶ
    struct shmid_ds  
    {  
        uid_t shm_perm.uid;  
        uid_t shm_perm.gid;  
        mode_t shm_perm.mode;  
    };
    */
    if(shmctl(shmid, IPC_RMID, 0) == -1)        //ʧ��
    {
        exit(EXIT_FAILURE);
    }  
    exit(EXIT_SUCCESS);
}
