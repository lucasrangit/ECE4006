/*
 * ECE 4006              Real-Time DSP with Dr. Barnwell              Fall 2005
 *
 * C implementation of a low-cost real-time* head tracking system for GTREP.
 *
 * Our implementation requires more smoothing of the camera movements, better
 * velocity thresholding in the x direction, and an abstracted main(). Besides
 * that it works reasonably well and quite fast (see printf of executable).
 * Also note that all generic bitmap functions have been moved to the
 * appropriate bitmap24.h header file.
 * --Lucasrangit 03:17, 27 Nov 2005 (EST)
 *
 * Section C Group 1
 *  WaiLing Chan
 *  Vincent Lacey
 *  Lucas Magasweran
 *  Jaimin Shah
 *  Justin Vogt
 *
 */

/* INCLUDES ******************************************************************/

#include<stdio.h>
#include<stdlib.h>                      //used for memory allocation operations
#include<math.h>                        //used for velocity and time
                                        // calculations
#include<string.h>                      //used for string sprintf and filenames
#include<time.h>                        //used for algorithm timing
#include"bitmap24.h"

/* GLOBAL CONSTANTS **********************************************************/

#define DEBUGF 1                        //general debug constant
#define TOPCROP 180                     //top of search region FROM BOTTOM OF IMAGE
#define BOTTOMCROP 80                   //bottom of search region FROM BOTTOM OF IMAGE
#define MASKHEIGHT 17                   //the height of the face mask used in findHead
#define MASKWIDTH 8                     //the width of "
#define HEADCORTHOLD 0.11                //correlation threshold for findHead
#define FPS 2                           //maximum frame rate
#define VTHOLDMIN 3.0                   //minimum velocity threshold
//#define VTHOLDMAX 70                   //maximum velocity threshold
#define COLOROUTPUT 1                   //set for color BMP as output
#define CENTTHOLD 80
#define XPTHOLD 70
#define YPTHOLD 15
#define MDIST 80

/* STRUCTURE DEFINITIONS *****************************************************/

typedef byte Binary[WIDTH];  //Binary is used to represent a Binary image;
                             //can be used in the following ways:
                             // Binary image[HEIGHT];
                             // Binary *image; image = malloc(sizeof(Binary[HEIGHT]));

typedef struct coord{  //coordinate data for bounding box and camera positioning
    int x,y;           //x = rows ; y = columns
}coord;

/* PROTOTYPES ****************************************************************/

void BMP2Binary(Bitmap24*, Binary*);    //converts a bitmap into a binary
void Binary2BMP(Binary*, Bitmap24*);    //converts a binary into a bitmap
void BW_BMP2Binary(Bitmap24*, Binary*); //takes a B&W bitmap and creates a binary mask
void printBinary(Binary*);              //displays a binary to the screen
void copyBinary(Binary*, Binary*);       //used to copy a binary image into new struct

byte skin_tone_filter(byte,byte);       //computes an r-g ratio and returns a boolean
void maskFilter(Binary*, Binary*, Binary*);        //creates a masked image
Binary* differenceFilter(Binary*, Binary*);  //finds the difference returns pointer to new Binary mask
                                                   // between two frames
int findHead(Binary*, coord*);          //find coordinates of the head
void drawBox(Bitmap24*, coord*);        //draw a bounding box around coordinates

/* FUNCTIONS *****************************************************************/

/* maskFilter
 * img = input binary image
 * mask = binary image mask
 * output = pointer to output image
 */
void maskFilter( Binary *img, Binary *mask, Binary *output ) {
    int i, j;
    for ( i = BOTTOMCROP ; i <= TOPCROP; ++i )     //taking forbidden region
        for ( j = 0 ; j < WIDTH ; ++j )            // into account,
            output[i][j] = img[i][j] & mask[i][j]; //mask off non-background pixels
}//maskFilter

/* differenceFilter
 * img1 = binary image at current time
 * img2 = binary image at previous time
 * output = pointer to output image
 */
Binary* differenceFilter( Binary *img1, Binary *img2 ) {
    int i, j;
	Binary *output = malloc(sizeof(Binary[HEIGHT]));
    for ( i = BOTTOMCROP ; i <= TOPCROP ; ++i )
        for ( j = 0 ; j < WIDTH ; ++j )
            output[i][j] = img1[i][j] - img2[i][j];
    return output;
}//differenceFilter

void copyBinary ( Binary *from, Binary *to ) {
    int i, j;
	for ( i = 0 ; i < HEIGHT ; ++i )
        for ( j = 0 ; j < WIDTH ; ++j )
            to[i][j] = from[i][j];
}

/* BMP2Binary converts a RGB bitmap to a B&W binary mask based on skin tone threshold
 * b = pointer to bitmap
 * skinmask = pointer to output skinmask (binary image)
 */
void BMP2Binary(Bitmap24 *b, Binary *skinmask) {
    int i, j;

    #if BDEBUG
        printf("Converting bitmap to binary...\n");
    #endif

    //write black pixels of bottom fobidden region
    for ( i = 0 ; i < BOTTOMCROP; ++i)
        for( j = 0; j < WIDTH; ++j)
            skinmask[i][j] = 0;

    //write black pixels of top forbidden region
    for ( i = HEIGHT-1 ; i > TOPCROP ; --i )
        for( j = 0 ; j < WIDTH ; ++j )
            skinmask[i][j] = 0;

    //perform skin_tone_filter'ing on the rest
    for ( i = BOTTOMCROP ; i <= TOPCROP ; ++i)
        for( j = 0 ; j < WIDTH ; ++j )
            skinmask[i][j] = skin_tone_filter( b->image[i][j][RED],
                                               b->image[i][j][GREEN] );

    #if BDEBUG
        printf("Finished conversion.");
    #endif
}//BMP2Binary

/* BW_BMP2Binary
 * b = pointer to a black and white bitmap
 * mask = pointer to output binary mask
 */
void BW_BMP2Binary(Bitmap24 *b, Binary *mask){
    int i, j;

    #if BDEBUG
        printf("Converting B&W bitmap to binary...\n");
    #endif

    for ( i = BOTTOMCROP ; i < TOPCROP ; ++i )
        for ( j = 0 ; j < WIDTH ; ++j ) {
            if ( b->image[i][j][RED] ) //all channels are equal for BW BMPs
                mask[i][j] = 1;        //anything other than "black" = 1
            else {
                mask[i][j] = 0;
                #if BDEBUG
                    printf("%d,%d\n", i, j);
                #endif
            }
    }

    #if BDEBUG
        printf("Finished conversion.");
    #endif
}//BW_BMP2Binary

/* skin_tone_filter performs the division of red to green.
 * for values between a region, return 1 (for skin), 0 for (non-skin)
 * r = red pixel value
 * g = green pixel value
 */
byte skin_tone_filter(byte r, byte g) {
    const double skin_min = 1.24;    //min value for skin ratio
    const double skin_max = 1.42;    //max value for skin ratio
    double RG_ratio;

    if ( r == 0 || g == 0 )  //do not divide if one of the colors are 0
        return 0;
    else {
        RG_ratio = (double) r/g; //computes the ratio and sets it as a double
        if ( (skin_min <= RG_ratio) && (RG_ratio <= skin_max) )
            return 1;
        else
            return 0;
    }
}//skin_tone_filter

/* Binary2BMP
 * bn = input binary image
 * b =  output bitmap image
 * ONLY USE AN ALREADY-OPENED BITMAP AS INPUT
 */
void Binary2BMP(Binary *bn, Bitmap24 *b){
    int i, j;

    #if BDEBUG
        printf("Converting binary to bitmap...\n");
    #endif
/*
    //create header file, not used because bitmap has already been created
    b->id[0] = 'B';
    b->id[1] = 'M';

    b->planes = 1;
    b->bpp = 24;

    b->fileSize = sizeof(bn)+40;
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
    b->impclrs = 0;
*/

    //overwrite bitmap's image data with that of binary's
    for ( i = 0 ; i < HEIGHT ; ++i ) {
        for ( j = 0 ; j < WIDTH ; ++j ) {
            if ( bn[i][j] ) {
                b->image[i][j][RED] = 255;
                b->image[i][j][GREEN] = 255;
                b->image[i][j][BLUE] = 255;
            }//if
            else {
                b->image[i][j][RED] = 0;
                b->image[i][j][GREEN] = 0;
                b->image[i][j][BLUE] = 0;
            }//else
        }//for j
    }//for i

}//Binary2BMP


void printBinary(Binary *bin){
        int i, j=0;

        if(j%WIDTH == 0)
                getchar();
        for(i = 0; i < HEIGHT; ++i)
                for( j = 0; j < WIDTH; ++j){
                        printf("image at %d,%d: %d\n", i,j,bin[i][j]);

                }
}//printBinary

//returns the coordinate of the center of the rectangle with the largest white pixel percentage
int findHead(Binary *frame, coord *centroid){
    byte subFrame[MASKHEIGHT][MASKWIDTH];
    byte facemask[MASKHEIGHT][MASKWIDTH];
    double maskArea = MASKHEIGHT*MASKWIDTH;  //define maskArea globaly
    double sum = 0;
    double cor = 0;
    int i, j, k, l;

    for ( i = TOPCROP ; i >= BOTTOMCROP + MASKHEIGHT-1 ; --i ) {
        for ( j = 0 ; j <= WIDTH - MASKWIDTH+1 ; ++j ) {
            sum = 0;
            for ( k = MASKHEIGHT-1 ; k >= 0 ; --k ){
                for(l = 0; l <= MASKWIDTH-1; ++l){
                    subFrame[k][l] = frame[i-k][j+l];
                    if (k > 2 && k < 14)
                        facemask[k][l] = 1;
                    else if((k> 0 && k < 16) && (l > 0 && l < 7))
                        facemask[k][l] = 1;
                    else if(k == 0 && l >1 && l < 6)
                        facemask[k][l] = 1;
                    else if(k == 16 && (l == 3 || l == 4))
                        facemask[k][l] = 1;
                    else
                        facemask[k][l] = 0;

                    sum += (subFrame[k][l] & facemask[k][l]);
                }//for l
            }//for k

            cor = sum / maskArea;

            if (cor >= HEADCORTHOLD) {
                printf("correlation \t\t\t%f\n", cor);
                centroid->x = i - (int)(MASKHEIGHT/2);
                centroid->y = j + (int)(MASKWIDTH/2);
                return 1;
            }//if
        }//for j
    }//for i

    centroid->x = -1;
    centroid->y = -1;
    return 0;
}//findHead

void drawBox(Bitmap24 *img, coord *centroid){
    byte r = 255;
    byte g = 255;
    byte b = 0;
    byte thickness = 2;
    byte dim_x = 30;
    byte dim_y = 40;
    byte half_x = (byte)(int)(dim_x/2); //casting as an int performs rounding w/o math.h
    byte half_y = (byte)(int)(dim_y/2);
    int i,t;

    //draw vertical sides
    for (i=(centroid->x - half_x - thickness+2); i <=(centroid->x + half_x + thickness-2); ++i){
        if ((i>0)&&(i< HEIGHT)){
            for (t=0; t <(thickness-1); ++t){
                if (((centroid->y - half_y - t) > 0) && ((centroid->y - half_y - t) < WIDTH)){
                    img->image[i][centroid->y - half_y -t][RED] = r;
                    img->image[i][centroid->y - half_y -t][GREEN] = g;
                    img->image[i][centroid->y - half_y -t][BLUE] = b;
                }//if

                if (((centroid->y + half_y + t) > 0) && ((centroid->y + half_y + t) < WIDTH)){
                    img->image[i][centroid->y + half_y +t][RED] = r;
                    img->image[i][centroid->y + half_y +t][GREEN] = g;
                    img->image[i][centroid->y + half_y +t][BLUE] = b;
                }//if
            }//for t
        }//if
    }//for i


    //draw horizontal sides
    for (i=(centroid->y - half_y - thickness+2); i <(centroid->y + half_y + thickness-2); ++i){

        if( i >0 && i< WIDTH){

            for (t=0; t <(thickness - 1); ++t){

                if(((centroid->x - half_x - t) > 0) && ((centroid->x - half_x - t) < HEIGHT)){
                    img->image[centroid->x-half_x-t][i][RED] = r;
                    img->image[centroid->x-half_x-t][i][GREEN] = g;
                    img->image[centroid->x-half_x-t][i][BLUE] = b;
                }//if

                if(((centroid->x + half_x + t) > 0) && ((centroid->x + half_x + t) < HEIGHT)){
                    img->image[centroid->x+half_x+t][i][RED] = r;
                    img->image[centroid->x+half_x+t][i][GREEN] = g;
                    img->image[centroid->x+half_x+t][i][BLUE] = b;

                }//if
            }//for t
        }//if
    }//for i
}//drawBox

int main(){
    Bitmap24 *frame;            //pointer to input frame (RGB)
	Binary *binFrame;           //binary version of frame
	Binary *old_binFrame;       //pointer to previous input binary frame
	Binary *diff_binFrame;      //temporary binary frame
    Bitmap24 *BGmask;           //background mask (read in as BMP)
    Binary *BGbinMask;          //binary version of mask
    coord *centroid;
    coord *cent_old;
    int foundHead = 0;
    int i;
    //int numPics = 116;
    int numPics = 43;
    double v, dx, dy, time_to_compute;
	int xp, yp, xp_old, yp_old;
    char input[20], output[20];
    time_t t1, t2;                     //time for determining algorithm time

    centroid = malloc(sizeof(coord));
    cent_old = malloc(sizeof(coord));

    centroid->x = BOTTOMCROP;          //starting position of search camera and tracking camera
    centroid->y = WIDTH/2;
	cent_old->x = centroid->x;
	cent_old->y = centroid->y;
    xp = centroid->x;
    yp = centroid->y;

    //create the backgroung mask
    BGmask = malloc(sizeof(Bitmap24));
    readBMP(BGmask, "../sample_data/teacher_walks_off-01-bg.bmp");
    BGbinMask = malloc(sizeof(Binary[HEIGHT]));
    BW_BMP2Binary(BGmask, BGbinMask);
    free(BGmask);  //free the BG mask (bitmap)

    //allocate BMP frame of the scene
    frame = malloc(sizeof(Bitmap24));
    //allocate binary image based on skin tone threshold
    binFrame = malloc(sizeof(Binary[HEIGHT]));
	//allocate old binary frame
    old_binFrame = malloc(sizeof(Binary[HEIGHT]));

    t1 = time(&t1);
    for ( i = 1 ; i <= numPics ; ++i ) {
        //read image
        //printf("../sample_data/teacher_with_student/teacher_with_student%03d.bmp\n",i);
        //sprintf(input,"../sample_data/teacher_with_student/teacher_with_student%03d.bmp",i);
        printf("../sample_data/teacher_desk_walks_off/teacher_desk_walks_off-04%02d.bmp\n",i);
        sprintf(input,"../sample_data/teacher_desk_walks_off/teacher_desk_walks_off-04%02d.bmp",i);
        readBMP(frame, input);

        BMP2Binary(frame, binFrame);

		//perform differencing to detect motion
        diff_binFrame = differenceFilter(binFrame, old_binFrame);

        //update old binary frame
		copyBinary(binFrame, old_binFrame);

		//mask off the background regions and update binary frame
        maskFilter(binFrame, BGbinMask, binFrame);

        //find head and compute centroid
        foundHead = findHead(binFrame, centroid);
        if ( !foundHead ) {
			printf("No head found.\n");
            centroid->x = xp; //cent_old->x;
            centroid->y = yp; //cent_old->y;
        } else {
            //Compute velocity
            dx = (centroid->x - cent_old->x)/FPS;
            dy = (centroid->y - cent_old->y)/FPS;
			//dx_avg = (dx + dx_1 + dx_2)*1/n;
            //v = sqrt(pow(dx,2) + pow(dy,2));
            v = (double)abs(dx) + (double)abs(dy);   //approximate manhattan distance (velocity)
            printf("velocity = %f\n",v);

			// Intelligence
            if ( (v > VTHOLDMIN) ) {           //minimum velocity threshold, don't move bb
              //&& (v < VTHOLDMAX) ) {        //threshold above maximum, don't follow

				// Predictive Threshold
					if ( (abs(centroid->x - xp) <= XPTHOLD)
					  || (abs(centroid->y - yp) <= YPTHOLD) ) {
                        printf("Predicted (yp,xp) = \t\t%d,%d\n",yp,xp);
						printf("Difference (y,x)\t\t%d,%d\n",abs(centroid->y - yp),abs(centroid->x - xp));
						printf("Manhattan Difference: \t\t%d\n",abs(abs(centroid->y - yp)-abs(centroid->x - xp)));
					} else {
						centroid->x = xp;
                        centroid->y = yp;
					}
                //centroid->x = (int)( (centroid->x + cent_old->x)/2 );
                //centroid->y = (int)( (centroid->y + cent_old->y)/2 );
            } else {                        //normal movement, track
				centroid->x = cent_old->x;  //restore previous values
                centroid->y = cent_old->y;
            }

			//Predicted Coordinates
			xp = centroid->x + (int)dx*FPS;
			yp = centroid->y + (int)dy*FPS;

			// Update history
			xp_old = xp;
			yp_old = yp;
			cent_old->x = centroid->x;
			cent_old->y = centroid->y;

        }//if (!foundHead)

        #if DEBUGF
            printf("centroid for frame %d (c,r):\t%d,%d\n", i, centroid->y, centroid->x);
            printf("---------------------------------------\n");
        #endif

        //draw bounding box to illustrate the second cameras view
        #if COLOROUTPUT
            drawBox(frame, centroid); //to print color image
        #else
            //write out the frame for testing purposes
            Binary2BMP(binFrame,frame); //comment out this line for color image
            drawBox(frame, centroid); //to print bounding box on RGB image image
        #endif

        free(diff_binFrame);
        //output stage
        sprintf(output,"output/%03d.bmp",i);
        writeBMP(frame, output);
    }//for i

    t2 = time(&t2);
    time_to_compute = abs(difftime(t2, t1));
    printf("\nseconds/frame = %E\n",time_to_compute/numPics);
    printf("approx. (for P4 2.5GHz) number of CPU cycles/frame = %e\n", (time_to_compute/numPics)*(2.4*pow(10, 9)));

    free(centroid);
    free(cent_old);
    free(BGbinMask);
    free(frame);
    free(binFrame);
	free(old_binFrame);
    return 1;
}//main
