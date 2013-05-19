/*
 * bitmap24.h               24-bit Bitmap library                 2005-11-22
 *                        400 x 300 size images only
 * Primary Developer:
 *  Justin Vogt
 *
 * Contributors:
 *  Vincent Lacey
 *  Lucas Magasweran
 *  Jaimin Shah
 *
 */

/* INCLUDES ******************************************************************/

#include<stdio.h>

/* GLOBAL CONSTANTS **********************************************************/

#define BDEBUG 0                        //bitmap debug constant
#define WIDTH 400                       //width from left to right
#define HEIGHT 300                      //heigh from top to bottom
#define COLOR 3                         //number of RGB colors?
#define JUNKBYTES 0                     //required BMP junkbytes for specified size
#define RED 0                           //used when accessing a color from an image array
#define GREEN 1                         //"
#define BLUE 2                          //"

/* STRUCTURE DEFINITIONS *****************************************************/

typedef unsigned char BYTE;  //8-bit data type

/* Bitmap used to read and convert image files to Binary */
typedef struct BITMAP24{
    char  id[2];           //holds 'BM'
    short planes,          //number of planes
          bpp;             //bits per pixel
    int   fileSize,        //holds the file size in bytes
          reserve,         //not used
          offset,          //bytes from start of file until the image data
          hsize,           //size of the header, in bytes
          width,           //width in pixels
          height,          //height in pixels
          comp,            //compression; if no compression, comp=0
          dataSize,        //size of just the image data, in bytes
          hres,            //horizontal resolution in pixels per meter
          vres,            //vertical resolution in pixels per meter
          colors,          //the number of colors in the image
          impclrs;         //if system has limited display options,
                           // tells which color is most important
    BYTE  image[HEIGHT][WIDTH][COLOR];//where image data is stored
    //int   pallette[0x100];  //not used in 24-bit images
}BITMAP24;

/* PROTOTYPES ****************************************************************/

int BytesToInt(BYTE*, short);            //converts an array of bytes to an integer
void IntToBytes(int, BYTE*, short);      //converts an integer into an array of bytes

void BITMAP24_Read(BITMAP24*, char*);         //reads a bitmap file into memory
void BITMAP24_Print(BITMAP24*);               //prints a bitmap file to the screen
void BITMAP24_Write(BITMAP24*, char*);        //writes a bitmap file to disk

/* FUNCTIONS *****************************************************************/

/* BITMAP24_Read
 * b = file pointer to a bitmap file
 * input = location of file in ascii
 */
void BITMAP24_Read(BITMAP24 *b, char *input) {
    unsigned char buf_id[2], buf_fileSize[4], buf_reserve[4], buf_offset[4], buf_hsize[4];
    unsigned char buf_width[4], buf_height[4], buf_planes[2], buf_bpp[2], buf_comp[4];
    unsigned char buf_dataSize[4], buf_hres[4], buf_vres[4], buf_colors[4], buf_impclrs[4];
    //unsigned char buf_pallette [0x100];
    unsigned char buf_image[WIDTH*HEIGHT*COLOR+JUNKBYTES];
    unsigned int i, j, k, junkBytes, bfimgsize, bpr;

    size_t size = 1;  //sizeof(unsigned char);

    FILE *ifp;
    #if BDEBUG
        printf("read variables assigned successfully\n");
    #endif
    ifp = fopen(input, "rb");

    ///////////////////////////////////////
    //          CREATE HEADER            //
    ///////////////////////////////////////

    //fread(Header,1,0x436,ifp);
    if ( ifp != NULL ) {
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

        #if BDEBUG
            printf("read buffers passed\n");
        #endif

        //create header of bitmap
        b->id[0] = buf_id[0];
        b->id[1] = buf_id[1];
        b->fileSize = BytesToInt(buf_fileSize,4);
        b->reserve = BytesToInt(buf_reserve,4);
        b->offset = BytesToInt(buf_offset,4);
        b->hsize = BytesToInt(buf_hsize,4);
        b->width = BytesToInt(buf_width,4);
        b->height = BytesToInt(buf_height,4);
        b->planes = BytesToInt(buf_planes,2);
        b->bpp = BytesToInt(buf_bpp,2);
        b->comp = BytesToInt(buf_comp,4);
        b->dataSize = BytesToInt(buf_dataSize,4);
        b->hres = BytesToInt(buf_hres,4);
        b->vres = BytesToInt(buf_vres,4);
        b->colors = BytesToInt(buf_colors,4);
        b->impclrs = BytesToInt(buf_impclrs,4);

        #if BDEBUG
            printf("Image header variables assigned:\n");
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

        /*
        //used non 24-bit color images
        for(i = 0; i < 0x400; i+=4){
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

        //compute junkBytes for stability
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
        while ( k < bfimgsize ) {
                //if the current byte is not a junk byte
                if ( k%bpr < bpr - junkBytes ) {
                        //if the current byte begins a new row, reset the column counter
                        if ( k%bpr == 0 )
                                j=0;    //if the current byte begins a new row and is not the first row

                        #if BDEBUG
                                if ( k%bpr == 0 ) {
                                        printf("Press any key to continue...\n");
                                        getchar();
                                }//if k is the start of a row
                        #endif

                        //for every byte that is not a junk pixel, seperate it by color.
                        //blue comes first
                        b->image[i][j][BLUE] = buf_image[k];
                        //green next
                        b->image[i][j][GREEN] = buf_image[k+1];
                        //last is red
                        b->image[i][j][RED] = buf_image[k+2];

                        #if 0
                                printf("image at %d,%d:\n",i,j);
                                printf("\t%x\n",b->image[i][j][RED]);
                                printf("\t%x\n",b->image[i][j][GREEN]);
                                printf("\t%x\n",b->image[i][j][BLUE]);
                        #endif

                        //if it's not junk, increment to the next pixel
                        k += 3;
                        ++j;    //increment the row counter

                }//if k is not a junk byte
                else //if it is junk, simply increment the counter
                        ++k;
                if ( k%bpr == 0 && k>0 )
                        ++i;
        }//close loop through buf_image
        #if BDEBUG
            printf("finished looping\n");
        #endif
    } else
            printf("No such file exists!\n");

    fclose(ifp);
    #if BDEBUG
        printf("leaving BITMAP24_Read.\n");
    #endif

}//BITMAP24_Read

/* BITMAP24_Print output BMP header info, and data if BDEBUG is set
 * b = input bitmap
 */
void BITMAP24_Print( BITMAP24 *b ) {
    #if BDEBUG
        int i, j;
    #endif

    //print header
    printf("Image data: \n");
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

    #if BDEBUG
        //print image
        printf("Image data:\n");
        for ( i = 0 ; i < b->height ; ++i ) {
            for ( j = 0 ; j < b->width ; ++j ) {
                printf("%d, %d: %x\n", i,j, ( b->image[i][j][2] )
                                          + ( b->image[i][j][1]<<8 )
                                          + ( b->image[i][j][0]<<16 ) );
            }//end for
        }//end for
    #endif
}

/* BITMAP24_Write
 * b = pointer to bitmap
 * output = string filename
 */
void BITMAP24_Write( BITMAP24 *b, char* output ) {
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

        IntToBytes(b->fileSize,buf_fileSize, 4);
        IntToBytes(b->reserve,buf_reserve, 4);
        IntToBytes(b->offset,buf_offset, 4);
        IntToBytes(b->hsize, buf_hsize,4);
        IntToBytes(b->width,buf_width, 4);
        IntToBytes(b->height,buf_height, 4);
        IntToBytes(b->planes,buf_planes, 2);
        IntToBytes(b->bpp,buf_bpp, 2);
        IntToBytes(b->comp,buf_comp, 4);
        IntToBytes(b->dataSize,buf_dataSize, 4);
        IntToBytes(b->hres,buf_hres, 4);
        IntToBytes(b->vres,buf_vres, 4);
        IntToBytes(b->colors,buf_colors, 4);
        IntToBytes(b->impclrs,buf_impclrs, 4);

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
                if(k%bpr < bpr-junkBytes) {
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
                        buf_image[k] = b->image[i][j][BLUE];
                        //green next
                        buf_image[k+1] = b->image[i][j][GREEN];
                        //last is red
                        buf_image[k+2] = b->image[i][j][RED];

                        //printf ("i,j,k=%d,%d,%d\n", i,j,k);
                        #if 0
                                printf("image at %d,%d:\n",i,j);
                                printf("\t%x\n",b->image[i][j][RED]);
                                printf("\t%x\n",b->image[i][j][GREEN]);
                                printf("\t%x\n",b->image[i][j][BLUE]);
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
}//BITMAP24_Write

int BytesToInt(BYTE *bytes, short numBytes){

        int converted=0, i;

        if(numBytes>0){
                for(i = 0; i < numBytes; ++i){
                        converted += *(bytes+i)<<(i*8);
                }//for
        }//if
        else
                printf("\tBytesToInt: cannot input zero bytes\n");
        return converted;
}//BytesToInt

void IntToBytes(int i, BYTE *buffer,short bytes){
    switch(bytes){
    case 2:
            buffer[0] = i&0x000000FF;
            buffer[1] = (i&0x0000FF00)>>8;
            break;
    case 3:
            buffer[0] = i&0x000000FF;
            buffer[1] = (i&0x0000FF00)>>8;
            buffer[2] = (i&0x00FF0000)>>16;
            break;
    case 4:
            buffer[0] = i&0x000000FF;
            buffer[1] = (i&0x0000FF00)>>8;
            buffer[2] = (i&0x00FF0000)>>16;
            buffer[3] = (i&0xFF000000)>>24;
            break;
    }//switch
}//IntToBytes
