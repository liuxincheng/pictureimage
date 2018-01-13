//glibc
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/mman.h>
#include<string.h>
//kernel 2.6.29
#include <FreeImage.h>



typedef struct _image{
	FIBITMAP *bitmap;
	BYTE * bitmapbuf;
	int bitmapwidth;
	int bitmapheight;
	int	bitmapbpp;
	int	bitmapbpl;
	char *bitmapdir;
	
} image; 

