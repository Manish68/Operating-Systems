#include<stdio.h>
#include<pthread.h>
#include<semaphore.h>
#include<stdlib.h>
#include<unistd.h>
#define BUFFER_SIZE 4
typedef int buffer_item;

buffer_item buffer[BUFFER_SIZE];
pthread_mutex_t mutex;
sem_t empty,full;
int counter;

pthread_t *producers;
pthread_t *consumers;

void initialize(){
    pthread_mutex_init(&mutex,NULL);
	sem_init(&full,1,0);
	sem_init(&empty,1,BUFFER_SIZE);
	counter=0;
}
/*int insert_item(buffer_item item){
    sem_wait(&empty);
    pthread_mutex_lock(&mutex);
    printf("Producer ");
}

int remove_item(buffer_item *item){

}*/
void * consumer(void * arg){
    buffer_item item;
    while(1){
        sleep(1);
        sem_wait(&full);
        pthread_mutex_lock(&mutex);
        item = buffer[--counter];
        printf("Consumer has consumed the item: %d \n",item);
        pthread_mutex_unlock(&mutex);
        sem_post(&empty);
    }
}

void *producer(void *arg){
    buffer_item item;
    while(1){
        sleep(1);
        printf("Enter the element you wish to produce:");
        scanf("%d",&item);
        sem_wait(&empty);
        pthread_mutex_lock(&mutex);
        printf("Producer has produced the item:%d\n",item);
        buffer[counter++] = item;
        pthread_mutex_unlock(&mutex);
        sem_post(&full);
    }
}
int main(){
    initialize();
    producers = (pthread_t *)(malloc(sizeof(pthread_t)));
    consumers = (pthread_t *)(malloc(sizeof(pthread_t)));
    pthread_create(producers,NULL,producer,NULL);
    pthread_create(consumers,NULL,consumer,NULL);
    pthread_join(*producers,NULL);
    pthread_join(*consumers,NULL);
    pthread_mutex_destroy(&mutex);
    sem_destroy(&empty);
    sem_destroy(&full);

}
