#include<iostream>
#include<pthread.h>
#include<semaphore.h>
#include<unistd.h>
#define BUFF_SIZE 10
using namespace std;
sem_t empty,full;
pthread_mutex_t mutex;
void *producer(void *args){
	int *val =new int;
	val=((int *)args);
	int z=0;
	while(z<10){
		sem_wait(&empty);
		pthread_mutex_lock(&mutex);
		cout<<"Producer "<< (*val)+1<<" produced"<<endl;
		pthread_mutex_unlock(&mutex);
		sem_post(&full);
		z++;
	}
}

void *consumer(void *args){
	int *val=new int;
	val = ((int *)args);
	int z=0;
	while(z<10){
		sem_wait(&full);
		pthread_mutex_lock(&mutex);
		cout<<"Consumer"<<(*val)+1<<" Consumed"<<endl;
		pthread_mutex_unlock(&mutex);
		sem_post(&empty);
		z++;
	}
}

int max(int a, int b){
	return (a>b)?a:b;
}
int main(){
	sem_init(&full,0,0);
	sem_init(&empty,0,BUFF_SIZE);
	pthread_mutex_init(&mutex,NULL);
	
	int P,C;
	cout<<"Enter no. of producers";
	cin>>P;
	cout<<"Enter no. of consumers";
	cin>>C;
	pthread_t producers[P];
	pthread_t consumers[C];
	int z =max(P,C);
	int arr[z];
	for(int i=0;i<z;i++)	arr[i]=i;
	for(int i=0;i<P;i++){
		pthread_create(&producers[i],NULL,producer,(void *)(&arr[i]));
	}
	for(int i=0;i<C;i++){
		pthread_create(&consumers[i],NULL,consumer,(void *)(&arr[i]));	
	}
	for(int i=0;i<P;i++){ pthread_join(producers[i],NULL);}
	for(int i=0;i<C;i++){ pthread_join(consumers[i],NULL);}
	sem_destroy(&empty);
	sem_destroy(&full);
	pthread_mutex_destroy(&mutex);
	return 0;

}
