#include<stdio.h>
#include<stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <dirent.h>
#include <fcntl.h>
#include<string.h>
#include <termios.h>
#include <sys/stat.h> 
#include"Socket.h"
#define SIZE 1024
bool Login=false;
bool Manage=false;
 int sock_fd;
struct commend{
    int commend;
    char info[1024];
};
void  login();
void browse();
void file_download();
void file_upload();
void quit();
void dir_change();
int main(){
    //先写login和目录传输
    while(1){
        char host[200]="";
        int port=0; 
        printf("请输入server端ip port:");
        scanf("%s%d", host,&port);
        sock_fd = connect_ftp(host, port);
        if(-1==sock_fd){
            printf("connected failed!\n");
            continue;
        }else{
            break;
        }
    }
        printf("连接成功 功能编码如下:\n");
        printf("登    录    login\n");
        printf("浏览目录    browse\n");
        printf("上传文件    file_upper\n");
        printf("下载文件    download\n");
        printf("登    出    quit\n");
        printf("更改目录    ch_dir\n");
        char action[100];
        while(1){
                    printf("FTP< ");
        scanf("%s",action);
       // printf("%s",action);
        if(strcmp(action,"login")==0&&!Login){
                login();
                continue;
        }
        else if(strcmp(action,"browse")==0&&Login){
                browse();
        }
        else if(strcmp(action,"download")==0&&Login){
                file_download();
        }
        else if(strcmp(action,"file_upper")==0&&Login&&Manage){
                file_upload();
        }
        else if(strcmp(action,"ch_dir")==0&&Login){
                dir_change();
        }        
        else if(strcmp(action,"quit")==0){
            printf("感谢您使用简易ftp,欢迎下次光临\n");
            close(sock_fd);
            exit(0);
        } else if(!Login){
            printf("您还未登录,请先登录!\n");
            continue;
         }else{
             printf("您的输如有误,或无此权限,请重新输入!\n");
             continue;
         }      
    }
}

void login(){
    char  username[SIZE];
    char  code[SIZE];
    char code_server[SIZE];
    struct commend log;
    printf("FTP<请输入用户名 ");
    scanf("%s",username);
    printf("FTP<请输入密码 ");
      struct termios oldflags, newflags;
    //设置终端为不回显模式
    tcgetattr(fileno(stdin), &oldflags);
    newflags = oldflags;
    newflags.c_lflag &= ~ECHO;
    newflags.c_lflag |= ECHONL;
    if (tcsetattr(fileno(stdin), TCSANOW, &newflags) != 0)
    {
        perror("tcsetattr");
        return -1;
    }
    //获取来自键盘的输入
    scanf("%s",code);
     //恢复原来的终端设置
    if (tcsetattr(fileno(stdin), TCSANOW, &oldflags) != 0)
    {
        perror("tcsetattr");
        return -1;
    }
    int action=1;
    log.commend=action;
    memset(log.info,'\0',1024);
    for(int i=0;i<strlen(username);i++){
        log.info[i]=username[i];
    }
    write(sock_fd, &log, sizeof(log));
    read(sock_fd, &code_server, sizeof(code_server));
    char right[10];
    memset(right,'\0',sizeof(right));

    if(strcmp(code_server,"null")==0){
        printf("FTP<用户不存在\n");
        return ;
    }
    if(strcmp(code,code_server)==0){
        printf("FTP<登录成功\n");
        Login=true;
        read(sock_fd,&right,sizeof(right));
        if(strcmp(right,"2")==0){
            Manage=true;
            printf("您好,管理员\n");
        }
    }
    else{
        printf("FTP<密码错误\n");
        return ;
    }
}
void browse(){
     struct commend log;
     struct dirent *dp;
     char dir[SIZE];
     char buf[1024];
     int action=2;
     log.commend=action;
     write(sock_fd, &log, sizeof(struct commend));
     read(sock_fd, &buf,256);
     while(strcmp(buf,"FIN\0")!=0){
         printf("%s\t",buf);
         read(sock_fd, &buf,256);
     }
     printf("\n");
    return;
}
void file_download(){
    printf("可下载目录如下:\n");
    browse();
    struct commend log;
    log.commend=3;
    write(sock_fd, &log, sizeof(struct commend));
    printf("请选择要下载的文件:\n");
    char filename[256];
    char check[256];
    scanf("%s",filename);
    write(sock_fd, &filename, sizeof(filename));  
    read(sock_fd,&check,sizeof(check));
    printf("%s",check);//check已经读过了
    if(strcmp(check,"RIT")!=0){
        printf("您输入的文件名有误,请重新输入!\n");
        return ;
    } 
    printf("请输入下载位置\n");
    char fileplace[256];
    scanf("%s",fileplace);
    if(opendir(fileplace)==NULL){
        printf("目录不存在,将下载到默认目录!\n");
        memset(fileplace,'\0',sizeof(fileplace));
        strcpy(fileplace,"/home/cuteabacus");
    }
    else{
         printf("目录合法!\n");
    }
    strcat(fileplace,"/");
    strcat(fileplace,filename);
    char t[256];
    if((access(fileplace,F_OK))!=-1)   
    {   
        printf("重名文件已存在!\n");   
        strcpy(t,"NO");
        write(sock_fd,&t,sizeof(t));
        return;
    }   
    else  
    {   
         printf("路径合法!\n");   
         strcpy(t,"YES");
         write(sock_fd,&t,sizeof(t));
    }   
    //开始接收
     char buf[256];
     char ans[256];
     int number;
     int size=0;
     char*file;
     int fd=open(fileplace,O_WRONLY|O_CREAT);
     memset(buf,'\0',sizeof(buf));
     memset(ans,'\0',sizeof(ans));
     read(sock_fd,buf,sizeof(buf));//printf("%s\n",buf); 
     while(strcmp(buf,"FIN\0")!=0){
          size=strlen(buf);
          printf("%s\n",buf);
          write(fd,buf,size);     
          memset(buf,'\0',sizeof(buf));
          number=strlen(buf);
          read(sock_fd,buf,sizeof(buf));
     }
        close(fd);
        int filesize;
        struct stat statbuff;  
        if(stat(fileplace, &statbuff) < 0){  
            return filesize;  
        }else{  
            filesize = statbuff.st_size;  
        }  
        printf("传输完成,文件大小%s\n",filesize);
        return;
}
void file_upload(){
        char buf[SIZE];
        struct commend log;
        log.commend=4;
        printf("请输入文件所在的路径:\n");
        char fileplace[256];
        scanf("%s",fileplace);
         printf("%s",fileplace);
        printf("请输入文件名:\n");
        char filename[256];
        scanf("%s",filename);
        char temp[256];
        memset(temp,'\0',sizeof(temp));
        strcat(temp,fileplace);
        printf("%s\n",temp);
        strcat(temp,"/");
        printf("%s\n",temp);
        strcat(temp,filename);
        printf("%s\n",temp);
        if((access(temp,F_OK))==-1)   
        {    
        printf("文件不存在\n");
        return;
       }   
       printf("开始传输:");
       strcpy(log.info,filename);
        write(sock_fd, &log, sizeof(struct commend));
        read(sock_fd,buf,sizeof(buf));
        if(strcmp(buf,"DENY")==0){
            printf("服务器拒绝了我们的请求\n");
            return;
        }
        int n;int fd;
        if ((fd=open(temp,O_RDONLY)) == -1) {
            printf("wrong\n");
            exit(1);
        }
        printf("hhhh\n");
        memset(buf,'\0',sizeof(buf));
        while(true){
            n=read(fd,buf,sizeof(buf));
            if(n==0)break;
            write(sock_fd,buf,sizeof(buf));
            printf("%s\n",buf);
            memset(buf,'\0',sizeof(buf));
        }
        close(fd);
        memset(buf,'\0',sizeof(buf));
        //结束符
        strcpy(buf,"FIN\0");
        write(sock_fd,buf,sizeof(buf));
        printf("传输完成!\n");
}

void dir_change(){
    struct commend log;
    log.commend=5;
    printf("请输入要转到的目录:\n");
    char fileplace[256];
    char buf[256];
    memset(buf,'\0',sizeof(buf));
    memset(fileplace,'\0',sizeof(fileplace));
    scanf("%s",fileplace);    
    write(sock_fd,&log,sizeof(struct commend));
    write(sock_fd,fileplace,sizeof(fileplace));
    read(sock_fd,buf,sizeof(buf));
    if(strcmp(buf,"NO")==0){
        printf("您给出的目录无效\n");
    }
    else{
        printf("目录更换成功\n");
    }
}
void right_change(){
    struct commend log;
    char buf[256];
    memset(buf,'\0',sizeof(buf));
    printf("请输入要进行的操作\n");
    printf("1.升级    2.降级\n");
    int action;
    scanf("%d",action);
    switch(action){
        case 1:{
            printf("请输入要升级的用户名\n");
            scanf("%s",buf);
            log.commend=6;
            write(sock_fd,&log,sizeof(struct commend));
            write(sock_fd,buf,sizeof(buf));
            memset(buf,'\0',sizeof(buf));
            read(sock_fd,buf,sizeof(buf));
            if(strcmp(buf,"YES"==0))printf("升级成功\n");
            else printf("普通用户中查无此人\n");
        }break;
        case 2:{
           printf("请输入要降级的用户名\n");
            scanf("%s",buf);
            log.commend=7;
            write(sock_fd,&log,sizeof(struct commend));
            write(sock_fd,buf,sizeof(buf));
            memset(buf,'\0',sizeof(buf));
            read(sock_fd,buf,sizeof(buf));
            if(strcmp(buf,"YES"==0))printf("降级成功\n");
            else printf("管理员用户中查无此人\n");
        }break;
        default:printf("您的 输入有误\n");break;
    }
}