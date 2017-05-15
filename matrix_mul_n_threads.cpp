#include<iostream>
#include<fstream>
#include<string>
#include<sstream>
#include<pthread.h>
#include<stdlib.h>
#include<sys/time.h>
using namespace std;
int **A,**B,**C;
int r1,c1,r2,c2;
typedef struct data{
    int x;
    int y;
}index;
bool get_matrix( char ch){
    string name;
    if(ch == 'A'){
        name = "matrixA.txt";
    }
    else{
        name = "matrixB.txt";
    }
    ifstream myfile (name.c_str());
    string line;
    int M=0,N1,N2;
    if(myfile.is_open()){

        while(getline(myfile,line)){
            istringstream iss(line);
            string sub;
            int length=0;
            while(iss>>sub){
                length+=1;
            }
            if(M==0){
                N1=length;
                N2=N1;
            }
            else {
                N1=N2;
                N2=length;
                if(N1!=N2){
                    cout<<"The no. of elements in "<<M<<"th and "<<M+1<<"th rows aren't same in matrix "<<ch<<endl;
                    return false;
                }
            }
            M++;
        }
        if(ch == 'A'){
            r1=M;
            c1=N1;
            A= (int **)(malloc(sizeof(int *)*(M+1)));
            for(int i=0;i<M;i++){
                A[i] = (int *)(malloc(sizeof(int)*N1));
            }
        }
        else if(ch =='B'){
            r2 = M;
            c2=N1;
            B= (int **)(malloc(sizeof(int *)*(M+1)));
            for(int i=0;i<M;i++){
            B[i] = (int *)(malloc(sizeof(int)*N1));
        }
        }
        myfile.close();
    }
    else{
        cout<<"Sorry unable to open input file"<<endl;
        return false;
    }
    ifstream  myfile2 (name.c_str());
    if(myfile2.is_open()){
        M=0;
        while(getline(myfile2,line)){
            istringstream iss(line);
            string sub;
            N1=0;
            while(iss>>sub){
                int no = atoi(sub.c_str());
                if(ch =='A'){
                    A[M][N1] = no;
                }
                else if(ch == 'B'){
                    B[M][N1] = no;
                }
                N1++;
            }
            M+=1;
        }
        myfile2.close();
        return true;

    }
    else{
        cout<<"Sorry unable to open input file"<<endl;
        return false;
    }
}
void *thread_func(void* vals){
    index * val=((index *)vals);
    int i1 = val->x;
    int i2 = val->y;
    cout<<i1<<" "<<i2<<endl;
    for(int i=i1;i<i2;i++){
            for(int j=0;j<c2;j++){
               	C[i][j]=0;
                for(int k=0;k<c1;k++)
                    C[i][j]+=(A[i][k]*B[k][j]);
            }
    }
}
int main(){
    clock_t tstart=clock();
    if(!get_matrix('A') || !get_matrix('B')){
        return 0;
    }
    if(c1!=r2)  {
        cout<<"Sorry the no of columns of 1st matrix arenot equal to the no. of rows of 2nd matrix"<<endl;
        return 0;
    }
    int k,n;
    C=(int **)(malloc(sizeof(int *)*r1));
    for(k = 0;k < r1;k++){
        C[k] = (int *)(malloc(sizeof(int) * c2));
    }
    k=0;
    cout<<"Enter no of threads to be used"<<endl;
    cin>>n;
    index indices[n];
    pthread_t thread[n];
    k=(r1/n);
    for(int i=0;i<n;i++){
            indices[i].x = (i*k);
            if(i==n-1){
                indices[i].y = r1;
            }
            else{
                indices[i].y=(i+1)*k;
            }
        pthread_create(&thread[i],NULL,thread_func,(void *)(&indices[i]));
    }
     int i,j;
    for(i=0;i<n;i++){
        pthread_join(thread[i],NULL);
    }
	
    cout <<"Matrix A is :"<<endl;
    for(i=0;i<r1;i++){
        for(j=0;j<c1;j++){
            cout <<A[i][j]<<" ";
        }
        cout << endl;
    }
    cout <<endl<<"Matrix B is:"<<endl;
    for(i=0;i<r2;i++){
        for(j=0;j<c2;j++){
            cout <<B[i][j]<<" ";
        }
        cout<<endl;
    }
    cout<<endl<<"Matrix C is:"<<endl;
    for(i=0;i<r1;i++){
        for(j=0;j<c2;j++){
            cout<<C[i][j]<<" ";
        }
        cout<<endl;
    }
	clock_t z = (clock()-tstart)*1.0/CLOCKS_PER_SEC;	
    cout<<"Time in milliseconds is: "<<z<<endl;
    return 0;
}
