#include<bits/stdc++.h>
#include<dirent.h>
#include<unistd.h>
#include<sys/stat.h>
#include<pwd.h>
#include<grp.h>
#include<string.h>
#include<time.h>
#include<langinfo.h>
#include<termios.h>
#include<sys/ioctl.h>
#include<fcntl.h>
#include<stack>
using namespace std;

// #define clrscr() printf("\033[H\033[J")
// #define erase() 
#define normal 0
#define command 1
#define up 65
#define down 66
#define left 67
#define right 68
#define enter 10
#define backspace 127
#define escape 27

void normal_mode();
void command_mode();
void term_raw();
void term_exit();
void cursorpos(int ,int);
void listff(string);
void display(string);
void nmode();
void searchfile(string, string);
vector<string> parsecmd(string);
void copy_file(string, string, string);
void copy_dir(string, string, string);

DIR* d;
struct dirent* dir;
struct stat file;
stack<string> fwd;
stack<string> bkwd;
stack<string> stk;
static struct termios orig;
static int term_fd = STDIN_FILENO;
struct winsize w;
int row = 1, col = 0, rc = 0;               //cursor default pos
vector<vector<string>> files;               //contains file names in current directory
string cwdir;                    // current working directory
int scroll = 15;
int start=0, last=scroll;
int mode = normal;
char cwd[256];                    //we have to clear cwd array as well??????????
bool searchff;


void term_exit(){
    tcsetattr(term_fd,TCSAFLUSH,&orig);
}

void term_raw()
{
    tcgetattr(STDIN_FILENO, &orig);
    atexit(term_exit); 
    struct termios raw;
    raw = orig;
    raw.c_lflag &= ~(OPOST | ECHO | ICANON | IEXTEN | ISIG);
    raw.c_cc[6] = 1;                                      //VMIN 5
    raw.c_cc[5] = 0;                                      //VTIME 6
    if (tcsetattr(term_fd,TCSAFLUSH,&raw) < 0){
        cout<<"cannot enable raw mode";
        exit(1);
    } 
}

void cursorpos(int x, int y){
    printf("%c[%d;%df",0x1B,y,x);
    fflush(stdout);
}

void listff(string currdir){
    // pair<string,string> ft;
    // cwdir = currdir;
    string path="";
    files.clear();
    vector<string> metadata;
    int fsize;
    int n=0;
    struct passwd *pswd;
    struct group *grp;
    struct tm *t;
    string permissions="";
    char date[25];
    stat(currdir.c_str(),&file);
    d = opendir(currdir.c_str());
    cout<<"\033[H\033[0;02J";         //clear screen and set cursor on 0,0
    if(d!=NULL){
        while((dir = readdir(d)) != NULL){
            path = currdir+"/"+dir->d_name;
            metadata.push_back(dir->d_name);
            stat(path.c_str(),&file);  
            if(S_ISREG(file.st_mode)) permissions+="-";
            else permissions+="d";
            permissions+=(file.st_mode & S_IRUSR) ? "r" : "-";
            permissions+=(file.st_mode & S_IWUSR) ? "w" : "-";
            permissions+=(file.st_mode & S_IXUSR) ? "x" : "-";
            permissions+=(file.st_mode & S_IRGRP) ? "r" : "-";
            permissions+=(file.st_mode & S_IWGRP) ? "w" : "-";
            permissions+=(file.st_mode & S_IXGRP) ? "x" : "-";
            permissions+=(file.st_mode & S_IROTH) ? "r" : "-";
            permissions+=(file.st_mode & S_IWOTH) ? "w" : "-";
            permissions+=(file.st_mode & S_IXOTH) ? "x" : "-";
            metadata.push_back(permissions);
            permissions="";
            pswd=getpwuid(file.st_uid);
            metadata.push_back(pswd->pw_name);
            // printf("%s ",pswd->pw_name);
            grp=getgrgid(file.st_gid);
            metadata.push_back(grp->gr_name);
            // printf("%s\t",grp->gr_name);
            fsize = (int)file.st_size;
            if(fsize>1024){
            fsize = (int)(fsize/1024);
            metadata.push_back(to_string(fsize)+"K");
            }
            else{
                metadata.push_back(to_string(fsize)+"B");
            }
            t = localtime(&file.st_mtime);
            strftime(date, sizeof(date), nl_langinfo(D_T_FMT), t);
            metadata.push_back(date);
            files.push_back(metadata);
            metadata.clear();
        }
        closedir(d);                               //set_w
        chdir(currdir.c_str());
    }
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    int x = 0, y = w.ws_row;
    // scroll = y-4;
    sort(files.begin(),files.end());
    start = 0;
    last = min(scroll, int(files.size()));
    for(int i=0;i<last;i++){
        cout<<files[i][1];
        cout<<"\t"<<files[i][2];
        cout<<" "<<files[i][3];
        cout<<"\t"<<files[i][4];
        cout<<"\t"<<files[i][5];
        cout<<"\t"<<files[i][0]<<"\n";
    }
    cursorpos(x,y-3);
    string text;
    if(mode==normal)
    text = ">>> NORMAL MODE: ";
    else if(mode==command)
    text = ">>> COMMAND MODE: ";
    cout<<text<<currdir;
    cursorpos(0,row);
    return;
}

void nmode(){
    char cwd[256];
    getcwd(cwd,256);
    cwdir = string(cwd);
    listff(cwdir);
    cursorpos(0, 1);
    return;
}

void display(string currdir){
    cout << "\033[H\033[0;02J";         //clear screen and set cursor on 0,0
    for(int i=start;i<last;i++){
        cout<<files[i][1];
        cout<<"\t"<<files[i][2];
        cout<<"  "<<files[i][3];
        cout<<"\t"<<files[i][4];
        cout<<"\t"<<files[i][5];
        cout<<"\t"<<files[i][0]<<"\n";
    }
    cursorpos(0,w.ws_row-3);
    string text = ">>> NORMAL MODE: ";
    cout<<text<<cwdir;
    // cursorpos(1,0);
    return;
}

bool searchfile(string currdir, string currff, pair<string,char> meta){
    if(meta.first==currff) return 1;
    if(meta.first=="."||meta.first=="..") return 0;
    if(meta.second=='-' && meta.first!=currff) return 0;
    else{
        DIR* d;
        struct dirent* dir;
        stat(currdir.c_str(),&file);
        d = opendir(currdir.c_str());
        if(d!=NULL){
        while((dir = readdir(d)) != NULL){
            meta.first=dir->d_name;
            // meta.first = "\'"+meta.first+"\'";
            stat((currdir+"/"+dir->d_name).c_str(),&file);  
            if(S_ISREG(file.st_mode)) meta.second='-';
            else meta.second='d';
            // metadata.push_back(meta);
            if(searchfile(currdir+"/"+meta.first,currff,meta)) return 1;
        }
        closedir(d); 
        } 
        return  0;                             //set_w
    }
}

vector<string> parsecmd(string currdir){
    vector<string> filecmd;
    // cout<<currdir;
    string filename="";
    int quote=0, space=1;
    for(int i=0;i<currdir.size();i++){
        if(currdir[i]==' ' && quote==0){
            filecmd.push_back(filename);
            filename="";
        }
        else if(currdir[i]=='\''){
            if(quote==1){
                filename+=currdir[i];
                filecmd.push_back(filename);
                filename="";
            }
            else{
                quote=1;
                filename+=currdir[i];
            }
        }
        else filename+=currdir[i];
    }
    filecmd.push_back(filename);
    return filecmd;
}

void copy_file(string cmdir_, string cmdir, mode_t mode){   ///   destination path
            //new file path
            char write_file[2000];
            int flag = 0, src_flag = -1, dest_flag=-1,n = 0;
            src_flag = open(cmdir_.c_str(),O_RDONLY);
            dest_flag = open((cmdir).c_str(), O_CREAT|O_WRONLY, mode);
            while((n = read(src_flag,write_file,sizeof(write_file)))>0) write(dest_flag,write_file,n);
        return;
}

void copy_dir(string cmdir_, string cmdir, mode_t mode){
    DIR *d;
    struct dirent *dir;
    string path="";
    struct stat file;
    vector<pair<string,pair<string,mode_t>>> files_cmd;
    pair<string,pair<string, mode_t>> metadata_cmd;
    pair<string, mode_t> meta_cmd;
    string permissions = "";
    // stat(cmdir_.c_str(),&file);
    d = opendir(cmdir_.c_str());
    if(d!=NULL){
        while((dir = readdir(d)) != NULL){
            path = cmdir_+"/"+dir->d_name;
            metadata_cmd.first = (dir->d_name);
            stat(path.c_str(),&file);
            meta_cmd.second = file.st_mode;
            if (S_ISREG(file.st_mode))
                permissions += "-";
            else
                permissions += "d";
            permissions += (file.st_mode & S_IRUSR) ? "r" : "-";
            permissions += (file.st_mode & S_IWUSR) ? "w" : "-";
            permissions += (file.st_mode & S_IXUSR) ? "x" : "-";
            permissions += (file.st_mode & S_IRGRP) ? "r" : "-";
            permissions += (file.st_mode & S_IWGRP) ? "w" : "-";
            permissions += (file.st_mode & S_IXGRP) ? "x" : "-";
            permissions += (file.st_mode & S_IROTH) ? "r" : "-";
            permissions += (file.st_mode & S_IWOTH) ? "w" : "-";
            permissions += (file.st_mode & S_IXOTH) ? "x" : "-";
            meta_cmd.first = permissions;
            permissions = "";
            metadata_cmd.second = meta_cmd;
            files_cmd.push_back(metadata_cmd);
        }
        closedir(d);                              
    }
    string temp = "";
    for(int i=cmdir_.size()-1;i>=0;i--){
        if(cmdir_[i]=='/') break;
        temp+=cmdir_[i];        
    }
    reverse(temp.begin(),temp.end());
    path = cmdir+"/"+temp;
    mkdir((path).c_str(),mode);
    sort(files_cmd.begin(),files_cmd.end());
    for(int i=2;i<files_cmd.size();i++){
        if(files_cmd[i].second.first[0]=='d')
            copy_dir(cmdir_+"/"+files_cmd[i].first, path, files_cmd[i].second.second);
        else
            copy_file(cmdir_+"/"+files_cmd[i].first, path+"/"+files_cmd[i].first, files_cmd[i].second.second);
    }
    return;
}

void move_file(string cmdir_, string cmdir){
    string filecmd="";
    for(int i=cmdir_.size()-1;i>=0;i--){
        if(cmdir_[i]=='/') break;
        filecmd+=cmdir_[i];        
    }
    reverse(filecmd.begin(),filecmd.end());
    if(rename(cmdir_.c_str(),(cmdir+"/"+filecmd).c_str())==0){
        cursorpos(0,w.ws_row-1);
        cout<<"\033[0K";
        cout<<"Moved successfully";
    }
}

void move_dir(string cmdir_, string cmdir){
    string filecmd="";
    for(int i=cmdir_.size()-1;i>=0;i--){
        if(cmdir_[i]=='/') break;
        filecmd+=cmdir_[i];        
    }
    reverse(filecmd.begin(),filecmd.end());
    if(rename(cmdir_.c_str(),(cmdir+"/"+filecmd).c_str())==0){
        cursorpos(0,w.ws_row-1);
        cout<<"\033[0K";
        cout<<"Moved successfully";
    }
}

void delete_file(string cmdir){
    remove(cmdir.c_str());
}

void delete_dir(string cmdir){
    DIR *d;
    struct dirent *dir;
    string path = "";
    struct stat file;
    vector<pair<string, char>> files_cmd;
    pair<string, char> metadata_cmd;
    d = opendir(cmdir.c_str());
    if (d != NULL)
    {
        while ((dir = readdir(d)) != NULL)
        {
            path = cmdir + "/" + dir->d_name;
            metadata_cmd.first = (dir->d_name);
            stat(path.c_str(), &file);
            if (S_ISREG(file.st_mode))
                metadata_cmd.second = '-';
            else
                metadata_cmd.second = 'd';
            files_cmd.push_back(metadata_cmd);
        }
        closedir(d);
    }
    sort(files_cmd.begin(), files_cmd.end());
    for (int i = 2; i < files_cmd.size(); i++)
    {
        if (files_cmd[i].second == 'd'){
                delete_dir(cmdir+"/"+files_cmd[i].first);
            }
        else
            delete_file(cmdir + "/" + files_cmd[i].first);
    }
    rmdir(cmdir.c_str());
    return;
}

void command_mode(){
    mode = command;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    cursorpos(0,w.ws_row-3);
    string text = ">>> COMMAND MODE: ";
    cout<<text<<cwdir;
    cursorpos(0,w.ws_row-2);
    cout<<"$ ";
    vector<string> filecmd;
    string filename;
    vector<char> cmd;
    vector<pair<string,char>> vec;
    pair<string,char> meta;
    string cmdir="",tempPath="";
    int flag=0;
    char *absPath;
    struct passwd *pw;
    uid_t uid;
    char c;
    while(c=getchar()){
            if(c==escape){
                ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
                row = 1;
                normal_mode();
            }
            else if(c==backspace){
                if(cmd.size()>0){
                cmd.pop_back();
                cursorpos(0,w.ws_row-2);
                cout<<"\033[0K$ ";
                cout<<string(cmd.begin(),cmd.end());
                }
            }
            else if(c==enter){
                if(string(cmd.begin(),cmd.end())=="quit"){
                    printf("\033[H\033[J");
                    exit(1);
                }   
                else if(string(cmd.begin(),cmd.begin()+5)=="goto "){
                    cmdir = string(cmd.begin()+5,cmd.end());
                    if(cmdir[0]=='~'){ 
                        uid = geteuid();
                        pw = getpwuid(uid);
                        tempPath = "/home/"+string(pw->pw_name);
                        tempPath+=cmdir.substr(1,cmdir.size());
                    }
                    else{
                        tempPath = cmdir;
                    }
                    // cout<<"3..\t"<<cwdir;
                    absPath = realpath(tempPath.c_str(), cwd);
                    if(absPath){
                        cwdir=string(cwd);
                        struct stat file;
                        stat(cwdir.c_str(), &file);
                        if(S_ISDIR(file.st_mode)){
                        stk.push(cwdir);
                        fwd.push(cwdir);
                        // cout<<cwdir;
                        listff(cwdir);
                        }
                        else{
                            string fname="";
                            pid_t pid = fork();
                            if (pid == 0)
                            {
                                fname = "\'" + files[start + row - 1][0] + "\'";
                                fname = cwdir + "/" + fname;
                                execl("/bin/sh", "sh", "-c", ("xdg-open " + fname).c_str(), (char *)NULL);
                                exit(1);
                            }
                        }
                    }
                    else{
                        cursorpos(0,w.ws_row-1);
                        cout<<"\033[0K$ ";
                        cursorpos(0,w.ws_row-1);
                        cout<<"File doesn't exist";
                    }
                }
                else if(string(cmd.begin(),cmd.begin()+7)=="search "){
                    cmdir = string(cmd.begin()+7,cmd.end());
                    if(searchfile(cwdir,cmdir, meta)){
                        cursorpos(0,w.ws_row-1);
                        cout<<"\033[0K";
                        cout<<"True";
                    }
                    else{
                        cursorpos(0,w.ws_row-1);
                        cout<<"\033[0K";
                        cout<<"False";
                    }
                }
                else if(string(cmd.begin(),cmd.begin()+7)=="rename "){
                    filecmd = parsecmd(string(cmd.begin()+7,cmd.end()));
                    if(filecmd[0][0]=='~'){ 
                        uid = geteuid();
                        pw = getpwuid(uid);
                        tempPath = "/home/"+string(pw->pw_name);
                        tempPath+=filecmd[0].substr(1,filecmd[0].size());
                        cmdir = tempPath;
                    }
                    absPath = realpath((cwdir+"/"+filecmd[0]).c_str(), cwd);
                    if(absPath){
                        cmdir = string(cwd);
                        filename="";
                        int i=0;
                        for(i=cmdir.size()-1;i>=0;i--){
                            if(cmdir[i]=='/') break;
                            else filename+=cmdir[i];
                        }
                        cmdir = cmdir.substr(0,i);
                        reverse(filename.begin(),filename.end());
                        if(rename((cmdir+"/"+filename).c_str(),(cmdir+"/"+filecmd[1]).c_str())==0){
                            cursorpos(0,1);
                            cwdir = cmdir;
                        stk.push(cwdir);
                        fwd.push(cwdir);
                            listff(cwdir);
                            cursorpos(0,w.ws_row-1);
                            cout<<"\033[0K";
                            cout<<"File renamed successfully";
                        }
                        else{
                            cursorpos(0,w.ws_row-1);
                            cout<<"\033[0K";
                            cout<<"File not found";
                        }
                    }
                    else{
                        cursorpos(0,w.ws_row-1);
                            cout<<"\033[0K";
                        cout<<"Cannot rename file";
                    }
                }
                else if(string(cmd.begin(),cmd.begin()+12)=="create_file " || string(cmd.begin(),cmd.begin()+11)=="create_dir "){
                    if(string(cmd.begin(),cmd.begin()+12)=="create_file ")
                    filecmd = parsecmd(string(cmd.begin()+12,cmd.end()));
                    else if(string(cmd.begin(),cmd.begin()+11)=="create_dir ")
                    filecmd = parsecmd(string(cmd.begin()+11,cmd.end()));
                    if(filecmd[1][0]=='~'){ 
                        uid = geteuid();
                        pw = getpwuid(uid);
                        tempPath = "/home/"+string(pw->pw_name);
                        cmdir = tempPath;
                        tempPath = filecmd[1].substr(1,filecmd[1].size());
                        cmdir+="/"+tempPath;
                        filecmd[1] = cmdir;
                    }
                    // cmdir = filecmd[1];
                    absPath = realpath((filecmd[1]).c_str(), cwd);
                    if(absPath){
                        cmdir = string(cwd);
                        int flag=-1;
                        // cwdir = cmdir;
                        // listff(cwdir);
                        if(string(cmd.begin(),cmd.begin()+11)=="create_dir "){
                        mode_t mode = (S_IRWXO | S_IRUSR | S_IWUSR | S_IXUSR);
                        flag = mkdir((filecmd[1]+"/"+filecmd[0]).c_str(),mode);
                        if(flag==0){
                            cursorpos(0,1);
                            cwdir = cmdir;
                            stk.push(cwdir);
                            fwd.push(cwdir);
                            listff(cwdir);
                            cursorpos(0,w.ws_row-1);
                            cout<<"\033[0K";
                            cout<<filecmd[0]<<" created successfully";
                        }
                        else{
                            cursorpos(0,w.ws_row-1);
                            cout<<"\033[0K";
                            cout<<"Cannot create "<<filecmd[0];
                        }
                        }
                        else {
                            mode_t mode = (S_IRWXO | S_IRUSR | S_IWUSR | S_IXUSR);
                            if(open((filecmd[1]+"/"+filecmd[0]).c_str(),O_WRONLY|O_CREAT|O_TRUNC,mode)){
                            cursorpos(0,1);
                            cwdir = cmdir;
                            stk.push(cwdir);
                            fwd.push(cwdir);
                            listff(cwdir);
                            cursorpos(0,w.ws_row-1);
                            cout<<"\033[0K";
                            cout<<filecmd[0]<<" created successfully";
                            }
                            else{
                            cursorpos(0,w.ws_row-1);
                            cout<<"\033[0K";
                            cout<<"Cannot create "<<filecmd[0];
                            }
                        }
                    }
                    else{
                        cursorpos(0,w.ws_row-1);
                        cout<<"\033[0K";
                        cout<<"Path doesn't exist";
                    }
                }
                else if(string(cmd.begin(),cmd.begin()+5)=="move "){
                    filecmd = parsecmd(string(cmd.begin()+5,cmd.end()));
                    if(filecmd[filecmd.size()-1][0]=='~'){
                        uid = geteuid();
                        pw = getpwuid(uid);
                        tempPath = "/home/"+string(pw->pw_name);
                        cmdir = tempPath;
                        tempPath = filecmd[filecmd.size()-1].substr(1,filecmd[filecmd.size()-1].size());
                        cmdir+="/"+tempPath;
                    }
                    else{
                        cmdir = filecmd[filecmd.size()-1];
                    }
                    
                    absPath = realpath(cmdir.c_str(),cwd);
                    if(absPath){
                        cmdir = string(cwd);  //dir_create
                        char *absolutePath;
                        string tempPath, cmdir_;
                        char cwd_[512], write_file[2000];
                        int flag = 0, src_flag = -1, dest_flag=-1,n = 0;
                        struct stat file;
                        for(int i=0;i<filecmd.size()-1;i++){
                            if(filecmd[filecmd.size()-1][0]=='~'){
                            uid = geteuid();
                            pw = getpwuid(uid);
                            tempPath = "/home/"+string(pw->pw_name);
                            tempPath += filecmd[i].substr(1,filecmd[i].size());
                            }
                            else{
                            tempPath = filecmd[i];
                            }
                            absolutePath = realpath(tempPath.c_str(),cwd_);    ///dir3
                            if(absolutePath){
                            cmdir_ = string(cwd_); //dir3
                            stat(cmdir_.c_str(),&file);
                            if(S_ISDIR(file.st_mode))
                            move_dir(cmdir_,cmdir);
                            else
                            move_file(cmdir_,cmdir);
                            }
                            else{
                            flag=1;
                            break;
                            }

                        }
                        if(!flag){
                        stk.push(cmdir);
                        fwd.push(cmdir);
                        listff(cwdir=cmdir);
                        cursorpos(0,w.ws_row-2);
                        cout<<"\033[0K";
                        }
                        else{
                        cursorpos(0,w.ws_row-1);
                        cout<<"\033[0K";
                        cout<<"Cannot move files";
                        }
                    }
                    else{
                        cursorpos(0,w.ws_row-1);
                        cout<<"\033[0K";
                        cout<<"Path doesn't exist";
                    }
                }
                else if(string(cmd.begin(),cmd.begin()+5)=="copy "){
                    filecmd = parsecmd(string(cmd.begin()+5, cmd.end()));
                    if(filecmd[filecmd.size()-1][0]=='~'){
                        uid = geteuid();
                        pw = getpwuid(uid);
                        tempPath = "/home/"+string(pw->pw_name);
                        tempPath += filecmd[filecmd.size()-1].substr(1,filecmd[filecmd.size()-1].size());
                    }
                    else{
                        tempPath = filecmd[filecmd.size()-1];
                    }
                    //end of getting destination path, now we have to resolve the path
                    
                    absPath = realpath(tempPath.c_str(),cwd);
                    if(absPath){
                        cmdir = string(cwd);  //dir_create
                        char *absolutePath;
                        string tempPath, cmdir_, permissions;
                        char cwd_[512], write_file[2000];
                        int flag = 0, src_flag = -1, dest_flag=-1,n = 0;
                        struct stat file;
                        mode_t mode;
                        for (int i = 0; i < filecmd.size() - 1; i++)
                        {
                            if(filecmd[filecmd.size()-1][0]=='~'){
                            uid = geteuid();
                            pw = getpwuid(uid);
                            tempPath = "/home/"+string(pw->pw_name);
                            tempPath += filecmd[i].substr(1,filecmd[i].size());
                            }
                            else{
                            tempPath = filecmd[i];
                            }
                            absolutePath = realpath(tempPath.c_str(),cwd_);    ///dir3
                            if(absolutePath){
                            cmdir_ = string(cwd_); //dir3
                            stat(cmdir_.c_str(),&file);
                            file.st_mode;
                            if (S_ISDIR(file.st_mode))
                                copy_dir(cmdir_, cmdir, file.st_mode);
                            else
                                copy_file(cmdir_,cmdir, file.st_mode);
                            }
                            else{
                            flag=1;
                            break;
                            }
                        }
                        if(!flag){
                        stk.push(cmdir);
                        fwd.push(cmdir);
                            listff(cwdir=cmdir);
                            cursorpos(0,w.ws_row-2);
                            cout<<"\033[0K";
                        }
                        else{
                            cursorpos(0,w.ws_row-1);
                            cout<<"\033[0K";
                            cout<<"-----Cannot copy files";
                        }
                    }
                    else{
                        cursorpos(0,w.ws_row-1);
                        cout<<"\033[0K";
                        cout<<"Path doesn't exist";
                    }                                                //check if absPath exists
                }                                                    //end of copy
                
                else if(string(cmd.begin(),cmd.begin()+7)=="delete "){
                    cmdir = string(cmd.begin()+7,cmd.end());
                    if(cmdir[0]=='~'){ 
                        uid = geteuid();
                        pw = getpwuid(uid);
                        tempPath = "/home/"+string(pw->pw_name);
                        tempPath+=cmdir.substr(1,cmdir.size());
                    }
                    else{
                        tempPath = cmdir;
                    }
                    absPath = realpath(tempPath.c_str(), cwd);
                    if(absPath){
                        cmdir=string(cwd);
                        struct stat file;
                        stat(cmdir.c_str(),&file);
                        if(S_ISDIR(file.st_mode)){
                            delete_dir(cmdir);
                            rmdir(cmdir.c_str());
                        }
                        else
                            delete_file(cmdir);
                        int i = 0;
                        for(i=cmdir.size()-1;i>=0;i--){
                            if(cmdir[i]=='/') break;
                        }
                        cwdir = cmdir.substr(0,i);
                        stk.push(cwdir);
                        fwd.push(cwdir);
                        // cout<<cwdir;
                        listff(cwdir);
                        cursorpos(0,w.ws_row-1);
                        cout<<"\033[0K$ ";
                        cursorpos(0,w.ws_row-1);
                        cout<<"File deletion successful";
                    }
                    else{
                        cursorpos(0,w.ws_row-1);
                        cout<<"\033[0K$ ";
                        cursorpos(0,w.ws_row-1);
                        cout<<"Path doesn't exist";
                    }
                }

                cursorpos(0,w.ws_row-2);
                cout<<"\033[0K$ ";
                cmd.clear();
                tempPath.erase();
            }

            else{                              //after enter
                cmd.push_back(c);
                cursorpos(0,w.ws_row-2);
                cout<<"\033[0K$ ";
                cout<<string(cmd.begin(),cmd.end());
            }
    }
}

void normal_mode(){
    mode = normal;
    listff(cwdir);
    char c;
    char cwd[256];
    int i;
    string fname;
    struct passwd *pw;
    uid_t uid;
    while(c=getchar()) {
        switch(c){

        case ':': 
        command_mode();
        break;

        case 'q': cout << "\033[H\033[0;02J";         //clear screen and set cursor on 0,0 
        exit(1);
        break;

        case 'h': fwd.push(cwdir);
        stk.push(cwdir);
        uid = geteuid();
        pw = getpwuid(uid);
        cwdir = "/home/"+string(pw->pw_name);
        listff(cwdir);
        row = 1;
        cursorpos(0,row);
        break;
        
        case 'H': fwd.push(cwdir);
        stk.push(cwdir);
        uid = geteuid();
        pw = getpwuid(uid);
        cwdir = "/home/"+string(pw->pw_name);
        listff(cwdir);
        row = 1;
        cursorpos(0,row);
        break;

        case enter: fflush(stdout);
            stk.push(cwdir);
            if(row==1 && files[start+row-1][0]==".");
            else if(row==2 && files[start+row-1][0]==".."){
            fwd.push(cwdir);
            cwdir = stk.top();
            for(i=cwdir.size()-1;i>=0;i--)
            if(cwdir[i]=='/') break;
            cwdir = cwdir.substr(0,i);
            if(cwdir.size()==0) cwdir+='/';
            listff(cwdir);
            }
        else{
            stat(files[start+row-1][0].c_str(),&file);
            if(files[start+row-1][1][0]=='d'){
            fwd.push(cwdir);
            cwdir = cwdir+"/"+files[start+row-1][0];
            listff(cwdir);
            }
            else{
            pid_t pid = fork();
            if (pid == 0) {
                fname = "\'"+files[start+row-1][0]+"\'";
                fname = cwdir+"/"+fname;
                execl("/bin/sh", "sh", "-c", ("xdg-open "+fname).c_str(), (char *) NULL);
                exit(1);
                }
            }
        }
        row=1;
        cursorpos(0,row);
        break;
        
        case up: fflush(stdout);
        if (row>1) {
            row--;
            cursorpos(0,row);
        }
        if (start != 0 && row==1) {
            start--;
            last--;
            display(cwdir);
            cursorpos(0,row);
        }
        break;

        case down: fflush(stdout);
        if (row<files.size() && row<scroll) {
            row++;
            cursorpos(0,row);
        }
        if (last!=files.size() && row==scroll) {
            start++;
            last++;
            display(cwdir);
            cursorpos(0,row);
        }
        break;
        
        case right: fflush(stdout);           //left
        if(!fwd.empty()){
        cwdir = fwd.top();
        bkwd.push(fwd.top());
        fwd.pop();
        listff(cwdir);
        }
        row=1;
        cursorpos(0,row);
        break;
        
        case left: fflush(stdout);          //right
        if(!bkwd.empty()){
        cwdir = bkwd.top();
        fwd.push(bkwd.top());
        bkwd.pop();
        listff(cwdir);
        }
        row=1;
        cursorpos(0,row);
        break;
        
        case backspace: fflush(stdout);
            stk.push(cwdir);
            fwd.push(cwdir);
            for(i=cwdir.size()-1;i>=0;i--)
            if(cwdir[i]=='/') break;
            cwdir = cwdir.substr(0,i);
            if(cwdir.size()==0) cwdir+='/';
            listff(cwdir);
            row=1;
            cursorpos(0,row);
            break;
        default: break;
        }
    }
    fflush(stdin);
}

int main(){
    term_raw();
    nmode();
    normal_mode();
    return 0;
}
