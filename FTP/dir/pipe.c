#include <stdio.h>   
#include <stdlib.h>     
#include <sys/types.h>   
#include <sys/stat.h>   
#include <fcntl.h> 
#define READ   0       /* The index of the read end of the pipe */
#define WRITE  1       /* The index of the write end of the pipe */
char* phrase = "Stuff this in your pipe and smoke it";
int main()
{
	 int fd [2], bytesRead;
	 char message [100]; /* Parent process' message buffer */
	 pipe (fd); /*Create an unnamed pipe */
	 if (fork () != 0) /* Child, writer */
   	{
     	    close(fd[READ]); /* Close unused end */
     	    write (fd[WRITE],phrase, strlen (phrase) + 1); /* include NULL*/
     	    close (fd[WRITE]); /* Close used end*/
	     
            int ret = mkfifo("fifo", 0666); // 创建命名管道  
            if(ret != 0){   // 出错  
               perror("mkfifo");
            }
	    int fdd =  open("fifo",O_WRONLY);
	     write (fdd,phrase, strlen (phrase) + 1);
   	     close(fdd);
	     printf("传输成功！\n");
	     if( remove("fifo") == 0 )
	        printf("Removed fifo.\n");
	     else
	        perror("remove\n");
	     exit(0);
	}
	 else /* Parent, reader*/
   	{
    	    close (fd[WRITE]); /* Close unusedend */
    	    bytesRead = read (fd[READ], message, 100);
	    
            printf ("Read %d bytes: %s\n", bytesRead, message); /* Send */
            close (fd[READ]); /* Close usedend */
	} 
}
