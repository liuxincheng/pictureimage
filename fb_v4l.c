#include "fb_v4l.h"


/*********************************************************************************************************
** Function name: get_grab_frame
** Descriptions: ��ȡͼ��֡���ú���������VIDIOCMCAPTURE��ioctl����ȡһ֡ͼƬ
** Input: *vd������ָ��
** 				frame,֡��
** Output : ��
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
** Descriptions: ��ȡ��һ֡��ͼ��
** Input: *vd ,����ָ��
** Output : ��
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
	// �ȴ�֡ͬ��
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
** Descriptions: ��ȡ��һ֡��ͼ��
** Input: *vd ,����ָ��
** Output : ����0��ʾ������ɷ��ء�
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
	ret = get_grab_frame( vd,vd->frame_current);			// ��ȡͼ������
	if( ret < 0 )
		return ret;
		
	if (ioctl(vd->fd, VIDIOCSYNC, &(vd->frame_current)) < 0) // �ȴ�֡ͬ��
	{   
		perror("v4l_grab_sync");
     	return ERR_SYNC;
  	}
  	vd->frame_using[vd->frame_current] = 0 ;
	return 0;	
}




/*********************************************************************************************************
** Function name: get_frame_address
** Descriptions: ��ȡ֡��ַ.���øú������Ի�ȡ��ǰ֡�Ļ����ַ
** Input: *vd ,����ָ��
** Output : ����֡ͼ�����ݵ�ָ���ַ.
** Created by:
** Created Date: 
**-------------------------------------------------------------------------------------------------------
** Modified by:
** Modified Date: 
**------------------------------------------------------------------------------------------------------
********************************************************************************************************/
unsigned char *get_frame_address(fb_v41 *vd)
{
	return (vd->map + vd->mbuf.offsets[vd->frame_current]); 	// ��MAP�ڴ����ҵ���ǰ֡����ʼָ��
}



/*********************************************************************************************************
** Function name: rgb_to_framebuffer
** Descriptions: дͼ�����ݵ�Framebuffer,ʹ�øú���ǰ����ɹ�ִ��open_framebuffer����.
** Input: *vd ,����ָ��
**				width,ͼ��Ŀ��vd->mmap.width
**        height,ͼ��߶�
**        xoffset,ͼ��Framebuffer X��ƫ����vd->vinfo.xoffset
**        yoffset,ͼ��Framebuffer Y��ƫ����
**        *img_ptr,��д��FrameBuffer������ָ��
** Output : ��
** Created by:
** Created Date: 
**-------------------------------------------------------------------------------------------------------
** Modified by:
** Modified Date: 
**------------------------------------------------------------------------------------------------------
********************************************************************************************************/
void rgb_to_framebuffer( fb_v41 *vd,										// 
                         int width,int height,					// ͼ���С
                         int xoffset,int yoffset,		    // ͼ����Framebufferƫ��λ��
                         unsigned short  *img_ptr )  // ͼ������ָ��
{
	int x,y;
	int location;
	unsigned short *loca_ptr;
	// Figure out where in memory to put the pixel

    for ( y = 0; y < height; y++ )				// ��ɨ��
	{
		location = xoffset * 2 + (y + yoffset) * vd->finfo.line_length;	
		loca_ptr = (unsigned short *) (vd->fbp + location);	       	
		for ( x = 0; x < width; x++ ) 		// ��ɨ��		
		{
			*(loca_ptr + x) = *img_ptr++;
		}
	}
}


/*********************************************************************************************************
** Function name: open_framebuffer
** Descriptions: �ú������ڳ�ʼ��FrameBuffer�豸���ڸú����д�FrameBuffer�豸�������豸Ӱ�䵽�ڴ�
** Input: *ptr,��Framebuffer�豸·��ָ��
**        *vd ,����ָ��
** Output : ���ط�0ֵ��ʾ����
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
		
	vd->fbfd = fbfd;	// �����FrameBuffer�豸�ľ��
	
    // Get fixed screen information	��ȡFrameBuffer�̶��������Ϣ
    if (ioctl(fbfd, FBIOGET_FSCREENINFO, &vd->finfo)) 
    {
		printf("Error reading fixed information.\n");
		return ERR_FRAME_BUFFER;
    }

    // Get variable screen information ��ȡFrameBuffer��Ļ�ɱ����Ϣ
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
    vd->fbp = (char *)mmap(0,screensize,PROT_READ|PROT_WRITE,MAP_SHARED,fbfd,0); // Ӱ��Framebuffer�豸���ڴ�
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
** Descriptions: ͨ���ú�����ʼ����Ƶ�豸
** Input: *fileptr���򿪵��ļ���ָ��
** 				*vd������ָ��
** 				dep���������
** 				pal����ɫ��
** 				width�����
** 				height���߶�
** Output : ��
** Created by:
** Created Date: 
**-------------------------------------------------------------------------------------------------------
** Modified by:
** Modified Date: 
**------------------------------------------------------------------------------------------------------
********************************************************************************************************/
int open_video( char *fileptr,fb_v41 *vd ,int dep,int pal,int width,int height)
{
	// ����Ƶ�豸
	if ((vd->fd = open(fileptr, O_RDWR)) < 0) 
	{
     	perror("v4l_open:");
		return ERR_VIDEO_OPEN;
	}
   // ��ȡ�豸
   if (ioctl(vd->fd, VIDIOCGCAP, &(vd->capability)) < 0) 
   {
   		perror("v4l_get_capability:");
      	return ERR_VIDEO_GCAP;
   }
   

	// ��ȡͼ��  
   	if (ioctl(vd->fd, VIDIOCGPICT, &(vd->picture)) < 0) 
   	{
    	perror("v4l_get_picture");
     	return ERR_VIDEO_GPIC;
	}
	// ����ͼ��
   	vd->picture.palette = pal;		// ��ɫ��
   	vd->picture.depth = dep;			// �������

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
   
   	// ��ȡ����Ӱ����Ϣ
   	if (ioctl(vd->fd, VIDIOCGMBUF, &(vd->mbuf)) < 0) 
   	{
		perror("v4l_get_mbuf");
      	return -1;
   	}
   
   	// �����豸�ڴ�Ӱ��
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
