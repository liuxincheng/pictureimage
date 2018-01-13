//glibc
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/mman.h>
#include<string.h>
//kernel 2.6.29
#include <linux/videodev.h>
#include <linux/fb.h>


 
#define ERR_FRAME_BUFFER	1
#define ERR_VIDEO_OPEN		2
#define ERR_VIDEO_GCAP		3
#define ERR_VIDEO_GPIC		4
#define ERR_VIDEO_SPIC		5
#define ERR_SYNC			6
#define ERR_FRAME_USING	7
#define ERR_GET_FRAME		8

typedef struct	_fb_v4l
{
	int		fbfd ;                                                                                               
    struct fb_var_screeninfo vinfo;               
    struct fb_fix_screeninfo finfo;               
    char 	*fbp;                                                                                               
    int 	fd;                                                                                                                  
	struct	video_capability capability;       
	struct	video_buffer 	buffer;                       
	struct	video_window 	window;                       
	struct	video_channel 	channel[8];       
	struct	video_picture 	picture;                
	struct	video_tuner 	tuner;                       
	struct	video_audio 	audio[8];               
	struct	video_mmap 		mmap;                                
	struct	video_mbuf 		mbuf;                                
	unsigned char *map;                                                                       
	int 	frame_current;
	int 	frame_using[VIDEO_MAX_FRAME];
}fb_v41;


#define DEFAULT_PALETTE  VIDEO_PALETTE_RGB565

#define FB_FILE 	"/dev/fb0"
#define V4L_FILE	"/dev/video0"
