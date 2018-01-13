#include "fb_v4l.h"


/*********************************************************************************************************
** Function name: get_grab_frame
** Descriptions: 获取图像帧，该函数调用了VIDIOCMCAPTURE的ioctl，获取一帧图片
** Input: *vd，参数指针
** 				frame,帧号
** Output : 无
** Created by:
** Created Date: 
**-------------------------------------------------------------------------------------------------------
** Modified by:
** Modified Date: 
**------------------------------------------------------------------------------------------------------
********************************************************************************************************/
int get_grab_frame(fb_v41 *vd, int frame)
{
   if (vd->frame_using[frame]) {
      fprintf(stderr, "get_grab_frame: frame %d is already used.\n", frame);
      return ERR_FRAME_USING;
   }

   vd->mmap.frame = frame;
   if (ioctl(vd->fd, VIDIOCMCAPTURE, &(vd->mmap)) < 0) {
      perror("v4l_grab_frame");
      return ERR_GET_FRAME;
   }
   vd->frame_using[frame] = 1;
   vd->frame_current = frame;
   return 0;
}

/*********************************************************************************************************
** Function name: get_next_frame
** Descriptions: 获取下一帧的图像
** Input: *vd ,参数指针
** Output : 无
** Created by:
** Created Date: 
**-------------------------------------------------------------------------------------------------------
** Modified by:
** Modified Date: 
**------------------------------------------------------------------------------------------------------
********************************************************************************************************/
int get_first_frame(fb_v41 *vd)
{
 	int ret;
  
	vd->frame_current = 0;
	ret = get_grab_frame( vd, 0 );
	if ( ret<0 )
		return ret;
	// 等待帧同步
	if (ioctl(vd->fd, VIDIOCSYNC, &(vd->frame_current)) < 0) 
	{
		perror("v4l_grab_sync");
      	return ERR_SYNC;
  	}
  	vd->frame_using[vd->frame_current] = 0 ;		
	return (0);
}

/*********************************************************************************************************
** Function name: get_next_frame
** Descriptions: 获取下一帧的图像
** Input: *vd ,参数指针
** Output : 返回0表示正常完成返回。
** Created by:
** Created Date: 
**-------------------------------------------------------------------------------------------------------
** Modified by:
** Modified Date: 
**------------------------------------------------------------------------------------------------------
********************************************************************************************************/
int get_next_frame(fb_v41 *vd)
{
	int ret;
	vd->frame_current ^= 1;
	ret = get_grab_frame( vd,vd->frame_current);			// 获取图像数据
	if( ret < 0 )
		return ret;
		
	if (ioctl(vd->fd, VIDIOCSYNC, &(vd->frame_current)) < 0) // 等待帧同步
	{   
		perror("v4l_grab_sync");
     	return ERR_SYNC;
  	}
  	vd->frame_using[vd->frame_current] = 0 ;
	return 0;	
}




/*********************************************************************************************************
** Function name: get_frame_address
** Descriptions: 获取帧地址.调用该函数可以获取当前帧的缓冲地址
** Input: *vd ,参数指针
** Output : 返回帧图像数据的指针地址.
** Created by:
** Created Date: 
**-------------------------------------------------------------------------------------------------------
** Modified by:
** Modified Date: 
**------------------------------------------------------------------------------------------------------
********************************************************************************************************/
unsigned char *get_frame_address(fb_v41 *vd)
{
	return (vd->map + vd->mbuf.offsets[vd->frame_current]); 	// 从MAP内存中找到当前帧的起始指针
}



/*********************************************************************************************************
** Function name: rgb_to_framebuffer
** Descriptions: 写图像数据到Framebuffer,使用该函数前必须成功执行open_framebuffer函数.
** Input: *vd ,参数指针
**				width,图像的宽度vd->mmap.width
**        height,图像高度
**        xoffset,图在Framebuffer X轴偏移量vd->vinfo.xoffset
**        yoffset,图在Framebuffer Y轴偏移量
**        *img_ptr,将写进FrameBuffer缓冲区指针
** Output : 无
** Created by:
** Created Date: 
**-------------------------------------------------------------------------------------------------------
** Modified by:
** Modified Date: 
**------------------------------------------------------------------------------------------------------
********************************************************************************************************/
void rgb_to_framebuffer( fb_v41 *vd,										// 
                         int width,int height,					// 图像大小
                         int xoffset,int yoffset,		    // 图像在Framebuffer偏移位置
                         unsigned short  *img_ptr )  // 图像数据指针
{
	int x,y;
	int location;
	unsigned short *loca_ptr;
	// Figure out where in memory to put the pixel

    for ( y = 0; y < height; y++ )				// 纵扫描
	{
		location = xoffset * 2 + (y + yoffset) * vd->finfo.line_length;	
		loca_ptr = (unsigned short *) (vd->fbp + location);	       	
		for ( x = 0; x < width; x++ ) 		// 行扫描		
		{
			*(loca_ptr + x) = *img_ptr++;
		}
	}
}


/*********************************************************************************************************
** Function name: open_framebuffer
** Descriptions: 该函数用于初始化FrameBuffer设备，在该函数中打开FrameBuffer设备，并将设备影射到内存
** Input: *ptr,打开Framebuffer设备路径指针
**        *vd ,参数指针
** Output : 返回非0值表示出错
** Created by:
** Created Date: 
**-------------------------------------------------------------------------------------------------------
** Modified by:
** Modified Date: 
**------------------------------------------------------------------------------------------------------
********************************************************************************************************/
int open_framebuffer(char *ptr,fb_v41 *vd)
{
	int fbfd,screensize;
    // Open the file for reading and writing
    fbfd = open( ptr, O_RDWR);
    if (fbfd < 0) 
    {
		printf("Error: cannot open framebuffer device.%x\n",fbfd);
		return ERR_FRAME_BUFFER;
    }
    printf("The framebuffer device was opened successfully.\n");
		
	vd->fbfd = fbfd;	// 保存打开FrameBuffer设备的句柄
	
    // Get fixed screen information	获取FrameBuffer固定不变的信息
    if (ioctl(fbfd, FBIOGET_FSCREENINFO, &vd->finfo)) 
    {
		printf("Error reading fixed information.\n");
		return ERR_FRAME_BUFFER;
    }

    // Get variable screen information 获取FrameBuffer屏幕可变的信息
    if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vd->vinfo)) 
    {
		printf("Error reading variable information.\n");
		return ERR_FRAME_BUFFER;
    }

    printf("%dx%d, %dbpp, xoffset=%d ,yoffset=%d \n", vd->vinfo.xres, 
    			vd->vinfo.yres, vd->vinfo.bits_per_pixel,vd->vinfo.xoffset,vd->vinfo.yoffset );

    // Figure out the size of the screen in bytes
    screensize = vd->vinfo.xres * vd->vinfo.yres * vd->vinfo.bits_per_pixel / 8;

    // Map the device to memory
    vd->fbp = (char *)mmap(0,screensize,PROT_READ|PROT_WRITE,MAP_SHARED,fbfd,0); // 影射Framebuffer设备到内存
    if ((int)vd->fbp == -1) 
    {
		printf("Error: failed to map framebuffer device to memory.\n");
		return ERR_FRAME_BUFFER;
    }
    printf("The framebuffer device was mapped to memory successfully.\n");
	return  0;
}

/*********************************************************************************************************
** Function name: open_video
** Descriptions: 通过该函数初始化视频设备
** Input: *fileptr，打开的文件名指针
** 				*vd，参数指针
** 				dep，像素深度
** 				pal，调色板
** 				width，宽度
** 				height，高度
** Output : 无
** Created by:
** Created Date: 
**-------------------------------------------------------------------------------------------------------
** Modified by:
** Modified Date: 
**------------------------------------------------------------------------------------------------------
********************************************************************************************************/
int open_video( char *fileptr,fb_v41 *vd ,int dep,int pal,int width,int height)
{
	// 打开视频设备
	if ((vd->fd = open(fileptr, O_RDWR)) < 0) 
	{
     	perror("v4l_open:");
		return ERR_VIDEO_OPEN;
	}
   // 获取设备
   if (ioctl(vd->fd, VIDIOCGCAP, &(vd->capability)) < 0) 
   {
   		perror("v4l_get_capability:");
      	return ERR_VIDEO_GCAP;
   }
   

	// 获取图象  
   	if (ioctl(vd->fd, VIDIOCGPICT, &(vd->picture)) < 0) 
   	{
    	perror("v4l_get_picture");
     	return ERR_VIDEO_GPIC;
	}
	// 设置图象
   	vd->picture.palette = pal;		// 调色板
   	vd->picture.depth = dep;			// 像素深度

   	vd->mmap.format =pal;
   	if (ioctl(vd->fd, VIDIOCSPICT, &(vd->picture)) < 0) 
   	{
		perror("v4l_set_palette");
      	return ERR_VIDEO_SPIC;
	}
   	vd->mmap.width = width; 		// width;
   	vd->mmap.height = height; 	// height;
   	vd->mmap.format = vd->picture.palette; 

   	vd->frame_current = 0;
   	vd->frame_using[0] = 0;
   	vd->frame_using[1] = 0;
   
   	// 获取缓冲影射信息
   	if (ioctl(vd->fd, VIDIOCGMBUF, &(vd->mbuf)) < 0) 
   	{
		perror("v4l_get_mbuf");
      	return -1;
   	}
   
   	// 建立设备内存影射
   	vd->map = mmap(0, vd->mbuf.size, PROT_READ|PROT_WRITE, MAP_SHARED, vd->fd, 0);
   	if ( vd->map < 0) 
   	{
		perror("v4l_mmap_init:mmap");
      	return -1;
   	}
	printf("The video device was opened successfully.\n");
  	// return get_first_frame(vd);
  	return 0;
}
