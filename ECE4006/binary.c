#include "ece4006header.h"

void maskFilter(unsigned char img[HEIGHT][WIDTH], unsigned char mask[HEIGHT][WIDTH], unsigned char output[HEIGHT][WIDTH]){
	int i, j;
	unsigned char temp;

	for(i = 0; i < HEIGHT; ++i)
		for(j = 0; j < WIDTH; ++j){
			temp = img[i][j] & mask[i][j];
			output[i][j] = temp;
		}

}//maskFilter

void differenceFilter(unsigned char img1[HEIGHT][WIDTH], unsigned char img2[HEIGHT][WIDTH], unsigned char output[HEIGHT][WIDTH]){
	int i,j;
	for(i = 0; i < HEIGHT; ++i)
		for(j = 0; j < WIDTH; ++j)
			output[i][j] = img1[i][j] - img2[i][j];
}//differenceFilter

void BMtoBinary(Bitmap *b, unsigned char skinmask[HEIGHT][WIDTH]){
	int i, j;
	for(i = 0; i < HEIGHT; ++i)
		for( j = 0; j < WIDTH; ++j)
			skinmask[i][j] = skin_tone_filter(b->image[i][j][RED], b->image[i][j][GREEN]);
}//BMtoBinary


//ONLY USE AN ALREADY-OPENED BITMAP AS INPUT
void BinarytoBM(unsigned char bn[HEIGHT][WIDTH], Bitmap *b){

	int i, j;

	/*b->id[0] = 'B';
	b->id[1] = 'M';

	b->planes = 1;
	b->bpp = 24;

	b->fileSize = 360054;
	b->reserve = 0;
	b->offset = 0;
	b->hsize = 40;
	b->width = WIDTH;
	b->height = HEIGHT;
	b->comp = 0;
	b->dataSize = 0;
	b->hres = 2835;
	b->vres = 2835;
	b->colors = 0;
	b->impclrs = 0;*/

	for(i = 0; i < HEIGHT; ++i){
		for(j = 0; j < WIDTH; ++j){
			if(bn[i][j]){
				b->image[i][j][RED] = 255;
				b->image[i][j][GREEN] = 255;
				b->image[i][j][BLUE] = 255;
			}//if
			else{
				b->image[i][j][RED] = 0;
				b->image[i][j][GREEN] = 0;
				b->image[i][j][BLUE] = 0;
			}//else
		}//for j
	}//for i

}//BinarytoBM

unsigned char skin_tone_filter(unsigned char r, unsigned char g){
	const double skin_min = 1.24;  // sets min value for ratio
	const double skin_max = 1.42; // sets max value for ratio

	double RG_ratio;

	if(r==0 && g == 0)
		return 0;
	else{
		RG_ratio = ((double)r)/((double)g); //computes the ratio and sets it as a double

		if((skin_min <= RG_ratio) && (RG_ratio <= skin_max))
			return 1;
		else
			return 0;
	}//if r==g==0

}//skin_tone_filter
