#include "ece4006header.h"

void main(){


	Bitmap bm;
	Bitmap *b = &bm;
	unsigned char frame[HEIGHT][WIDTH];

//	int i,j;
//	unsigned char skinmask[HEIGHT][WIDTH];


	//printf("bitmap has been successfully created\n");

	readBMP(b, "img.bmp");
	//printBMP(b);

	BMtoBinary(b,frame);
	BinarytoBM(frame,b);

	//cropBMP(b,120,80);
	//printf("bitmap has been successfully read\n");

	//printBMP(b);

	//printBMP(b);

/*	for(i = 0; i < HEIGHT; ++i){
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

*/	//printf("image has been masked\n");
	writeBMP(b, "img_out.bmp");



}//main
