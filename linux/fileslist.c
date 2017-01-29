#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>

int readFileList()
{	
	DIR* dir ;   
	struct dirent* ptr ;
	//char base[1000];
	if((dir=opendir("./music"))==NULL)
	{
		perror("Open dir error...");
		exit(1);
	}
	
	while((ptr=readdir(dir))!=NULL)
	{
		///current dir OR parrent dir
		if(strcmp(ptr->d_name,".")==0||strcmp(ptr->d_name,"..")==0)
		continue ;
		///file
		else if(ptr->d_type==8)
		printf("name=%s \n",ptr->d_name);
		/*else if(ptr->d_type == 10)	///link file
			printf("d_name:%s/%s\n",basePath,ptr->d_name);
		else if(ptr->d_type == 4)	///dir
		{
			memset(base,'\0',sizeof(base));
			strcpy(base,basePath);
			strcat(base,"/");
			strcat(base,ptr->d_name);
			readFileList(base);
		}//*/
	}
	closedir(dir);
	return 1 ;
}

int main(void)
{
	readFileList();
	return 0 ;
}