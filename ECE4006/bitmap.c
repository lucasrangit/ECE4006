#include "ece4006header.h"

#undef BDEBUG
#define BDEBUG 0

/*
int main()
{

	Bitmap bm;
	Bitmap *b = &bm;

	int i,j;
	unsigned char skinmask[HEIGHT][WIDTH];


	//printf("bitmap has been successfully created\n");

	readBMP(b, "img.bmp");
	//printf("bitmap has been successfully read\n");

	//printBMP(b);

	//printBMP(b);

	for(i = 0; i < HEIGHT; ++i){
		for( j = 0; j < WIDTH; ++j){
			skinmask[i][j] = skin_tone_filter(b->image[i][j][0], b->image[i][j][1]);
			if(!skinmask[i][j]){
				b->image[i][j][0]=0;
				b->image[i][j][1]=0;
				b->image[i][j][2]=0;
			}
			else{
				b->image[i][j][0]=255;
				b->image[i][j][1]=255;
				b->image[i][j][2]=255;
			}
		}
	}

	//printf("image has been masked\n");
	writeBMP(b, "img_out.bmp");

	return 0;
}
*/

//readBMP takes in a file pointer to a bitmap file and outputs a Bitmap
void readBMP(Bitmap *b,char *input){
unsigned char buf_id[2], buf_fileSize[4], buf_reserve[4], buf_offset[4], buf_hsize[4];
	unsigned char buf_width[4], buf_height[4], buf_planes[2], buf_bpp[2], buf_comp[4];
	unsigned char buf_dataSize[4], buf_hres[4], buf_vres[4], buf_colors[4], buf_impclrs[4];
	//unsigned char buf_pallette [0x100];
	unsigned char buf_image[WIDTH*HEIGHT*COLOR+JUNKBYTES];


	unsigned int i,j,k,junkBytes, bfimgsize, bpr;




	size_t size = 1;//sizeof(unsigned char);

	FILE *ifp;

	//printf("read variables assigned successfully\n");

	ifp = fopen(input, "rb");

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

	//printf("read buffers passed\n");

	//create header of bitmap
	b->id[0] = buf_id[0];
	b->id[1] = buf_id[1];
	b->fileSize = (buf_fileSize[0])+(buf_fileSize[1]<<8)+(buf_fileSize[2]<<16)+(buf_fileSize[3]<<24);
	b->reserve = (buf_reserve[0])+(buf_reserve[1]<<8)+(buf_reserve[2]<<16)+(buf_reserve[3]<<24);
	b->offset = (buf_offset[0])+(buf_offset[1]<<8)+(buf_offset[2]<<16)+(buf_offset[3]<<24);
	b->hsize = (buf_hsize[0])+(buf_hsize[1]<<8)+(buf_hsize[2]<<16)+(buf_hsize[3]<<24);
	b->width = (buf_width[0])+(buf_width[1]<<8)+(buf_width[2]<<16)+(buf_width[3]<<24);
	b->height = (buf_height[0])+(buf_height[1]<<8)+(buf_height[2]<<16)+(buf_height[3]<<24);
	b->planes = buf_planes[0]+(buf_planes[1]<<4);
	b->bpp = buf_bpp[0]+(buf_bpp[1]<<4);
	b->comp = (buf_comp[0])+(buf_comp[1]<<8)+(buf_comp[2]<<16)+(buf_comp[3]<<24);
	b->dataSize = (buf_dataSize[0])+(buf_dataSize[1]<<8)+(buf_dataSize[2]<<16)+(buf_dataSize[3]<<24);
	b->hres = (buf_hres[0])+(buf_hres[1]<<8)+(buf_hres[2]<<16)+(buf_hres[3]<<24);
	b->vres = (buf_vres[0])+(buf_vres[1]<<8)+(buf_vres[2]<<16)+(buf_vres[3]<<24);
	b->colors = (buf_colors[0])+(buf_colors[1]<<8)+(buf_colors[2]<<16)+(buf_colors[3]<<24);
	b->impclrs = (buf_impclrs[0])+(buf_impclrs[1]<<8)+(buf_impclrs[2]<<16)+(buf_impclrs[3]<<24);

	//printf("image header variables assigned\n");

			#if BDEBUG
				printf("ID: %c%c\n", b->id[0], b->id[1]);
				printf("File Size: %d\n", b->fileSize);
				printf("Reserve: %d\n", b->reserve);
				printf("Offset: %d\n", b->offset);
				printf("Header size: %d\n", b->hsize);
				printf("Image Width: %d\n", b->width);
				printf("Image Height: %d\n", b->height);
				printf("Planes: %d\n", b->planes);
				printf("Bits/pixel: %d\n", b->bpp);
				printf("Compression: %d\n", b->comp);
				printf("Data size: %d\n", b->dataSize);
				printf("Horizontal resolution: %d\n", b->hres);
				printf("Vertical resolution: %d\n", b->vres);
				printf("Colors: %d\n", b->colors);
				printf("Important colors: %d\n", b->impclrs);


			#endif





	/*for(i = 0; i < 0x400; i+=4){
		b.pallette[i/4]=(buf_pallette[i])+(buf_pallette[i+1]<<8)+(buf_pallette[i+2]<<16)+(buf_pallette[i+3]<<24);
	}

	#if 0
		for(i = 0; i< 0x100; ++i)
				printf("pallette at %x: %x\n", i,b.pallette[i]);
	#endif
*/

	///////////////////////////////////////
	//          READ IMAGE               //
	///////////////////////////////////////
	junkBytes = ((b->width*3)%4==0)? (0) : (4-((b->width)*3)%4);
#if BDEBUG
	printf("Junk bytes: %d\n", junkBytes);
#endif



	//the number of total bytes in buf_image is equal to the width in pixels times the
	//number of bytes per pixel plus the filler bytes at the end, quantity times the height in pixels of the image.
	bpr = b->width * b->bpp/8+junkBytes;
	bfimgsize = bpr * b->height;//bytes per row * number of rows

	fread(buf_image, size, bfimgsize, ifp);
	//printf("looping through image(read):\nbfimgsize=%d", bfimgsize);

	j = i = k = 0;
	//loop through buf_image
	while(k < bfimgsize){
		//if the current byte is not a junk byte
		if(k%bpr < bpr-junkBytes){
			//if the current byte begins a new row, reset the column counter
			if(k%bpr == 0)
				j=0;
			//if the current byte begins a new row and is not the first row,



			#if BDEBUG
				if(k%bpr == 0){
					printf("Press any key to continue...\n");
					getchar();
				}//if k is the start of a row
			#endif
			//for every byte that is not a junk pixel, seperate it by color.

				//blue comes first
				b->image[i][j][2] = buf_image[k];
				//green next
				b->image[i][j][1] = buf_image[k+1];
				//last is red
				b->image[i][j][0] = buf_image[k+2];

			#if 0
				printf("image at %d,%d:\n",i,j);
				printf("\t%x\n",b->image[i][j][0]);
				printf("\t%x\n",b->image[i][j][1]);
				printf("\t%x\n",b->image[i][j][2]);
			#endif

				//if it's not junk, increment to the next pixel
				k+=3;
				++j;

				//increment the row counter

		}//if k is not a junk byte

		//if it is junk, simply increment the counter
		else
			++k;
		if(k%bpr == 0 && k>0)
			++i;

	}//close loop through buf_image
	//printf("finished looping\n");
	}//close if
	else{
		printf("No such file exists\n");
	}

	fclose(ifp);
	//printf("leaving readBMP\n");

}//readBMP


void printBMP(Bitmap *b){

	//int i, j;

	//print header
	printf("ID: %c%c\n", b->id[0], b->id[1]);
	printf("File Size: %d\n", b->fileSize);
	printf("Reserve: %d\n", b->reserve);
	printf("Offset: %d\n", b->offset);
	printf("Header size: %d\n", b->hsize);
	printf("Image Width: %d\n", b->width);
	printf("Image Height: %d\n", b->height);
	printf("Planes: %d\n", b->planes);
	printf("Bits/pixel: %d\n", b->bpp);
	printf("Compression: %d\n", b->comp);
	printf("Data size: %d\n", b->dataSize);
	printf("Horizontal resolution: %d\n", b->hres);
	printf("Vertical resolution: %d\n", b->vres);
	printf("Colors: %d\n", b->colors);
	printf("Important colors: %d\n", b->impclrs);


	//print image
/*	printf("Image data: \n");

	for(i = 0; i < b->height; ++i){
		for(j = 0; j < b->width; ++j){
			printf("%d, %d: %x\n", i,j, (b->image[i][j][2])+(b->image[i][j][1]<<8) + (b->image[i][j][0]<<16));
		}//end for
	}//end for
*/
}

void writeBMP(Bitmap *b, char* output){
	FILE *ofp;

	unsigned char buf_id[2], buf_fileSize[4], buf_reserve[4], buf_offset[4], buf_hsize[4];
	unsigned char buf_width[4], buf_height[4], buf_planes[2], buf_bpp[2], buf_comp[4];
	unsigned char buf_dataSize[4], buf_hres[4], buf_vres[4], buf_colors[4], buf_impclrs[4];
	//unsigned char buf_pallette [0x100];
	unsigned char buf_image[WIDTH*HEIGHT*COLOR +JUNKBYTES];

	int i, j, k, junkBytes, bpr, bfimgsize;
	size_t size = 1;//sizeof(unsigned char);

	//write header
	buf_id[0] = b->id[0];
	buf_id[1] = b->id[1];

	buf_fileSize[0] = (b->fileSize&0x000000FF);
	buf_fileSize[1] = (b->fileSize&0x0000FF00)>>8;
	buf_fileSize[2] = (b->fileSize&0x00FF0000)>>16;
	buf_fileSize[3] = (b->fileSize&0xFF000000)>>24;

	buf_reserve[0] = (b->reserve&0x000000FF);
	buf_reserve[1] = (b->reserve&0x0000FF00)>>8;
	buf_reserve[2] = (b->reserve&0x00FF0000)>>16;
	buf_reserve[3] = (b->reserve&0xFF000000)>>24;

	buf_offset[0] = (b->offset&0x000000FF);
	buf_offset[1] = (b->offset&0x0000FF00)>>8;
	buf_offset[2] = (b->offset&0x00FF0000)>>16;
	buf_offset[3] = (b->offset&0xFF000000)>>24;

	buf_hsize[0] = (b->hsize&0x000000FF);
	buf_hsize[1] = (b->hsize&0x0000FF00)>>8;
	buf_hsize[2] = (b->hsize&0x00FF0000)>>16;
	buf_hsize[3] = (b->hsize&0xFF000000)>>24;


	buf_width[0] = (b->width&0x000000FF);
	buf_width[1] = (b->width&0x0000FF00)>>8;
	buf_width[2] = (b->width&0x00FF0000)>>16;
	buf_width[3] = (b->width&0xFF000000)>>24;

	buf_height[0] = (b->height&0x000000FF);
	buf_height[1] = (b->height&0x0000FF00)>>8;
	buf_height[2] = (b->height&0x00FF0000)>>16;
	buf_height[3] = (b->height&0xFF000000)>>24;

	buf_planes[0] = (b->planes&0x00FF);
	buf_planes[1] = (b->planes&0xFF00)<<8;

	buf_bpp[0] = (b->bpp&0x00FF);
	buf_bpp[1] = (b->bpp&0xFF00)<<8;

	buf_comp[0] = (b->comp&0x000000FF);
	buf_comp[1] = (b->comp&0x0000FF00)>>8;
	buf_comp[2] = (b->comp&0x00FF0000)>>16;
	buf_comp[3] = (b->comp&0xFF000000)>>24;


	buf_dataSize[0] = (b->dataSize&0x000000FF);
	buf_dataSize[1] = (b->dataSize&0x0000FF00)>>8;
	buf_dataSize[2] = (b->dataSize&0x00FF0000)>>16;
	buf_dataSize[3] = (b->dataSize&0xFF000000)>>24;


	buf_hres[0] = (b->hres&0x000000FF);
	buf_hres[1] = (b->hres&0x0000FF00)>>8;
	buf_hres[2] = (b->hres&0x00FF0000)>>16;
	buf_hres[3] = (b->hres&0xFF000000)>>24;

	buf_vres[0] = (b->vres&0x000000FF);
	buf_vres[1] = (b->vres&0x0000FF00)>>8;
	buf_vres[2] = (b->vres&0x00FF0000)>>16;
	buf_vres[3] = (b->vres&0xFF000000)>>24;

	buf_colors[0] = (b->colors&0x000000FF);
	buf_colors[1] = (b->colors&0x0000FF00)>>8;
	buf_colors[2] = (b->colors&0x00FF0000)>>16;
	buf_colors[3] = (b->colors&0xFF000000)>>24;


	buf_impclrs[0] = (b->impclrs&0x000000FF);
	buf_impclrs[1] = (b->impclrs&0x0000FF00)>>8;
	buf_impclrs[2] = (b->impclrs&0x00FF0000)>>16;
	buf_impclrs[3] = (b->impclrs&0xFF000000)>>24;

	ofp = fopen(output,"wb");

	fwrite(buf_id,size, 2, ofp);
	fwrite(buf_fileSize,size, 4, ofp);
	fwrite(buf_reserve,size, 4, ofp);
	fwrite(buf_offset,size, 4, ofp);
	fwrite(buf_hsize,size, 4, ofp);
	fwrite(buf_width,size, 4, ofp);
	fwrite(buf_height,size, 4, ofp);
	fwrite(buf_planes,size, 2, ofp);
	fwrite(buf_bpp,size, 2, ofp);
	fwrite(buf_comp,size, 4, ofp);
	fwrite(buf_dataSize,size, 4, ofp);
	fwrite(buf_hres,size, 4, ofp);
	fwrite(buf_vres,size, 4, ofp);
	fwrite(buf_colors,size, 4, ofp);
	fwrite(buf_impclrs,size, 4, ofp);
	//fwrite(buf_pallette,size, 0x100, ifp);
	//printf("header successfully written\n");

	//write image
	junkBytes = ((b->width*3)%4==0)? (0) : (4-((b->width)*3)%4);
	bpr = b->width * b->bpp/8+junkBytes;

	bfimgsize = bpr * b->height;//bytes per row * number of rows
	//printf("junkBytes = %d, bpr = %d, bfimgsize = %d\n", junkBytes, bpr, bfimgsize);
	//getchar();

	j = i = k = 0;
	//printf("about to loop through b->image\n");
	//loop through buf_image
	while(k < bfimgsize){
		//if the current byte is not a junk byte
		if(k%bpr < bpr-junkBytes){
			//if the current byte begins a new row, reset the column counter
			if(k%bpr == 0)
				j=0;
			//if the current byte begins a new row and is not the first row,
			//increment the row counter
			if(k%bpr == 0 && k > 0)
				++i;

			#if BDEBUG
				if(k%bpr == 0){
					printf("Press any key to continue...\n");
					getchar();
				}//if k is the start of a row
			#endif
			//for every byte that is not a junk pixel, seperate it by color.

				//blue comes first
				buf_image[k] = b->image[i][j][2];
				//green next
				buf_image[k+1] = b->image[i][j][1];
				//last is red
				buf_image[k+2] = b->image[i][j][0];

				//printf ("i,j,k=%d,%d,%d\n", i,j,k);
			#if BDEBUG
				printf("image at %d,%d:\n",i,j);
				printf("\t%x\n",b->image[i][j][0]);
				printf("\t%x\n",b->image[i][j][1]);
				printf("\t%x\n",b->image[i][j][2]);
			#endif

				//if it's not junk, increment to the next pixel
				k+=3;
				++j;

		}//if k is not a junk byte

		//if it is junk, simply increment the counter
		else
			++k;
	}//close loop through buf_image
//printf("finished looping, buffer size is %d\n", k);


fwrite(buf_image, size, bfimgsize, ofp);
fclose(ofp);
}//writeBMP

void cropBMP(Bitmap *b, unsigned short topCrop, unsigned short bottomCrop){
	int i, j;

	printf("topCrop = %d, bottomCrop = %d\n", topCrop,bottomCrop);

	if((topCrop < b->height) && (bottomCrop < b->height)){
		for(i=0; i<topCrop; ++i){
			for(j=0; j<b->width; ++j){
				b->image[b->height-1-i][j][0] = 0;
				b->image[b->height-1-i][j][1] = 0;
				b->image[b->height-1-i][j][2] = 0;
			}// for j
		}//for i
		for(i = 0; i<bottomCrop;++i){
			for(j = 0; j < b->width; ++j){
				b->image[i][j][0] = 0;
				b->image[i][j][1] = 0;
				b->image[i][j][2] = 0;
			}//for j
		}//for i
	}//if
	else{
		if(topCrop > b->height)
			printf("Cannot crop more rows from top than the image has");
		else
			printf("Cannot crop more rows from bottom than the image has");
	}//else
}//cropBMP
