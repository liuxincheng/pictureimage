#include "image.h"

int image_to_rgb565(image *rgbimage)
{
 	FIBITMAP *dib;

	dib = FreeImage_Load(FreeImage_GetFileType(rgbimage->bitmapdir, 0), rgbimage->bitmapdir, 0);
	if(dib == NULL)
		goto error;

	rgbimage->bitmap = FreeImage_ConvertTo16Bits565(dib);
	FreeImage_Unload(dib);
	if(rgbimage->bitmap != NULL){
		rgbimage->bitmapbpp = FreeImage_GetBPP(rgbimage->bitmap);
		rgbimage->bitmapwidth = FreeImage_GetWidth(rgbimage->bitmap);
		rgbimage->bitmapheight = FreeImage_GetHeight(rgbimage->bitmap);
		rgbimage->bitmapbpl = FreeImage_GetPitch(rgbimage->bitmap);
		rgbimage->bitmapbuf = (BYTE*)malloc(rgbimage->bitmapbpl*rgbimage->bitmapheight);

		printf("rgbbpp=%d,%dx%d,rgbbpl=%d\n", rgbimage->bitmapbpp, 
			rgbimage->bitmapwidth, rgbimage->bitmapheight, rgbimage->bitmapbpl);

		FreeImage_ConvertToRawBits(rgbimage->bitmapbuf, rgbimage->bitmap, rgbimage->bitmapbpl, 16,
			FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK, TRUE);

		//FreeImage_Save(FIF_JPEG, FreeImage_ConvertTo24Bits(rgbimage->bitmapbuf), outputimg, 0);
		FreeImage_Unload(rgbimage->bitmap);

	}
	else
		goto error;
	return 0;
error:
	printf("image to rgb565 error\n");
	return -1;

}


