//arm-linux-gcc -o tcp-s -lpthread tcp-s.c
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/stat.h>
#include <iconv.h>
#include <dirent.h>
#include <fcntl.h>

#define SERVPORT 8580 /*server port number */
#define BACKLOG 10 /*max link*/
#define MAX_MESG_LEN 1024  /*max message len*/
#define MAX_MP3_NUMS 1000 /*max mp3 nums*/

//#define PTHREADTEST //多线程模式

struct mp3info
{
	//0 空 1存在
	int state;
	//文件名
	char filesname[50];
	//作者
	char author[50];
	//专辑
	char special[50];
	//时长
	char duration[50];
}musiclist[MAX_MP3_NUMS];
int musicnums=0;

void savelist()
{
	int i=0;
	FILE* fpsave=fopen("data.list","w");
	for(i=0;i<MAX_MP3_NUMS;i++)
	{
		if(musiclist[i].state==1)
		{
			fprintf(fpsave,"'%s' '%s' '%s' '%s'\n",
			musiclist[i].filesname,
			musiclist[i].author,
			musiclist[i].special,
			musiclist[i].duration);
		}
	}
	
	fclose(fpsave);
}

//编码转换
int code_convert(char *from_charset,char *to_charset,char *inbuf,int inlen,char *outbuf,int outlen)
{
	iconv_t cd;
	int rc;
	char **pin = &inbuf;
	char **pout = &outbuf;

	cd = iconv_open(to_charset,from_charset);
	if (cd==0)
		return -1;
	memset(outbuf,0,outlen);
	if (iconv(cd,pin,&inlen,pout,&outlen) == -1)
		return -1;
	iconv_close(cd);
	return 0;
}

//utf-8 to gb2312
int u2g(char *inbuf,int inlen,char *outbuf,int outlen)
{
	return code_convert("utf-8","gb2312",inbuf,inlen,outbuf,outlen);
}

//gb2312 to utf-8
int g2u(char *inbuf,size_t inlen,char *outbuf,size_t outlen)
{
	return code_convert("gb2312","utf-8",inbuf,inlen,outbuf,outlen);
}

//获取文件大小
double get_file_size(const char *path)
{
	double filesize = -1;
	struct stat statbuff;
	if(stat(path, &statbuff) < 0){
		return filesize;
	}else{
		filesize = statbuff.st_size;
	}
	return filesize;
}

//获取音乐列表
int commandlist(int client_fd)
{
	char tmpstr[MAX_MESG_LEN]="";
	char client_mesg[MAX_MESG_LEN]="";
	int i;
	sprintf(tmpstr,"%d\n",musicnums);
	if (send(client_fd, tmpstr, strlen(tmpstr)+1, 0) == -1)
	{
		printf("\tsend error!\n");
		//close(client_fd);
		return 1;
	}
	for(i=0;i<MAX_MP3_NUMS;i++)
	{
		if(musiclist[i].state==1)
		{
			sprintf(tmpstr,"'%s' '%s' '%s' '%s'\n",
			musiclist[i].filesname,
			musiclist[i].author,
			musiclist[i].special,
			musiclist[i].duration);
			u2g(tmpstr,strlen(tmpstr),client_mesg,MAX_MESG_LEN);
			if (send(client_fd, client_mesg, strlen(client_mesg)+1, 0) == -1)
			{
				printf("\tsend error!\n");
				//close(client_fd);
				return 1;
			}
			printf("\tsend: %s",tmpstr);
		}
	}
	return 0;
}

//下载文件
int commandget(int client_fd,char* filesname)
{
	char tmpstr[MAX_MESG_LEN]="";
	char client_mesg[MAX_MESG_LEN]="";
	sprintf(tmpstr,"./music/%s",filesname);
	double filessize=get_file_size(tmpstr);
	FILE* fpout=fopen(tmpstr,"rb");
	if(fpout==NULL)
	{
		printf("\tFile not found\n");
		send(client_fd, "NULL\n", strlen("NULL\n")+1, 0);
		//close(client_fd);
		return 1;
	}
	//printf("found ");
	
	sprintf(tmpstr,"%s\n",filesname);
	u2g(tmpstr,strlen(tmpstr),client_mesg,MAX_MESG_LEN);
	if (send(client_fd, client_mesg, strlen(client_mesg)+1, 0) == -1)
	{
		printf("\tsend error!\n");
		//close(client_fd);
		return 1;
	}
	
	sprintf(tmpstr,"%lf\n",filessize);
	printf("\tsize     :[%10.0lf]B\n",filessize);
	if (send(client_fd, tmpstr, strlen(tmpstr)+1, 0) == -1)
	{
		printf("\tsend error!\n");
		//close(client_fd);
		return 1;
	}

	char buffer[MAX_MESG_LEN];
	bzero(buffer,MAX_MESG_LEN);
	int length=0 ;
	double sendsum=0;
	int sendbit=0;
	for(;(length=fread(buffer,sizeof(char),MAX_MESG_LEN,fpout))>0;)
	{
		if((sendbit=send(client_fd,buffer,length,0))<0)
		{
			printf("\tsend error!\n");
			//close(client_fd);
			return 1;
		}
		sendsum+=sendbit;
		printf("\tsend Bits:[%10.0lf/%10.0lf]B\r",sendsum,filessize);

		bzero(buffer,MAX_MESG_LEN);
	}
	printf("\tsend Bits:[%10.0lf/%10.0lf]B\n",sendsum,filessize);
	fclose(fpout);
	
	return 0;
}

//上传文件
int commandput(int client_fd,char* filesname,double filessize)
{
	char tmpstr[MAX_MESG_LEN]="";
	char client_mesg[MAX_MESG_LEN]="";
	sprintf(tmpstr,"./music/%s",filesname);
	if((access(tmpstr,F_OK))!=-1)
	{
		printf("\tFile already exist\n");
		if (send(client_fd, "REPETITION\n", strlen("REPETITION\n")+1, 0) == -1)
		{
			printf("\tsend error!\n");
			//close(client_fd);
			return 1;
		}
		return 1;
	}
	
	if (send(client_fd, "OK\n", strlen("OK\n")+1, 0) == -1)
	{
		printf("\tsend error!\n");
		//close(client_fd);
		return 1;
	}

	FILE *fpput = fopen(tmpstr, "w");
	if (fpput == NULL)
	{
		printf("File:\t%s Can Not Write!\n", filesname);
		return 1;
	}

	char buffer[MAX_MESG_LEN];
	bzero(buffer,MAX_MESG_LEN);
	int length=0 ;
	double recsum=0;
	int recbit=0;
	for(;(length = recv(client_fd, buffer, MAX_MESG_LEN, 0))>0;)
	{
		recbit = fwrite(buffer, sizeof(char), length, fpput);

		recsum+=recbit;
		printf("\trec Bits :[%10.0lf/%10.0lf]B\r",recsum,filessize);

		bzero(buffer, MAX_MESG_LEN);
	}

	printf("\trec Bits :[%10.0lf/%10.0lf]B\n",recsum,filessize);

	fclose(fpput);
	
	return 0;
}

//删除文件
int commanddel(int client_fd,char* filesname)
{
	char tmpstr[MAX_MESG_LEN]="";
	char client_mesg[MAX_MESG_LEN]="";
	int i=0;
	for(i=0;i<MAX_MP3_NUMS;i++)
	{
		if(musiclist[i].state==1
		&&strcmp(musiclist[i].filesname,filesname)==0)
		{
			//bug201701291543 赋值写成等于了
			musiclist[i].state=0;
			musicnums--;
			savelist();
			sprintf(tmpstr,"./music/%s",filesname);
			//printf("\t[%s]\n",tmpstr);
			if(remove(tmpstr)==0)
			{
				printf("\tDEL SUCCEED\n");
			}
			else
			{
				perror("remove");
			}
			if (send(client_fd, "SUCCEED\n", strlen("SUCCEED\n")+1, 0) == -1)
			{
				printf("\tsend error!\n");
				//close(client_fd);
				return 1;
			}
			break;
		}
	}
	if(i==MAX_MP3_NUMS)
	{
		printf("\tFile not found\n");
	}
	return 0;
}

//工作线程
void thfuniction(int* sockfd)
{
	char command[10]="";
	char filesname[50]="";
	double filessize;
	char author[50]="未知";
	char special[50]="无";
	char duration[50]="00:00";
	int client_fd=*sockfd; /*sockfd:监听socket;client_fd:数据传输socket */
	char client_mesg[MAX_MESG_LEN];
	char tmpstr[MAX_MESG_LEN]="";
	int i=0;
	
	/* 子进程代码段 */
	if(recv(client_fd, client_mesg, MAX_MESG_LEN, 0) == -1)
	{
		close(client_fd);
	}
	//printf("rec:[%s]\n",client_fd,client_mesg);
	sscanf(client_mesg,"%s '%[^\']' '%[^\']' '%[^\']' '%[^\']' '%[^\']'",command,filesname,author,special,duration,tmpstr);
	filessize=atof(tmpstr);

	if(strcmp(command,"LIST")==0)
	{
		printf("\tcommand  :[%s]\n",command);
		commandlist(client_fd);
	}
	else if(strcmp(command,"GET")==0)
	{
		g2u(filesname,strlen(filesname),tmpstr,MAX_MESG_LEN);
		strcpy(filesname,tmpstr);
		printf("\
\tcommand  :[%s]\n\
\tfilesname:[%s]\n",
command,filesname);
		commandget(client_fd,filesname);
	}
	else if(strcmp(command,"PUT")==0)
	{
		g2u(filesname,strlen(filesname),tmpstr,MAX_MESG_LEN);
		strcpy(filesname,tmpstr);
		g2u(author,strlen(author),tmpstr,MAX_MESG_LEN);
		strcpy(author,tmpstr);
		g2u(special,strlen(special),tmpstr,MAX_MESG_LEN);
		strcpy(special,tmpstr);
		g2u(duration,strlen(duration),tmpstr,MAX_MESG_LEN);
		strcpy(duration,tmpstr);
		printf("\
\tcommand  :[%s]\n\
\tfilesname:[%s]\n\
\tauthor   :[%s]\n\
\tspecial  :[%s]\n\
\tduration :[%s]\n\
\tsize     :[%.0lf]B\n",
command,filesname,author,special,duration,filessize);

		for(i=0;i<MAX_MP3_NUMS;i++)
		{
			if(musiclist[i].state==0)
				break;
		}
		if(i<MAX_MP3_NUMS)
		{
			if(commandput(client_fd,filesname,filessize)==0)
			{
				musiclist[i].state=1;
				strcpy(musiclist[i].filesname,filesname);
				strcpy(musiclist[i].author,author);
				strcpy(musiclist[i].special,special);
				strcpy(musiclist[i].duration,duration);
				musicnums++;
				savelist();
			}
		}
		else
		{
			if (send(client_fd, "NOT ENOUGH\n", strlen("NOT ENOUGH\n")+1, 0) == -1)
			{
				printf("\tsend error!\n");
				//close(client_fd);
				//return 1;
			}
		}
	}
	else if(strcmp(command,"DEL")==0)
	{
		g2u(filesname,strlen(filesname),tmpstr,MAX_MESG_LEN);
		strcpy(filesname,tmpstr);
		printf("\
\tcommand  :[%s]\n\
\tfilesname:[%s]\n",
command,filesname);
		commanddel(client_fd,filesname);
	}
	else
	{
		printf("\tcommand not found\n");
		send(client_fd, "NULL\n", strlen("NULL\n")+1, 0);
	}

	printf("\tsocket close\n");
	close(client_fd);
}

//初始化
int init()
{
	system("echo encode:$LANG");
	printf("load data\n");
	if(opendir("./music")==NULL)
	{
		printf("create music folder\n");
		mkdir("./music",0755);
	}

	int i;
	for(i=0;i<MAX_MP3_NUMS;i++)
	{
		musiclist[i].state=0;
	}
	
	char tmpstr[MAX_MESG_LEN]="";
	FILE* fpinit=fopen("data.list","r");
	if(fpinit!=NULL)
	{
		for(musicnums=0;fgets(tmpstr,MAX_MESG_LEN,fpinit)!=NULL;musicnums++)
		{
			sscanf(tmpstr,"'%[^\']' '%[^\']' '%[^\']' '%[^\']'",
			musiclist[musicnums].filesname,
			musiclist[musicnums].author,
			musiclist[musicnums].special,
			musiclist[musicnums].duration);
			musiclist[musicnums].state=1;
		}
		fclose(fpinit);
	}
	printf("[%4d] CMF\n",musicnums);

	for(i=0;i<musicnums;i++)
	{
		printf("%2d: [%12s] [%5s] [%5s] [%5s] [%d]\n",
		i,
		musiclist[i].filesname,
		musiclist[i].author,
		musiclist[i].special,
		musiclist[i].duration,
		musiclist[i].state);
	}
	

	printf("load completed\n");
	return 0;
}

int main()
{
	pthread_t id;
	int ret;
	int sin_size;
	int sockfd;
	int client_fd; /*sockfd:监听socket;client_fd:数据传输socket */
	char client_mesg[MAX_MESG_LEN];
	struct sockaddr_in my_addr; /* load address message */
	struct sockaddr_in remote_addr; /* client address message */

	init();
	
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror( "init socket error:\n"); 
		exit(1);
	}
	printf("init socket\n");
	my_addr.sin_family=AF_INET;
	my_addr.sin_port=htons(SERVPORT);
	my_addr.sin_addr.s_addr = INADDR_ANY;
	bzero( &(my_addr.sin_zero),8);

	// 设置套接字选项避免地址使用错误
	int on=1;
	if((setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on)))<0)
	{
		perror("set socket option error:");
		exit(1);
	}
	printf("set socket option\n");
	if(bind(sockfd, (struct sockaddr *) &my_addr, sizeof(struct sockaddr))==-1)
	{
		perror( "bind error: ");
		exit(1);
	}
	printf("bind\n");

	#ifdef PTHREADTEST
		printf("multithreading mode\n");
	#else
		printf("single thread mode\n");
	#endif
	
	if (listen(sockfd, BACKLOG) == -1)
	{
		perror( "listen error:\n");
		exit(1);
	}
	printf("listen:\n");
	
	sin_size = sizeof(struct sockaddr_in);
	int clientid=1;
	for(;;clientid++)
	{
		if ((client_fd = accept(sockfd, (struct sockaddr *) &remote_addr, (socklen_t *)&sin_size)) == -1)
		{
			perror( "connect error, client socket closed:\n");
			continue;
		}
		printf("[%04d-%s:%2d] connect\n",clientid,inet_ntoa(remote_addr.sin_addr),client_fd);

		#ifdef PTHREADTEST
			//多线程
			if(pthread_create(&id,NULL,(void *)thfuniction,(void*)&client_fd)!=0)// 成功返回0，错误返回错误编号
			{
				printf ("create thread error!\n");
				close(client_fd);
			}
		#else
			//单线程
			thfuniction(&client_fd);
		#endif
	}
	printf("close server\n");
	return 0;
}
