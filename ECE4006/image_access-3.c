#include<stdio.h>

#define IMG_SIZE 310000
#define BDEBUG 1
#define WIDTH 10
#define HEIGHT 10
#define COLOR 3

typedef struct bitmap{
	char id[2];
	short planes, bpp;
	int fileSize, reserve, offset, hsize, width, height, comp, dataSize;
	int hres, vres, colors, impclrs, pallette[0x100], image[HEIGHT][WIDTH][COLOR];


}Bitmap;

int main()
{
	unsigned char buf_id[2], buf_fileSize[4], buf_reserve[4], buf_offset[4], buf_hsize[4];
	unsigned char buf_width[4], buf_height[4], buf_planes[2], buf_bpp[2], buf_comp[4];
	unsigned char buf_dataSize[4], buf_hres[4], buf_vres[4], buf_colors[4], buf_impclrs[4];
	unsigned char buf_pallette [0x100];
	unsigned char buf_image[WIDTH*HEIGHT*COLOR];
	unsigned char output[] = "img2.bmp";

	int i,j,k,l, kp, junkBytes;

	Bitmap b;

	size_t size = 1;//sizeof(unsigned char);

	FILE *ifp, *ofp;

	ifp = fopen("small.bmp","rb");
	//fread(Header,1,0x436,ifp);
	if(ifp != NULL)
	{
	//read bitmap data into buffers
	fread(buf_id,size, 2, ifp);
	fread(buf_fileSize,size, 4, ifp);
	fread(buf_reserve,size, 4, ifp);
	fread(buf_offset,size, 4, ifp);
	fread(buf_hsize,size, 4, ifp);
	fread(buf_width,size, 4, ifp);
	fread(buf_height,size, 4, ifp);
	fread(buf_planes,size, 2, ifp);
	fread(buf_bpp,size, 2, ifp);
	fread(buf_comp,size, 4, ifp);
	fread(buf_dataSize,size, 4, ifp);
	fread(buf_hres,size, 4, ifp);
	fread(buf_vres,size, 4, ifp);
	fread(buf_colors,size, 4, ifp);
	fread(buf_impclrs,size, 4, ifp);
	//fread(buf_pallette,size, 0x100, ifp);
	fread(buf_image, size, 320, ifp);
/*
	fseek(ifp,0x436,SEEK_SET);
	printf("made it to read\n");
	fread(image,8,400*300*3,ifp);
	printf("past read\n");
	ofp = fopen(output, "w");
	fwrite(Header,1,0x436,ofp);
	printf("2nd fwrite");
	fwrite(image,8,400*300*3,ofp);
	fclose(ofp);
	fclose(ifp);
*/

	//read buffer data into useable variables
	b.id[0] = buf_id[0];
	b.id[1] = buf_id[1];
	b.fileSize = (buf_fileSize[0])+(buf_fileSize[1]<<8)+(buf_fileSize[2]<<16)+(buf_fileSize[3]<<24);
	b.reserve = (buf_reserve[0])+(buf_reserve[1]<<8)+(buf_reserve[2]<<16)+(buf_reserve[3]<<24);
	b.offset = (buf_offset[0])+(buf_offset[1]<<8)+(buf_offset[2]<<16)+(buf_offset[3]<<24);
	b.hsize = (buf_hsize[0])+(buf_hsize[1]<<8)+(buf_hsize[2]<<16)+(buf_hsize[3]<<24);
	b.width = (buf_width[0])+(buf_width[1]<<8)+(buf_width[2]<<16)+(buf_width[3]<<24);
	b.height = (buf_height[0])+(buf_height[1]<<8)+(buf_height[2]<<16)+(buf_height[3]<<24);
	b.planes = buf_planes[0]+(buf_planes[1]<<4);
	b.bpp = buf_bpp[0]+(buf_bpp[1]<<4);
	b.comp = (buf_comp[0])+(buf_comp[1]<<8)+(buf_comp[2]<<16)+(buf_comp[3]<<24);
	b.dataSize = (buf_dataSize[0])+(buf_dataSize[1]<<8)+(buf_dataSize[2]<<16)+(buf_dataSize[3]<<24);
	b.hres = (buf_hres[0])+(buf_hres[1]<<8)+(buf_hres[2]<<16)+(buf_hres[3]<<24);
	b.vres = (buf_vres[0])+(buf_vres[1]<<8)+(buf_vres[2]<<16)+(buf_vres[3]<<24);
	b.colors = (buf_colors[0])+(buf_colors[1]<<8)+(buf_colors[2]<<16)+(buf_colors[3]<<24);
	b.impclrs = (buf_impclrs[0])+(buf_impclrs[1]<<8)+(buf_impclrs[2]<<16)+(buf_impclrs[3]<<24);



			#if BDEBUG
				printf("ID: %c%c\n", b.id[0], b.id[1]);
				printf("File Size: %d\n", b.fileSize);
				printf("Reserve: %d\n", b.reserve);
				printf("Offset: %d\n", b.offset);
				printf("Header size: %d\n", b.hsize);
				printf("Image Width: %d\n", b.width);
				printf("Image Height: %d\n", b.height);
				printf("Planes: %d\n", b.planes);
				printf("Bits/pixel: %d\n", b.bpp);
				printf("Compression: %d\n", b.comp);
				printf("Data size: %d\n", b.dataSize);
				printf("Horizontal resolution: %d\n", b.hres);
				printf("Vertical resolution: %d\n", b.vres);
				printf("Colors: %d\n", b.colors);
				printf("Important colors: %d\n", b.impclrs);
			#endif

	}
	else
		printf("No such file exists\n");



	/*for(i = 0; i < 0x400; i+=4){
		b.pallette[i/4]=(buf_pallette[i])+(buf_pallette[i+1]<<8)+(buf_pallette[i+2]<<16)+(buf_pallette[i+3]<<24);
	}

	#if 0
		for(i = 0; i< 0x100; ++i)
				printf("pallette at %x: %x\n", i,b.pallette[i]);
	#endif
*/
	junkBytes = 4-((b.width)*3)%4;
	printf("Junk bytes: %d\n", junkBytes);
	j = i = 0;

	for(k = 0; k < (b.width*3+junkBytes)*b.height; ++k){
		if(k%32 < 3*b.width){
			if(k%32 == 0)
				j=0;
			if(k%32 == 0 && k > 0)
				++i;
			switch(k%3){
			case 0:
				b.image[i][j][2] = buf_image[k];
				//printf("buf_image: %x\n", buf_image[k]);
				//printf("image at %d,%d:%x\n",i,j, b.image[i][j][2]);
				//printf("image at i,j,k= %d,%d,%d: %x\n", i, j, k, buf_image[k]);
				break;
			case 1:
				b.image[i][j][1] = buf_image[k];
				//printf("buf_image: %x\n", buf_image[k]);
				//printf("image at %d,%d:%x\n",i,j, b.image[i][j][1]);
				//printf("image at i,j,k= %d,%d,%d: %x\n", i, j, k, buf_image[k]);
				break;
			case 2:
				b.image[i][j][0] = buf_image[k];
				//printf("buf_image: %x\n", buf_image[k]);
				//printf("image at %d,%d:%x\n",i,j, b.image[i][j][0]);
				//printf("image at i,j,k= %d,%d,%d: %x\n", i, j, k, buf_image[k]);
				break;
			default:
				printf("error in image read\n");
			}

			printf("image at %d,%d:%x\n",i,j, b.image[i][j][0]+(b.image[i][j][1]<<8)+(b.image[i][j][2]<<16));
			++j;
			if(k%32 == 0){
				printf("Press any key to continue\n");
				getchar();
			}

			}
		//if(k<10)
		//	printf("Image at %d,%d: %x\n", i,j, b.image[i][j][2]);//+(b.image[i][j][1]<<8)+(b.image[i][j][0]<<16));
		}
/*		for(i = 0; i < 320; i+=4)
		printf("Image at %d: %x\n",i, buf_image[i]+(buf_image[i+1]<<8)+(buf_image[i+2]<<16));

				/*		for(i = 0; i <WIDTH*HEIGHT*COLOR; ++i){
			switch (i%4){
			case 0:
				b.image[

			for(j = 0; j < HEIGHT; ++j){
				for(k = 0; k < COLOR; ++k){
					image[i][j][k]=buf_image[i][j][k]buf_image[i+1]<<8)+(buffer[i+2]<<16)+(buffer[i+3]<<24);
		}

#if BDEBUG
		for(i = 0; i< 0x100; ++i)
				printf("Image: %x\n", image[i]);
	#endif
}*/


	return 0;
}
