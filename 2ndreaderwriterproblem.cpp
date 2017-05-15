#include<pthread.h>
#include<semaphore.h>
#include<iostream>
#include<string>
#include<stdlib.h>
using namespace std;
int readcount=0,writecount=0,val=0;
sem_t rmutex,wmutex,wrt,readTry;
pthread_t *readers;
pthread_t *writers;
pthread_attr_t *rattr;
pthread_attr_t *wattr;
void *readerfn(void *args){
    sem_wait(&readTry);
    sem_wait(&rmutex);
    readcount++;
    if(readcount==1){
        sem_wait(&wrt);
    }
    sem_post(&rmutex);
    sem_post(&readTry);
    cout<<"Reader"<<readcount<<"read the data:"<<val<<endl;
    sem_wait(&rmutex);
    readcount--;
    if(readcount==0){
        sem_post(&wrt);
    }
    sem_post(&rmutex);
    pthread_exit(0);

}
void *writerfn(void *args){
    sem_wait(&wmutex);
    writecount++;
    if(writecount == 1){
        sem_wait(&readTry);
    }
    sem_post(&wmutex);
    sem_wait(&wrt);
    cout<<"Writer"<<writecount<<"is writing"<<endl;
    cout<<"Enter data to be written"<<endl;
    cin>>val;
    //Do Writing here
    sem_post(&wrt);
    sem_wait(&wmutex);
    writecount--;
    if(writecount==0){
        sem_post(&readTry);
    }
    sem_post(&wmutex);
    pthread_exit(0);
}
int main(){
    int reader,writer;
    sem_init(&rmutex,0,1);
    sem_init(&wmutex,0,1);
    sem_init(&wrt,0,1);
    sem_init(&readTry,0,1);
    cout<<"Enter no. of readers"<<endl;
    cin>>reader;
    cout<<"Enter no. of writers"<<endl;
    cin>>writer;
    readers = (pthread_t *)(malloc(sizeof(pthread_t)*reader));
    rattr = (pthread_attr_t *)(malloc(sizeof(pthread_attr_t)*reader));
    writers = (pthread_t *)(malloc(sizeof(pthread_t)*writer));
    wattr = (pthread_attr_t *)(malloc(sizeof(pthread_attr_t)*writer));
    string order;
    cout<<"Specify order of reader and writer processes coming in:(R for reader and W for writer)"<<endl;
    cin>>order;
    int rs=0,ws=0;
    for(int i=0;i<order.length();i++){
        if(order[i]==' ')   continue;
        else if(order[i] == 'R' || order[i] == 'r')  rs+=1;
        else if(order[i] == 'W' || order[i] == 'w')  ws+=1;
        else{
            cout<<"Sorry string is invalid"<<endl;
            pthread_exit(NULL);
            return 0;
        }
    }
    if(rs!=reader || ws != writer){
        cout<<"The no of readers and writers don't match as specified earlier"<<endl;
        pthread_exit(NULL);
        return 0;
    }
    for(int i=0;i<reader;i++){
        pthread_attr_init(&rattr[i]);
    }
    for(int i=0;i<writer;i++){
        pthread_attr_init(&wattr[i]);
    }
	int k=0,j=0;
    int arr[200];
    for(int i=0;i<200;i++) arr[i]=i;
    for(int i=0;i<order.length();i++){
	if(order[i]==' ')	continue;
	else if(order[i] == 'R' || order[i] == 'r'){
        cout<<"Reader"<<++k<<"created."<<endl;
        pthread_create(&readers[k-1],&rattr[k-1],readerfn,(void *)(&arr[k]));
	}
	else if(order[i]=='W' || order[i]=='w'){
		cout<<"Writer"<<j+1<<"created."<<endl;
        	pthread_create(&writers[j],&wattr[j],writerfn,(void *)(&arr[j+1]));
		j++;
	}
    }
    k=0,j=0;
    for(int i=0;i<order.length();i++){
        if(order[i]==' ')   continue;
        else if(order[i] == 'R' || order[i]=='r'){
            pthread_join(readers[k++],NULL);
        }
        else if(order[i] == 'W' || order[i] == 'w'){
            pthread_join(writers[j++],NULL);
        }
    }
    sem_destroy(&rmutex);
    sem_destroy(&wmutex);
    sem_destroy(&wrt);
    sem_destroy(&readTry);
    return 0;
}
