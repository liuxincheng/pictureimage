//glibc
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/mman.h>
#include<string.h>

#include<sys/types.h>
#include<dirent.h>
#include<unistd.h>

DIR * dir;
struct dirent * ptr;


const char * suffix[] ={".jpg", ".bmp", ".gif"};
#define		suffix_max		3

int dir_open(char * dir_path)
{
	dir =opendir(dir_path);
	if(dir == NULL)
	{
		printf("can not open %s\n", dir_path);
		return -1;
	}
	return 0;
}
char* dir_read()
{
	do
	{
		ptr = readdir(dir);
		if(ptr == NULL)
		{
			printf("dir is end\n");
			closedir(dir);
			return NULL;
		}
	}while(!strcmp(ptr->d_name, ".") || !strcmp(ptr->d_name, ".."));
	printf("d_name: %s\n", ptr->d_name);
	return ptr->d_name;
}

int	image_type_check(char * image_name)
{
	int suffix_num;
	while(*image_name)
	{
		if(*image_name == '.')
		{
			for(suffix_num = 0; suffix_num < suffix_max; suffix_num++)
			{
				if(!strcmp(image_name, suffix[suffix_num]))
				{
					printf("suffix = %s\n", suffix[suffix_num]);
					return 0;
				}
			}
		}
		image_name++;
	}
	return -1;
}
 
