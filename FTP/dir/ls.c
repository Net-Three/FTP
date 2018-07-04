#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <grp.h>
#include <pwd.h>
#include <time.h>

/*
The following flags are defined for the st_mode field:
　　　　　 是什么类型的文件
           S_IFMT     0170000   bit mask for the file type bit fields　遮罩位
           S_IFSOCK   0140000   socket
           S_IFLNK    0120000   symbolic link
           S_IFREG    0100000   regular file
           S_IFBLK    0060000   block device
           S_IFDIR    0040000   directory
           S_IFCHR    0020000   character device
           S_IFIFO    0010000   FIFO
           S_ISUID    0004000   set UID bit
           S_ISGID    0002000   set-group-ID bit (see below)
           S_ISVTX    0001000   sticky bit (see below)
　　　　　 是否有可读写权限 
           S_IRWXU    00700     mask for file owner permissions 遮罩位
           S_IRUSR    00400     owner has read permission
           S_IWUSR    00200     owner has write permission
           S_IXUSR    00100     owner has execute permission
           S_IRWXG    00070     mask for group permissions
           S_IRGRP    00040     group has read permission
           S_IWGRP    00020     group has write permission
           S_IXGRP    00010     group has execute permission
           S_IRWXO    00007     mask for permissions for others (not in group)
           S_IROTH    00004     others have read permission
           S_IWOTH    00002     others have write permission
           S_IXOTH    00001     others have execute permission
*/
int display_file(char *path, char *filename)
{
/* The stat structure.  保存文件属性信息
struct stat {
    unsigned long   st_dev;         Device.  
    unsigned long   st_ino;        File serial number.  *
    unsigned int    st_mode;       * File mode.  *
    unsigned int    st_nlink;      * Link count.  *
    unsigned int    st_uid;        * User ID of the file's owner.  *
    unsigned int    st_gid;        * Group ID of the file's group. 
    unsigned long   st_rdev;       * Device number, if device.  *
    unsigned long   __pad1;
    long            st_size;       * Size of file, in bytes.  *
    int             st_blksize;    * Optimal block size for I/O.  *
    int             __pad2;
    long            st_blocks;     * Number 512-byte blocks allocated. *
    int             st_atime;      * Time of last access.  *
    unsigned int    st_atime_nsec;
    int             st_mtime;      * Time of last modification.  *
    unsigned int    st_mtime_nsec;
    int             st_ctime;      * Time of last status change.  *
    unsigned int    st_ctime_nsec;
    unsigned int    __unused4;
    unsigned int    __unused5;
};
*/
    struct stat st;//stat结构体
/* The passwd structure. 保存用户信息
struct passwd
{
  char *pw_name;          * Username.  *
  char *pw_passwd;        * Password.  *
  __uid_t pw_uid;         * User ID.  *
  __gid_t pw_gid;         * Group ID.  *
  char *pw_gecos;         * Real name.  *
  char *pw_dir;           * Home directory.  *
  char *pw_shell;         * Shell program.  *
};
*/
  struct passwd *pw;//passwd结构体

    int i;//
/* The group structure.     
struct group
  {
    char *gr_name;          Group name.    
    char *gr_passwd;        Password.    
    __gid_t gr_gid;         Group ID.    
    char **gr_mem;          Member list.    
  };
*/
    struct group *gr;//组结构体
/* Used by other time functions.  *
struct tm
{
  int tm_sec;            * Seconds.    [0-60] (1 leap second) *
  int tm_min;            * Minutes.    [0-59] *
  int tm_hour;           * Hours.    [0-23] *
  int tm_mday;           * Day.        [1-31] *
  int tm_mon;            * Month.    [0-11] *
  int tm_year;            * Year    - 1900.  *
  int tm_wday;            * Day of week.    [0-6] *
  int tm_yday;            * Days in year.[0-365]    *
  int tm_isdst;            * DST.        [-1/0/1]*

#ifdef    __USE_BSD
  long int tm_gmtoff;        * Seconds east of UTC.  *
  __const char *tm_zone;    * Timezone abbreviation.  *
#else
  long int __tm_gmtoff;        * Seconds east of UTC.  *
  __const char *__tm_zone;    * Timezone abbreviation.  *
#endif
}; */
   struct tm *tm;//tm结构体 表示时间
    stat(path, &st);//将文件信息赋值给结构体

    switch(st.st_mode & S_IFMT)   //确定文件类型
    {
    case S_IFREG:  printf("-");    break;
    case S_IFDIR:  printf("d");    break;
    case S_IFLNK:  printf("l");    break;
    case S_IFBLK:  printf("b");    break;
    case S_IFCHR:  printf("c");    break;
    case S_IFIFO:  printf("p");    break;
    case S_IFSOCK: printf("s");    break;
    }

    for(i = 8; i >= 0; i--)　//三位8进制表示权限777 一位代表2^3
    {    
        if(st.st_mode & (1 << i))
        {
            switch(i%3)
            {
            case 2: printf("r"); break;
            case 1: printf("w"); break;
            case 0: printf("x"); break;
            }
        }
        else
            printf("-");
    }

    pw = getpwuid(st.st_uid);//用户id
    gr = getgrgid(st.st_gid);//用户组id

    printf("%2d %s %s %4ld", st.st_nlink, pw->pw_name, gr->gr_name, st.st_size);
    //输出文件的路径，用户名，用户组，文件大小　

    tm = localtime(&st.st_ctime);
    printf(" %04d-%02d-%02d %02d:%02d",tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min);
　　//输出最近修改文件时间
    printf(" %s\n", filename);
　　//输出文件名//这里也可以根据文件类型变色
    return 0;
}

int display_dir(char *dirname)
{
    DIR *dir;
/*
	struct dirent
  	{
	#ifndef __USE_FILE_OFFSET64
    	__ino_t d_ino;
    	__off_t d_off;
	#else
    	__ino64_t d_ino;
    	__off64_t d_off;
	#endif
    	unsigned short int d_reclen;
    	unsigned char d_type;
    	char d_name[256];        * We must not include limits.h! *
 	};
*/
    struct dirent *dirent;
    struct stat st;
    char buf[1024];

    dir = opendir(dirname);
    while((dirent = readdir(dir)) != NULL)
    {
        strcpy(buf, dirname);//读取目录下的文件路径
        strcat(buf, "/");
        strcat(buf, dirent->d_name);//填充路径信息
        if(stat(buf, &st))//赋值
        {
            perror("stat");
            return -1;
        }

        if(dirent->d_name[0] != '.')
            display_file(buf, dirent->d_name);//递归调用
　　　　//是否还有else?此处只取一层
    }
}

int main(int argc, char **argv)
{
    struct stat st;
    char buf[1024];

    if(stat(argv[1], &st) < 0)
    {
        perror("stat");
        return -1;
    }
　　
    if((st.st_mode & S_IFMT) == S_IFDIR)
        display_dir(argv[1]);
    else
        display_file(argv[1], argv[1]);//0是程序名

    return 0;
}
