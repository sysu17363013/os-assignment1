#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#define N 5

//�ź���ʹ�õĲ���
sem_t chopsticks[N];
sem_t r;
int philosophers[N] = {0, 1, 2, 3, 4};

//swapָ����Ҫ�Ĳ���
int islocked[N] = {0};

//������ʹ�õĲ���
pthread_mutex_t chops[N];

//�ӳٺ���
void delay (int len) {
    int i = rand() % len;
    int x;
    while (i > 0) {
        x = rand() % len;
        while (x > 0) {
            x--;
        }
        i--;
    }
}

//����������ĿǰΪ����bug
void xchg(int *x, int *y) {
    __asm__("xchgl %0, %1" : "=r" (*x) : "m" (*y));
}

//�������ʹ�õĽ���취����������ĸ���ѧ�����������
void philosopher (void* arg) {
    int i = *(int *)arg;
    int left = i;
    int right = (i + 1) % N;
    int leftkey;
    int rightkey;
    while (1) {
        leftkey = 1;
        rightkey = 1;


        printf("philosopher %d is thinking\n", i);
        delay(50000);

        printf("philosopher %d is hungry\n", i);
        sem_wait(&r);

        //sem_wait(&chopsticks[left]);
        //pthread_mutex_lock(&chopsticks[left]);
        do {
            xchg(&leftkey, &islocked[left]);
        }while (leftkey);
        printf("philosopher %d pick up no.%d chopstick,now he only has one chopstick,he can't eat now\n", i, left);

        //sem_wait(&chopsticks[right]);
        //pthread_mutex_lock(&chopsticks[right]);
        do {
            xchg(&rightkey, &islocked[right]);
        }while (rightkey);
        printf("philosopher %d pick up no.%d chopstick,now he two chopsticks,he starts to eat\n", i, right);
        delay(50000);

        //sem_post(&chopsticks[left]);
        //pthread_mutex_unlock(&chopsticks[left]);
        islocked[left] = 0;
        printf("philosopher %d put down no.%d chopstick\n", i, left);

        //sem_post(&chopsticks[right]);
        //pthread_mutex_unlock(&chopsticks[right]);
        islocked[right] = 0;
        printf("philosopher %d put down no.%d chopstick\n", i, right);

        sem_post(&r);
    }
}

//�������ʹ�õĽ���취����������ѧ����������������ҿ��ӣ���ż������ѧ���෴��
void philosopher2 (void* arg) {
    int i = *(int *)arg;
    int left = i;
    int right = (i + 1) % N;
    while (1) {
        printf("��ѧ��%d����˼������\n", i);
        delay(50000);

        printf("��ѧ��%d����\n", i);
        if (i % 2 == 0) {//ż����ѧ�ң����Һ���
            sem_wait(&chopsticks[right]);
            printf("��ѧ��%d������%d�ſ���,����ֻ��һ֧����,���ܽ���\n", i, right);
            sem_wait(&chopsticks[left]);
            printf("��ѧ��%d������%d�ſ���, ��������֧����,��ʼ����\n", i, left);
            delay(50000);
            sem_post(&chopsticks[left]);
            printf("��ѧ��%d������%d�ſ���\n", i, left);
            sem_post(&chopsticks[right]);
            printf("��ѧ��%d������%d�ſ���\n", i, right);
        } else {//������ѧ�ң��������
            sem_wait(&chopsticks[left]);
            printf("��ѧ��%d������%d�ſ���, ��������֧����,��ʼ����\n", i, left);
            sem_wait(&chopsticks[right]);
            printf("��ѧ��%d������%d�ſ���,����ֻ��һ֧����,���ܽ���\n", i, right);
            delay(50000);
            sem_post(&chopsticks[right]);
            printf("��ѧ��%d������%d�ſ���\n", i, right);
            sem_post(&chopsticks[left]);
            printf("��ѧ��%d������%d�ſ���\n", i, left);
        }
    }
}

int main (int argc, char **argv) {
    srand(time(NULL));
    pthread_t PHD[N];


    for (int i=0; i<N; i++) {
        sem_init(&chopsticks[i], 0, 1);
    }
    sem_init(&r, 0, 4);

    for (int i=0; i<N; i++) {
        pthread_mutex_init(&chops[i], NULL);
    }

    for (int i=0; i<N; i++) {
        pthread_create(&PHD[i], NULL, (void*)philosopher, &philosophers[i]);
    }
    for (int i=0; i<N; i++) {
        pthread_join(PHD[i], NULL);
    }

    for (int i=0; i<N; i++) {
        sem_destroy(&chopsticks[i]);
    }
    sem_destroy(&r);
    for (int i=0; i<N; i++) {
        pthread_mutex_destroy(&chops[i]);
    }
    return 0;
}