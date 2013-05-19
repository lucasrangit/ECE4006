/*
 * Creates usable variables for everything contained in a bitmap file
 * (haven't gotten anything past the header to work yet).
 */

#include<stdio.h>

#define IMG_SIZE 361000
#define BDEBUG 1;
int main()
{

	unsigned char output[] = "img2.bmp";
	unsigned char buffer[IMG_SIZE];
	short id, planes, bpp;
	int fileSize, reserve, offset, hsize, width, height, comp, dataSize;
	int hres, vres, colors, impclrs, pallette[0x100], image[500];
	int i,j;
	size_t size = sizeof(unsigned char);

	size_t size_file;

	FILE *ifp, *ofp;

	//open files
	ifp = fopen("img.bmp","rb");
	ofp = fopen(output, "wb");

	//read image into buffer
	fread(buffer, size,IMG_SIZE,ifp);


	//create image structure
	for(i = 0; i < IMG_SIZE; ++i){
		switch (i) {
		case 0x0000:
			id = buffer[i]+(buffer[i+1]<<4);

			#if BDEBUG
				printf("ID: %d\n", id);
			#endif
			break;
		case 0x0002:
			fileSize = (buffer[i])+(buffer[i+1]<<8)+(buffer[i+2]<<16)+(buffer[i+3]<<24);
			#if BDEBUG
				printf("File Size: %d\n", fileSize);
			#endif
			break;
		case 0x0006:
			reserve = (buffer[i])+(buffer[i+1]<<8)+(buffer[i+2]<<16)+(buffer[i+3]<<24);
			#if BDEBUG
				printf("Reserve: %d\n", reserve);
			#endif
			break;
		case 0x000a:
			offset = (buffer[i])+(buffer[i+1]<<8)+(buffer[i+2]<<16)+(buffer[i+3]<<24);
			#if BDEBUG
				printf("Offset: %d\n", offset);
			#endif
			break;
		case 0x000e:
			hsize = (buffer[i])+(buffer[i+1]<<8)+(buffer[i+2]<<16)+(buffer[i+3]<<24);
			#if BDEBUG
				printf("Header size: %d\n", hsize);
			#endif
			break;
		case 0x0012:
			width = (buffer[i])+(buffer[i+1]<<8)+(buffer[i+2]<<16)+(buffer[i+3]<<24);
			#if BDEBUG
				printf("Image Width: %d\n", width);
			#endif
			break;
		case 0x0016:
			height = (buffer[i])+(buffer[i+1]<<8)+(buffer[i+2]<<16)+(buffer[i+3]<<24);
			#if BDEBUG
				printf("Image Height: %d\n", height);
			#endif
			break;
		case 0x001a:
			planes = buffer[i]+(buffer[i+1]<<4);

			#if BDEBUG
				printf("Planes: %d\n", planes);
			#endif
			break;
		case 0x001c:
			bpp = buffer[i]+(buffer[i+1]<<4);

			#if BDEBUG
				printf("Bits/pixel: %d\n", bpp);
			#endif
			break;
		case 0x001e:
			comp = (buffer[i])+(buffer[i+1]<<8)+(buffer[i+2]<<16)+(buffer[i+3]<<24);
			#if BDEBUG
				printf("Compression: %d\n", comp);
			#endif
			break;
		case 0x0022:
			dataSize = (buffer[i])+(buffer[i+1]<<8)+(buffer[i+2]<<16)+(buffer[i+3]<<24);
			#if BDEBUG
				printf("Data size: %d\n", dataSize);
			#endif
			break;
		case 0x0026:
			hres = (buffer[i])+(buffer[i+1]<<8)+(buffer[i+2]<<16)+(buffer[i+3]<<24);
			#if BDEBUG
				printf("Horizontal resolution: %d\n", hres);
			#endif
			break;
		case 0x002a:
			vres = (buffer[i])+(buffer[i+1]<<8)+(buffer[i+2]<<16)+(buffer[i+3]<<24);
			#if BDEBUG
				printf("Vertical resolution: %d\n", vres);
			#endif
			break;
		case 0x002e:
			colors = (buffer[i])+(buffer[i+1]<<8)+(buffer[i+2]<<16)+(buffer[i+3]<<24);
			#if BDEBUG
				printf("Colors: %d\n", colors);
			#endif
			break;
		case 0x0032:
			impclrs = (buffer[i])+(buffer[i+1]<<8)+(buffer[i+2]<<16)+(buffer[i+3]<<24);
			#if BDEBUG
				printf("Important colors: %d\n", impclrs);
			#endif
			break;
		case 0x0036:

			break;
		default:
			;
		}
	}

	for(i = 0x0036; i < 0x436; i+=4){
		pallette[i/4-0x0036]=(buffer[i])+(buffer[i+1]<<8)+(buffer[i+2]<<16)+(buffer[i+3]<<24);
	}

	#if BDEBUG
		for(i = 0; i< 0x100; ++i)
				printf("pallette: %x\n", pallette[i]);
	#endif

		for(i = 0x0436; i <IMG_SIZE; ++i){
		image[i/4-0x0436]=(buffer[i])+(buffer[i+1]<<8)+(buffer[i+2]<<16)+(buffer[i+3]<<24);
		printf("%d\n", i);
		}

#if BDEBUG
		for(i = 0; i< 0x100; ++i)
				printf("Image: %x\n", image[i]);
	#endif
}
