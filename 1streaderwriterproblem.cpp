#include<pthread.h>
#include<semaphore.h>
#include<iostream>
#include<string>
#include<stdlib.h>
using namespace std;
int readcount=0,val=0;
sem_t mutex,wrt;
pthread_t *readers;
pthread_t *writers;
pthread_attr_t *rattr;
pthread_attr_t *wattr;
void *readerfn( void * arg){
    sleep(1);
    int temp = *((int *)arg);
    sem_wait(&mutex);
    readcount++;
    if(readcount==1){
        sem_wait(&wrt);
    }
    sem_post(&mutex);
    cout<<"Reader"<<temp<<"read the data:"<<val<<endl;
    sem_wait(&mutex);
    readcount--;
    if(readcount==0){
        sem_post(&wrt);
    }
    sem_post(&mutex);
    pthread_exit(0);

}
void *writerfn(void *arg){
    sleep(1);
    int temp =*((int *)arg);
    sem_wait(&wrt);
    cout<<"Writer "<<temp<<" is writing"<<endl;
    cout<<"Enter data to be written"<<endl;
    scanf("%d",&val);
    //Do Writing here
    sem_post(&wrt);
    pthread_exit(0);
}
int main(){
    int reader,writer;
    sem_init(&wrt,0,1);
    sem_init(&mutex,0,1);
    cout<<"Enter no. of readers"<<endl;
    cin>>reader;
    cout<<"Enter no. of writers"<<endl;
    cin>>writer;
    readers = (pthread_t *)(malloc(sizeof(pthread_t)*reader));
    rattr = (pthread_attr_t *)(malloc(sizeof(pthread_attr_t)*reader));
    writers = (pthread_t *)(malloc(sizeof(pthread_t)*writer));
    wattr = (pthread_attr_t *)(malloc(sizeof(pthread_attr_t)*writer));
    for(int i=0;i<reader;i++){
        pthread_attr_init(&rattr[i]);
    }
    for(int i=0;i<writer;i++){
        pthread_attr_init(&wattr[i]);
    }
    for(int i=0;i<reader;i++){
        cout<<"Reader"<<i+1<<"created."<<endl;
        pthread_create(&readers[i],&rattr[i],readerfn,(void *)i);
    }
    for(int i=0;i<writer;i++){
        cout<<"Writer"<<i+1<<"created."<<endl;
        pthread_create(&writers[i],&wattr[i],writerfn,(void *)i);
    }
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
    int k=0,j=0;
    for(int i=0;i<order.length();i++){
        if(order[i]==' ')   continue;
        else if(order[i] == 'R' || order[i]=='r'){
            pthread_join(readers[k++],NULL);
        }
        else if(order[i] == 'W' || order[i] == 'w'){
            pthread_join(writers[j++],NULL);
        }
    }
    sem_destroy(&mutex);
    sem_destroy(&wrt);
    return 0;
}
