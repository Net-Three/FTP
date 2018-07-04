#include <stdio.h>
#include <fcntl.h>
#include <dirent.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include<stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include<string.h>
#include "CreatServer.h"
#define PORT 2400
 int sock_fd;
struct commend{
    int commend;
    char info[1024];
};
void* process_client();

int main(){
    int ss = create_ftp_server(PORT);
    if(-1 == ss)
        exit(-1);
    while(1){
        //接受客户端连接
        socklen_t addrlen = sizeof(struct sockaddr);
        struct sockaddr_in client_addr; //客户端地址结构
        int client_sock = accept(ss, (struct sockaddr*)&client_addr, &addrlen);
        if(client_sock < 0){
            printf("accept error\n");
        }
        printf("accept success\n");
        pthread_t pid;//创建进程
        //&client_sock作为参数传到了处理线程中
        if(pthread_create(&pid, NULL, process_client, &client_sock) < 0){
            printf("pthread_create error\n");
        }
    }
}

//处理客户端程序
void *process_client(void *arg){
    int size = 0, fd, count = 0, sockid = *(int*)arg;
    struct commend pac;
    long total = 0, cur = 0;
    char*dir="/home/cuteabacus/FTP/dir";
    //循环接收命令
    while(1) {
        memset(&pac, 0, sizeof(pac));
        size = read(sockid, &pac, sizeof(pac));
        if(size > 0){
            if (pac.commend == 1){
                char username[1024];
                 for(int i=0;i<strlen(pac.info);i++){
                    username[i]=pac.info[i];
                }
                int fd,n;
                int mark=0;
                bool manage=false;
                char name[1024];
                char code[1024];
                char buf[1024];
                char right[10];
                memset(right,'\0',sizeof(right));
                char *path="/home/cuteabacus/FTP/server/common_login";
                char *path1="/home/cuteabacus/FTP/server/manage_login";
                 FILE *fp;
                     if ((fp=fopen(path1,"r"))==NULL){
                    printf("打开文件%s错误\n",path);
                    return NULL;
                }
                fseek(fp,0,SEEK_END); 
                rewind(fp);
                while((fgets(buf,1000,fp))!=NULL){
                    memset(name,'\0',sizeof(name));
                    memset(code,'\0',sizeof(code));
                     if(strlen(buf)==0)break;
                     sscanf(buf,"%s %s",name,code);
                      if(strcmp(name,username)==0){
                          write(sockid,code,sizeof(code));
                          strcpy(right,"2");
                          write(sockid,right,sizeof(right));
                          mark++;
                          manage=true;
                          break;
                      }
                      memset(buf,'\0',sizeof(buf));
                }
                fclose(fp);
                if(!mark){
                        if ((fp=fopen(path,"r"))==NULL){
                            printf("打开文件%s错误\n",path);
                            return NULL;
                        }
                        fseek(fp,0,SEEK_END); 
                        rewind(fp);
                        while((fgets(buf,1000,fp))!=NULL){
                            memset(name,'\0',sizeof(name));
                            memset(code,'\0',sizeof(code));
                            if(strlen(buf)==0)break;
                            sscanf(buf,"%s %s",name,code);
                            if(strcmp(name,username)==0){
                                write(sockid,code,sizeof(code));
                                strcpy(right,"1");
                                write(sockid,right,sizeof(right));
                                mark++;
                                break;
                            }
                            memset(buf,'\0',sizeof(buf));
                        }
                        fclose(fp);
                }
                if(mark){
                    printf("%s\n",name);
                }
                else{
                    write(sockid,"null",sizeof("null"));
                }
                continue;
            }
            else if (pac.commend== 2){
                   DIR *dfd;
                   struct dirent*dp;
                   char  buf[256];
                    if((dfd = opendir(dir)) == NULL){
                            fprintf(stderr, "dirwalk: can't open %s\n", dir);
                            return;
                        }
                     while((dp = readdir(dfd)) != NULL){ //读目录记录项
                        if(strcmp(dp->d_name, ".") == 0 || strcmp(dp -> d_name, "..") == 0){
                            continue;  //跳过当前目录以及父目录.和.都不管
                        }
                    //加２的原因是还有２个斜杠
                        if(strlen(dir) + strlen(dp -> d_name) + 2 > 1024){
                            fprintf(stderr, " name %s %s too long\n", dir, dp->d_name);
                        }else{
                            char name[1024];
                            memset(name,'\0',sizeof(name));
                            sprintf(name, "%s", dp->d_name);
                            write(sockid,&name,sizeof(name));
                        }
                    }
                     strcpy(buf,"FIN\0");
                     write(sockid,&buf,256);
                     continue;
            }
            else if (pac.commend== 3){
                //获取要下载的文件名
                char filename[256];
                char ans[1024];
                char buf[1024];
                read(sockid,filename,sizeof(filename));
                printf("%s\n",filename);
                char fileplace[256];
                memset(ans,'\0',sizeof(ans));
                strcpy(fileplace,dir);
                strcat(fileplace,"/");
                strcat(fileplace,filename);
                printf("%s\n",fileplace);
                if((access(fileplace,F_OK))==-1){
                    strcpy(ans,"NOT\0");
                    write(sockid,&ans,256);
                    continue;
                }
                else{
                    strcpy(ans,"RIT\0");
                    write(sockid,&ans,256);
                }   
                char t[256];
                read(sockid,t,256);
                if(strcmp(t,"NO")==0)
                continue;
                //发送文件
                memset(buf,'\0',sizeof(buf));
                if ((fd=open(fileplace,O_RDONLY)) == -1) {
                    printf("wrong\n");
                    exit(1);
                }
                int n;
                while(true){
                    n=read(fd,buf,sizeof(buf));
                    if(n==0)break;
                    write(sockid,buf,sizeof(buf));
                    printf("%s\n",buf);
                    memset(buf,'\0',sizeof(buf));
                }
                close(fd);
                memset(buf,'\0',sizeof(buf));
                //结束符
                strcpy(buf,"FIN\0");
                write(sockid,buf,sizeof(buf));
            /*    int filesize;
                struct stat statbuff;  
                if(stat(fileplace, &statbuff) < 0){  
                    return filesize;  
                }else{  
                    filesize = statbuff.st_size;  
                }  
                struct commend temp;
                temp.commend=filesize;
                write(sockid,&temp,sizeof(struct commend));
                printf("%d\n",filesize);*/
                continue;
            }
            else if(pac.commend == 4){
                char filename[256];
                char buf[1024];                
                char temp[256];
                memset(temp,'\0',sizeof(temp));
                strcat(temp,dir);
                strcat(temp,"/");
                strcat(temp,pac.info);
                if((access(temp,F_OK))!=-1)   
                {    
                    printf("文件已存在\n");
                    strcpy(buf,"DENY");
                    write(sockid,buf,sizeof(buf));
                    return;
                 }   
                 else{
                     strcpy(buf,"YES");
                     write(sockid,buf,sizeof(buf));
                 }
                    int fd=open(temp,O_WRONLY|O_CREAT);
                    int size=0;
                    memset(buf,'\0',sizeof(buf));
                    read(sockid,buf,sizeof(buf));//printf("%s\n",buf); 
                    while(strcmp(buf,"FIN\0")!=0){
                        size=strlen(buf);
                        printf("%s\n",buf);
                        write(fd,buf,size);     
                        memset(buf,'\0',sizeof(buf));
                        read(sockid,buf,sizeof(buf));
                    }
                          printf("%s\n",buf);
                        close(fd);
                        continue;
            }else if(pac.commend==5){
                printf("jjjj\n");
                char fileplace[256];
                char buf[256];
                memset(fileplace,'\0',sizeof(fileplace));
                memset(buf,'\0',sizeof(buf));
                read(sockid,fileplace,sizeof(fileplace));
                printf("%s\n",fileplace);
                if(opendir(fileplace)!=NULL){
                    strcpy(buf,"YES");
                    write(sockid,buf,sizeof(buf));
                    dir=fileplace;
                }
                else{ 
                    strcpy(buf,"NO");
                    write(sockid,buf,sizeof(buf));
                }
                continue;
            }else if(pac.commend==0){

            }else if(pac.commend==6){

            }else if(pac.commend==7){
                
            }
        } else{
            printf("client disconnected\n");
            close(sockid);
            break;
        }
    }
    return 0;
}
