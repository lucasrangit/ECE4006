#include<stdio.h>

#define BDEBUG 0
#define WIDTH 400
#define HEIGHT 300
#define COLOR 3
#define JUNKBYTES 0
#define TOPCROP 120
#define BOTTOMCROP 220
#define RED 0
#define GREEN 1
#define BLUE 2

typedef struct bitmap{
	char id[2];
	short planes, bpp;
	int fileSize, reserve, offset, hsize, width, height, comp, dataSize;
	int hres, vres, colors, impclrs;
	unsigned char image[HEIGHT][WIDTH][COLOR];
	//int pallette[0x100];

}Bitmap;


void readBMP(Bitmap*, char*);
void printBMP(Bitmap*);
void writeBMP(Bitmap*, char*);
//void cropBMP(Bitmap*, unsigned short, unsigned short);

unsigned char skin_tone_filter(unsigned char, unsigned char);
void maskFilter(unsigned char[HEIGHT][WIDTH], unsigned char[HEIGHT][WIDTH], unsigned char[HEIGHT][WIDTH]);
void differenceFilter(unsigned char[HEIGHT][WIDTH], unsigned char[HEIGHT][WIDTH], unsigned char[HEIGHT][WIDTH]);
void BMtoBinary(Bitmap*, unsigned char[HEIGHT][WIDTH]);
void BinarytoBM(unsigned char[HEIGHT][WIDTH], Bitmap*);
