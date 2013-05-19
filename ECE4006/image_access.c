#include<stdio.h>

int main()
{
	unsigned char Header[0x435], image[400][300][3];
	unsigned char output[] = "img2.bmp";
	int i, j;
	FILE *ifp, *ofp;
	ifp = fopen("img.bmp","r");
	fread(Header,1,0x435,ifp);
	fseek(ifp,0x436,SEEK_SET);
	fread(image,1,400*300*3,ifp);
	ofp = fopen(output, "w");
	fwrite(Header,1,0x435,ofp);
	for(i = 0; i < 25; ++i)
		for(j = 0; j < 25; ++j)
			image[i][j][1] = 255;

	fwrite(image,1,400*300*3,ofp);
	fclose(ofp);
	fclose(ifp);

	return 0;
}
