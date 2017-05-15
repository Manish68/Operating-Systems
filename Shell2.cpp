#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<stdarg.h>
#include<sys/types.h>
#include<vector>
#include<iostream>
#include<sys/wait.h>
#include<string>
#include<setjmp.h>
#include<unistd.h>
#include<csignal>
#define MAX_ARGS 100
#define BUFSIZE 1024
#define HISTORY_COUNT 200
#define TRY do{ jmp_buf ex_buf__; if( !setjmp(ex_buf__) ){
#define CATCH } else {
#define ETRY } }while(0)
#define THROW longjmp(ex_buf__, 1)
using namespace std;
typedef struct job{
    int pid;
    bool running;
    string cmd;
}job;
int lasttoberun=-1;
int ltostoporinbg=-1;
vector<string> history;
vector<job> jobs;

pid_t fgProcess = 0;
int toBg = 0;
int pipe_ints[2];
void cdCmd(vector<string> tokens);
void helpcmd(vector<string> tokens);
void exit(vector<string> tokens);
void historyc(vector<string> tokens);
void pwd(vector<string> tokens);

char *builtin_str[] = {
  "cd",
  "help",
  "exit",
  "history",
  "pwd"
};

void (*builtin_func[]) (vector<string>) = {
  &cdCmd,
  &helpcmd,
  &exit,
  &historyc,
  &pwd
};
int no_builtins(){
    return sizeof(builtin_str)/sizeof(char *);
}

void sigint_handler(int s) {
    if (fgProcess != 0)
        kill(fgProcess, SIGKILL);
    fgProcess = 0;
    return;
}
void sigtstp_handler(int s) {
    if (fgProcess != 0)
        kill(fgProcess, SIGSTOP);
    toBg = 1;
    return;
}
void read_line(char **cmd){
    int c,position=0;
    int bufsize=BUFSIZE;
    while(1){
        c=getchar();
        if(c==EOF || c=='\n'){
            (*cmd)[position]='\0';
            return;
        }
        else{
            (*cmd)[position]=c;
        }
        position++;

        if(position>=bufsize){
            bufsize+=BUFSIZE;
            (*cmd)=(char *)realloc((*cmd),bufsize);
            if(!(*cmd)){
                fprintf(stderr,"Couldn't allocate the specified memory");
                exit(EXIT_FAILURE);
            }
        }
    }

}
vector<string> splitatpipe(char *cmd){
    vector<string> cmds;
    string token="";
    for(int i=0;i<strlen(cmd);i++){
        if(cmd[i]!='|'){
            token=token + cmd[i];
        }
        else{
            while(cmd[i+1]=='|' && i<strlen(cmd)){
                i++;
            }
            if(token.length()>0){
                cmds.push_back(token);
                token.clear();
            }
        }
    }
    if(token.length()>0){
            cmds.push_back(token);
            token.clear();
    }
    return cmds;
}
vector<string> parse(string cmd){
    //can do realloc here as well
    char *pch;
    char **argv2=(char **)malloc(sizeof(char *)*MAX_ARGS);
    char delimit[]=" \t\r\n\v\f";
    char *cmdd = strdup(cmd.c_str());
    pch=strtok(cmdd,delimit);
    int i=0;
    while(pch!=NULL){
        argv2[i]=(char *)malloc(sizeof(char)*(strlen(pch)+1));
        strcpy(argv2[i],pch);
        i++;
        pch=strtok(NULL,delimit);
    }
    argv2[i]=NULL;
    vector<string> arguments;
    for(int j=0;j<i;j++){
        arguments.push_back(argv2[j]);
    }
    return arguments;
}
void shellprompt(){
    char *currentdirectory=(char *)malloc(1024*sizeof(char));
    char *hostname=(char *)malloc(sizeof(char)*1024);
    gethostname(hostname,1024);
    printf("%s@%s : %s###",getenv("LOGNAME"),hostname,getcwd(currentdirectory,1024));
}
void cdCmd(vector<string> tokens){
    if(tokens.size()==1){
        cout<<"Sorry only 1 arguments passed"<<endl;
        return;
    }
    else if(tokens.size()>2){
        cout<<"Sorry no. of arguments is too many"<<endl;
        return;
    }
    else{
        /*TRY{
            int status = chdir(tokens[1].c_str());
            if(status!=0)   THROW;
        }
        CATCH{
            cout<<"Invalid address specified"<<endl;
            return;
        }*/
	if(chdir(tokens[1].c_str())!=0){
		cout<<"Invalid Address Specified"<<endl;
	}
        history.push_back(tokens[0]+" "+tokens[1]);
        return;
    }
}
void helpcmd(vector<string> tokens){
    history.push_back("help" +tokens[0]);
    if(tokens.size()>1){
            cout<<"Help command takes no arguments"<<endl;
        }
    else{
    	cout<<"Help command here"<<endl;		
    }
	return;

}
void historyc(vector<string> tokens){
    history.push_back("history");
    for(int i=0;i<history.size();i++){
        cout<<i+1<<")   "<<history[i]<<endl;
    }
    return;
}
void exit(vector<string> tokens){
    if(tokens.size()>1){
            cout<<"Exit command doesn't take any arguments"<<endl;
    }
    else{
    history.push_back("exit");
    if (jobs.size() == 0)
        exit(EXIT_SUCCESS);
    else
        cout << "Some background processes exist. You need to kill the background processes to terminate." << endl;
    return;
    }
}
void pwd(vector<string> tokens){//Can also use GetCurrentDirectory()
    if(tokens.size()>1){
        cout<<"Sorry this command doesn't take any argument"<<endl;
    }
    else{
        char *cwd = (char *)malloc(sizeof(char)*1024);
        if(getcwd(cwd,sizeof(cwd))!=NULL ){
            fprintf(stdout,"Current Working Directory :%s\n",cwd);
        }
        else{
            perror("Sorry couldn't fetch pwd ");
        }
        return;
    }
}

void launch(vector<string>tokens,int pipe){
    FILE *inputFile;
    FILE *outputFile;
    int inputflag=0,outputflag=0;
    string s ="";
    for(int i=0;i<tokens.size();i++){
        if(tokens[i]==">" || tokens[i] == ">>"){
            outputflag = i;
        }
        if(tokens[i]=="<"){
            inputflag = i;
        }
        s+=tokens[i];
        s+=" ";
    }
    if (pipe == -1 || pipe == 0)
        history.push_back(s);
    if (pipe == 1)
        history[history.size() - 1].append("| " + s);
    if (inputflag) {
        inputFile = fopen(tokens[inputflag + 1].c_str(), "r");
        if (!inputFile) {
            cout << "Invalid Input File." << endl;
            return;
        }
        //dup2(fileno(inFile), 0);
    }
    if (outputflag) {
        if (tokens[outputflag] == ">")
            outputFile = fopen(tokens[outputflag + 1].c_str(), "w");
        else if (tokens[outputflag] == ">>")
            outputFile = fopen(tokens[outputflag + 1].c_str(), "a");
        //dup2(fileno(outFile), 1);
    }
    if (inputflag || outputflag) {
        if (inputflag) {
            while (tokens.size() > inputflag)
                tokens.pop_back();
        } else {
            while (tokens.size() > outputflag)
                tokens.pop_back();
        }
    }
    int flag = 0;
    string last_param = tokens[tokens.size() - 1];
    int last_param_len = last_param.length() - 1;
    if (last_param[last_param_len] == '&') {
        tokens[tokens.size() - 1] = last_param.substr(0, last_param_len);
        flag = 1;
    }
    // Important: Vector conversion to array of strings
    const char **argv = new const char* [tokens.size() + 1];
    for (int j = 0; j < tokens.size(); j++)
        argv[j] = tokens[j].c_str();
    argv[tokens.size()] = NULL;
    int status;
    pid_t pid = fork();
    if (pid == 0) {
        if (inputflag)
            dup2(fileno(inputFile), 0);
        if (outputflag)
            dup2(fileno(outputFile), 1);
        if (pipe == 0) {
            close(1);
            close(pipe_ints[0]);
            dup2(pipe_ints[1], 1);
        } else if (pipe == 1) {
            close(0);
            close(pipe_ints[1]);
            dup2(pipe_ints[0], 0);
        }
        execvp(argv[0], (char**) argv);
        cout << "Invalid Command." << endl;
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        perror("Shell Error: Error forking process.");
    } else {
        //do {
        fgProcess = pid;
	pid_t w_pid;
        if (!flag)
            do {
                w_pid = waitpid(pid, &status, WNOHANG);
                signal(SIGINT, sigint_handler);
                signal(SIGTSTP, sigtstp_handler);
                if (toBg) {
                    job instance;
                    instance.pid = pid;
                    instance.running = 0;
                    instance.cmd = s;
                    jobs.push_back(instance);
                    toBg = 0;
                    fgProcess = 0;
                    cout << "Job added to the background and stopped." << endl;
                    break;
                }
            } while (w_pid == 0);
        else {
            pid_t w_pid = waitpid(pid, &status, WNOHANG);
            if (w_pid == 0) {
                job instance;
                instance.pid = pid;
                instance.running = 1;
                instance.cmd = s.substr(0, s.length() - 2);
                jobs.push_back(instance);
            }
        }
        //fgProcess = pid;
        //} while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }
    if (inputflag)
        fclose(inputFile);
    if (outputflag)
        fclose(outputFile);
    return;
}
void execute(vector<string> tokens, int pipe){
    if(tokens.size()==0 || tokens[0]==""){
        cout<<"Sorry you didn't provide any command"<<endl;
    }
	int a=0;
    for(int i=0;i<no_builtins();i++){
        if(strcmp(tokens[0].c_str(),builtin_str[i])==0){
            (*builtin_func[i])(tokens);
	    a=1;
        }
    }
    /*if(tokens[0] == "cd"){
		cdCmd(tokens);
}
else if(tokens[0] == "history") historyc(tokens);
else if(tokens[0] == "pwd")  pwd(tokens);
else if (tokens[0] == "help" ) 	helpcmd(tokens);
else if (tokens[0] == "exit") 	exit(tokens);*/
	if(a==1)	return;
    else if(tokens[0][0]=='!'){
        if(tokens.size()> 1 ){
            cout<<"Sorry no. of arguments required is 1"<<endl;
        }
        else{
            string number = tokens[0].substr(1);
            int n = atoi(number.c_str());
            if(abs(n)> history.size()){
                cout<<"Sorry the specified no. of commands haven't been issued so far"<<endl;
            }
            else if(n<0){
                cout<<"Command is: "<<history[history.size()+n]<<endl;
                vector<string> cmds = parse(history[history.size()+n]);
                execute(cmds,pipe);
            }
            else if(n>0){
                cout<<"Command is : "<<history[n-1]<<endl;
                vector<string> cmds = parse(history[n-1]);
                execute(cmds,pipe);
            }
            else if(n==0){
                cout<<"Sorry this is invalid"<<endl;
            }
        }
    }
    else if(tokens[0]=="jobs"){
        history.push_back("jobs");
        if (jobs.size() == 0) {
            cout << "There are no current jobs." << endl;
            return;
        }
        for (int i = 0; i < jobs.size(); i++)
            if (jobs[i].running){
            if(i==ltostoporinbg)
                cout<<"["<<i+1<<"]+"<<jobs[i].pid <<" Running: "<< jobs[i].cmd << endl;
            else if(i== lasttoberun)
                cout<<"["<<i+1<<"]-"<<jobs[i].pid <<" Running: "<< jobs[i].cmd << endl;
            else
                cout<<"["<<i+1<<"] "<<jobs[i].pid <<" Running: "<< jobs[i].cmd << endl;
            }
            else{
               if(i==ltostoporinbg)
                cout<<"["<<i+1<<"]+"<<jobs[i].pid <<" Stopped: "<< jobs[i].cmd << endl;
            else if(i== lasttoberun)
                cout<<"["<<i+1<<"]-"<<jobs[i].pid <<" Stopped: "<< jobs[i].cmd << endl;
            else
                cout<<"["<<i+1<<"] "<<jobs[i].pid <<" Stopped: "<< jobs[i].cmd << endl;
            }
    }
    else if(tokens[0] == "kill"){
        string s ="";
        for(int i=0;i<tokens.size();i++){
            s+=tokens[i];
        }
        history.push_back(s);
        if(tokens.size()< 2){
            cout<<"Sorry please specify which process to kill"<<endl;
            return;
        }
        else if(tokens.size()> 2){
            cout<<"Excessive arguments to kill"<<endl;
            return;
        }
        else{
            if(tokens[1][0] == '%'){
                string number = tokens[1].substr(1);
                int n;
                /*TRY{n = atoi(number.c_str());}
                CATCH{cout<<"The specified number is not correct"<<endl;
                    return;}*/
		n=atoi(number.c_str());
                if(abs(n) > jobs.size()){
                    cout<<"Sorry the no. of background processes is :"<<jobs.size()<<endl;
                    return;
                }
                else if(n<0){
                    cout<<"Invalid job number"<<endl;
                    return;
                }
                kill (jobs[n-1].pid,SIGKILL);
                for(int i=0;i<jobs.size();i++){
                    if( i == n-1){
                        if(i== lasttoberun){
                            lasttoberun=ltostoporinbg;
                        }
                        else if(i==ltostoporinbg){
                            ltostoporinbg = lasttoberun;
                        }
                        jobs.erase(jobs.begin()+i);
                        cout<<"Process numbered"<<n<<"has been killed"<<endl;
                    }
                }
            }
            else{
                int n,a=0;
                /*TRY{    n = atoi(tokens[1].c_str());    }
                CATCH{  cout<<"The specified number is not correct"<<endl;
                    return; }*/
		n=atoi(tokens[1].c_str());
                for(int i=0;i<jobs.size();i++){
                    if(jobs[i].pid == n){
                        kill(n,SIGKILL);
                        jobs.erase(jobs.begin()+i);
                        cout<<"Process with pid "<<n<<"has been killed"<<endl;
                        a=1;
                        break;
                    }
            }
            if(a==0){
                cout<<"Sorry the given process pid isnot present to kill"<<endl;
            }
            return;
        }
    }
    }
    else if(tokens[0]=="bg"){
        string s ="";
        for(int i=0;i<tokens.size();i++){
            s+=tokens[i];
        }
        history.push_back(s);
        if(tokens.size()>2 || tokens.size()<2){
            cout<<"Sorry specify a single 1 argument"<<endl;
            return;
        }
        else{
            if(tokens[1][0]== '%'){
                int n;
                /*TRY{    n = atoi(tokens[1].substr(1).c_str());    }
                CATCH{  cout<<"The specified number is not correct"<<endl;
                    return; }*/
		n=atoi(tokens[1].substr(1).c_str());
                if(n > jobs.size()){
                    cout<<"Sorry the number is not correct"<<endl;
                    return;
                }
                ltostoporinbg = lasttoberun;
                lasttoberun = n-1;
                kill(jobs[n - 1].pid, SIGCONT);
                if(!jobs[n-1].running){
                    jobs[n-1].running=true;
                    cout<<"["<<n<<"]+ "<<s<<" &"<<endl;
                }
                else{
                    cout<<"Job number "<<n<<"already running in background"<<endl;
                }
            }
        }
    }
    else if(tokens[0] == "fg"){
        string s ="";
        for(int i=0;i<tokens.size();i++){
            s+=tokens[i];
        }
        history.push_back(s);
        if(tokens.size()>2 || tokens.size()<2){
            cout<<"Sorry specify a single 1 argument"<<endl;
            return;
        }
        else{
            if(tokens[1][0]=='%'){
                string s = tokens[1].substr(1);
                pid_t pid,wid;
                int no;
                if(s.length()==0 || s=="+"){
                    pid = jobs[ltostoporinbg].pid;
                    no=ltostoporinbg+1;
                }
                else if(s == "-"){
                    pid = jobs[lasttoberun].pid;
                    no=lasttoberun+1;
                }
                else{
                    no = atoi(s.c_str());
                    if(no > jobs.size()){
                        cout<<"Sorry, invalid number"<<endl;
                        return;
                    }
                    pid = jobs[no-1].pid;
                }
               if(!jobs[no-1].running){
                    kill(pid,SIGCONT);
                    jobs.erase(jobs.begin()+no-1);
               }
               int status;
               do{
                    wid = waitpid(pid,&status,WCONTINUED);
                    signal(SIGINT,sigint_handler);
                    signal(SIGTSTP,sigtstp_handler);
                    if(toBg){
                        job x;
                        x.pid= pid;
                        x.running = false;
                        x.cmd = jobs[no-1].cmd;
                        jobs.push_back(x);
                        toBg = 0;
                        fgProcess = 0;
                        cout << "Job added to the background and stopped." << endl;
                        break;
                    }

               }while(wid == 0);
            }
        }
    }
    else{
        launch(tokens,pipe);
    }

}
void loop(){
    char *cmd=(char *)malloc(sizeof(char)*BUFSIZE);
    if(!cmd){
        fprintf(stderr,"Couldn't allocate the specified memory");
        exit(EXIT_FAILURE);
    }
    vector <string> argv;
    int status;
    while(1){
        shellprompt();
        read_line(&cmd);
	pid_t pid = waitpid(pid,&status,WNOHANG);
	pipe(pipe_ints);
        argv = splitatpipe(cmd);
        if(argv.size()>1){
            vector<string> cmd1 = parse(argv[0]);
            vector<string> cmd2 = parse(argv[1]);
            execute(cmd1,0);
            execute(cmd2,1);
        }
        else{
            vector<string> singlecmd=parse(argv[0]);
            execute(singlecmd,-1);
        }
	if(pid > 0){
		int j=0;

	vector<job>::iterator i;
            for (i = jobs.begin(); i != jobs.end(); i++) {
                j++;
                if (i->pid == pid) {
                    cout << '[' << j << ']' << " " << pid << " Done " << i->cmd << endl;
                    jobs.erase(i);
                    break;
                }
            }
	}
	argv.clear();

    }
}
int main(int argc , char **argv){
    loop();
    return 0;
};
