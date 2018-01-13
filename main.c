#include "image.h"
#include "fb_v4l.h"

int main(int argc,char **argv)
{
	fb_v41 vd;
	image rgbimage;
	int ret;
	char *filename;

	if(*(argv+1) == NULL){
		printf("image dir is NULL\n");
		return 0;
	}
	
	ret = open_framebuffer(FB_FILE, &vd);// 
	if( 0!= ret )
		return 0;
	printf("image file name is %s\n", *(argv+1));
	while(1)
	{
		if(dir_open(*(argv+1)) == -1)
			return 0;
		
		while(filename = dir_read(), filename != NULL)
		{
			if(image_type_check(filename) == -1)
				continue;
			rgbimage.bitmapdir = filename;
			ret = image_to_rgb565(&rgbimage);
			if(ret != 0)
				return 0;
			memset(vd.fbp, 0x00, vd.vinfo.xres*vd.vinfo.yres*vd.vinfo.bits_per_pixel/8);
		
			rgb_to_framebuffer(&vd, rgbimage.bitmapwidth, rgbimage.bitmapheight, 0, 0, (unsigned short*)rgbimage.bitmapbuf);//
			free(rgbimage.bitmapbuf);
			sleep(3);

		}
	}
	close(vd.fbfd);//
	return 0;
}




