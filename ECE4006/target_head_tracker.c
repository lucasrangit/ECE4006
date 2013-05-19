/*
********************************************************************************
* ECE4006 - Real-Time DSP with Dr. Barnwell - Fall 2005
* Georgia Institute of Technology
*
* (c) Attribution-NonCommercial-ShareAlike 2.5
*     You are free: to copy, distribute, display, and perform the work
*                   to make derivative works
*
* The only difference here from the working C code is that RTDX is set up to
* listen for the red and green channels of each image.
* Note that, on the EVM this code currently does not work, as it utilizes
* floating-point arithmetic, and the EVM does not seem to support it, as
* evidenced by:
*
* 	printf("%f\n", 0.5);
*
* 	returning a value 0.0000
*
* Filename: target_head_tracker.c
* Programmer(s): Section C Group 1
*                WaiLing Chan
*                Vincent Lacey
*                Lucas Magasweran
*                Jaimin Shah
*                Justin Vogt
*
* Lucas Magasweran (LM)
* Created: 2005/12/08
* Description: C implementation of a low-cost real-time* head tracking system.
********************************************************************************
*/

/*
********************************************************************************
*                               INCLUDES
********************************************************************************
*/
#include <stdio.h>      /* C_I/O                                    */
#include <stdlib.h>             //used for memory allocation operations
//#include <math.h>               //used for velocity and time
//#include <string.h>             //used for string sprintf and filenames
//#include <time.h>               //used for algorithm timing
#include "../../bitmap24.h"
/* RTDX */
#include <rtdx.h>       /* defines RTDX target API calls            */
#include "target.h"     /* defines TARGET_INITIALIZE()              */


/*
********************************************************************************
*                               GLOBAL CONSTANTS
********************************************************************************
*/
/* DEBUG FLAGS */
#define DEBUGF 1                //general debug constant
#define PDEBUG 1                //timer debug flag

/* IMAGE PARAMETERS */
#define WIDTH 400                       //width from left to right
#define HEIGHT 300                      //heigh from top to bottom

/* VISION PARAMETERS */
#define NUMPICS       8        //number of pictures in input directory images/
#define FPSPOW2       1         //closest power of two to FPS = 2^FPSPOW2)
#define TOPCROP       180       //top of search region FROM BOTTOM OF IMAGE
#define BOTTOMCROP    80        //bottom of search region FROM BOTTOM OF IMAGE
#define MASKHEIGHT    17        //the height of the face mask used in FindHead
#define MASKWIDTH     8         //the width of "
#define HEADCORTHOLD  0.12      //correlation threshold for FindHead
#define VTHOLDMIN     3.0       //minimum velocity threshold
//#define VTHOLDMAX     70        //maximum velocity threshold
#define COLOROUTPUT   1         //set for color BMP as output
#define CENTTHOLD     80        //centroid center distance threshold
#define XPTHOLD       70        //x position distance threshold
#define YPTHOLD       15        //y "
#define VOFFSET       20        //velocity offset

/* RTDX */
#define MAX_ELEMENTS  3750

/*
********************************************************************************
*                               DATA TYPES
********************************************************************************
*/
//typedef unsigned char BYTE;  //8-bit data type

typedef BYTE BINARY[WIDTH];     //Binary is used to represent a Binary image;
                                //can be used in the following ways:
                                // Binary image[HEIGHT];
                                // Binary *image; image = malloc(sizeof(Binary[HEIGHT2]));

typedef BYTE RAW_DATA[WIDTH];   //single channel of raw image data

typedef struct COORD {          //coordinate data for bounding box and camera positioning
    int x,y;                    //x = rows ; y = columns
} COORD;

/*
********************************************************************************
*                               GLOBAL VARIABLES
********************************************************************************
*/
float Reciprocal[256];          //used to store the reciprocal lookup table

/* declare a global input channel                                   */
RTDX_CreateInputChannel( ichan );
RTDX_CreateOutputChannel( ochan );


/*
********************************************************************************
*                               PROTOTYPES
********************************************************************************
*/

/* BITMAP24 host functions */
//void BitmapToBinary(BITMAP24*, BINARY*);     //converts a bitmap into a binary
//void BinaryToBitmap(BINARY*, BITMAP24*);     //converts a binary into a bitmap
//void BW_BitmapToBinary(BITMAP24*, BINARY*);  //takes a B&W bitmap and creates a binary mask
//void DrawBox(BITMAP24*, COORD*);         //draw a bounding box around coordinates

/* RAW_DATA host functions */
//void BitmapToRaw(BITMAP24*, RAW_DATA*, int);  //store a channel of a BMP into a raw_data

/* RAW_DATA target functions */
void BW_RawToBinary(RAW_DATA*, BINARY*);  //convert a B&W (0,255) to a binary (0,1) Binary
void RawToBinary(RAW_DATA*, RAW_DATA*, BINARY*);  //convert RAW_DATA to Binary
void PrintBinary(BINARY*);               //displays a binary to the screen
void CopyBinary(BINARY*, BINARY*);       //used to copy a binary image into new struct

/* target filters */
BYTE SkinToneFilter(BYTE,BYTE);            //computes an r-g ratio and returns a boolean
void MaskFilter(BINARY*, BINARY*, BINARY*);  //performs the masking of an image, returns new BINARY
void DifferenceFilter(BINARY*, BINARY*, BINARY*);  //finds the difference returns pointer to new Binary mask
                                             // between two frames
/* target tracking functions */
int FindHead(BINARY*, COORD*);          //find coordinates of the head

/* RTDX */
void ReadRawData(RAW_DATA*);


/*
********************************************************************************
*                               FUNCTIONS
********************************************************************************
*/

void ReadRawData(RAW_DATA* raw_data) {
    int arraydata[MAX_ELEMENTS];
    int status,k,l;
    int m = 0, n = 0;
    int unpack;
    int pack_count = 0;
    for (l = 0; l < 120000/(4*MAX_ELEMENTS); ++l) {
        /* receive an array of integers from the host               */
        status = RTDX_read( &ichan, arraydata, sizeof(arraydata) );
        if ( status != sizeof(arraydata) ) {
                printf( "ERROR: RTDX_read failed!\n" );
                exit( -1 );
        } else
            printf("Read block %d.\n",l);

        k = 0; //number of incoming BYTEs into the RAW_DATA bg_mask
        for (k = 0; k < sizeof(arraydata)/4; ++k) {
             unpack = arraydata[k];
             m = n*4;
             for (pack_count = 3; pack_count >= 0; --pack_count) {
                 raw_data[m/WIDTH][m%WIDTH+pack_count] = unpack&0x000000FF;
                 unpack = unpack>>8;
             }
             n++;
        }//for k
    }//for l
}


/*
********************************************************************************
* DESCRIPTION: mask a Binary image based off in input mask
*
* ARGUMENTS : BINARY*, BINARY*, BINARY*
*
* RETURNS : void
*
* NOTES : third parameter is the output pointer
********************************************************************************
*/
void MaskFilter (BINARY *img, BINARY *mask, BINARY *output) {
    int i, j;


    for ( i = BOTTOMCROP ; i <= TOPCROP; ++i )     //taking forbidden region
        for ( j = 0 ; j < WIDTH ; ++j )            // into account,
            output[i][j] = img[i][j] & mask[i][j]; //mask off non-background pixels
}

/*
********************************************************************************
* DESCRIPTION: compute the difference between two binary images
*
* ARGUMENTS : BINARY*, BINARY*
*
* RETURNS :
*
* NOTES : output = img1 - img2
********************************************************************************
*/
void DifferenceFilter (BINARY *img1, BINARY *img2, BINARY *output) {
    int i, j;

    for ( i = BOTTOMCROP ; i <= TOPCROP ; ++i )
        for ( j = 0 ; j < WIDTH ; ++j )
            output[i][j] = img1[i][j] - img2[i][j];

}

/*
********************************************************************************
* DESCRIPTION: copy a binary image
*
* ARGUMENTS : BINARY*, BINARY*
*
* RETURNS : void
*
* NOTES : second parameter is a pointer to where you want the copy
********************************************************************************
*/
void CopyBinary (BINARY *from, BINARY *to) {
    int i, j;
    for ( i = 0 ; i < HEIGHT ; ++i )
        for ( j = 0 ; j < WIDTH ; ++j )
            to[i][j] = from[i][j];
}


/*
********************************************************************************
* DESCRIPTION: convert two RAW_DATA channels (RED,GREEN) into a binary image
*              basd off a skin tone filter
*
* ARGUMENTS : red RAW_DATA*, green RAW_DATA*, skinmask BINARY*
*
* RETURNS :
*
* NOTES :
********************************************************************************
*/
void RawToBinary (RAW_DATA *r, RAW_DATA *g, BINARY *skinmask) {
    int i, j;


    #if BDEBUG
        printf("Skin tone filtering RAW_DATA channels...\n");
    #endif

    /* write 0 to bottom fobidden region */
    for ( i = 0 ; i < BOTTOMCROP; ++i)
        for( j = 0; j < WIDTH; ++j)
            skinmask[i][j] = 0;

    /* write 0 to top forbidden region */
    for ( i = HEIGHT-1 ; i > TOPCROP ; --i )
        for( j = 0 ; j < WIDTH ; ++j )
            skinmask[i][j] = 0;

    /* perform SkinToneFilter'ing on the rest */
    for ( i = BOTTOMCROP ; i <= TOPCROP ; ++i)
        for( j = 0 ; j < WIDTH ; ++j )
            skinmask[i][j] = SkinToneFilter( r[i][j],
                                               g[i][j]);


    #if BDEBUG
        printf("Finished skin tone filtering.\n");
    #endif
}

/*
********************************************************************************
* DESCRIPTION: convert a B&W to BINARY
*
* ARGUMENTS : RAW_DATA*, BINARY*
*
* RETURNS : void
*
* NOTES :
********************************************************************************
*/
void BW_RawToBinary (RAW_DATA *r, BINARY *mask) {
    int i, j;


    #if BDEBUG
        printf("Converting B&W RAW_DATA to binary...\n");
    #endif

    for ( i = BOTTOMCROP ; i < TOPCROP ; ++i )
        for ( j = 0 ; j < WIDTH ; ++j ) {
            if ( r[i][j] )
                mask[i][j] = 1;
            else
                mask[i][j] = 0;
    }

    #if BDEBUG
        printf("Finished conversion.\n");
    #endif
}

/*
********************************************************************************
* DESCRIPTION: perform division of a RED pixel by a GREEN pixel
*
* ARGUMENTS : red BYTE, green BYTE
*
* RETURNS : BYTE
*
* NOTES : this function has been optimized through the use of a Reciprocal
*         lookup table defined globally.
********************************************************************************
*/
BYTE SkinToneFilter (BYTE r, BYTE g) {
    const double skin_min = 1.24;    //min value for skin ratio
    const double skin_max = 1.42;    //max value for skin ratio
    double RG_ratio;
    double gg;

    if ( r == 0 || g == 0 )  //do not divide if one of the colors are 0
        return 0;
    else {
        gg = Reciprocal[(int)g];  //lookup g's reciprocal
        RG_ratio = (double) r*gg; //computes the ratio and sets it as a double
        if ( (skin_min <= RG_ratio) && (RG_ratio <= skin_max) )
            return 1;
        else
            return 0;
    }
}

/*
********************************************************************************
* DESCRIPTION: displayed a BINARY image to the screen
*
* ARGUMENTS : BINARY*
*
* RETURNS : void
*
* NOTES :
********************************************************************************
*/
void PrintBinary (BINARY *bin) {
    int i, j=0;


    if (j%WIDTH == 0)
        getchar();
    for (i = 0; i < HEIGHT; ++i)
        for (j = 0; j < WIDTH; ++j)
           printf("image at %d,%d: %d\n", i,j,bin[i][j]);
}

/*
********************************************************************************
* DESCRIPTION: returns the coordinate of the center of the rectangle with the
*              largest white pixel percentagemask a Binary image based off in
*              input mask.
*
* ARGUMENTS : BINARY*, BINARY*, BINARY*
*
* RETURNS : void
*
* NOTES : third parameter is the output pointer
********************************************************************************
*/
int FindHead (BINARY *frame, COORD *centroid) {
    BYTE subFrame[MASKHEIGHT][MASKWIDTH];
    BYTE facemask[MASKHEIGHT][MASKWIDTH];
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
                printf("correlation \t\t\t%1.5f\n", cor);
                centroid->x = i - (int)(MASKHEIGHT>>1);
                centroid->y = j + (int)(MASKWIDTH>>1);
                return 1;
            }//if
        }//for j
    }//for i

    centroid->x = -1;
    centroid->y = -1;

    return 0;
}

/*
********************************************************************************
*                               MAIN FUNCTION
********************************************************************************
*/
int main() {
    RAW_DATA bg_raw_data[HEIGHT];
    RAW_DATA raw_red[HEIGHT];
    RAW_DATA raw_green[HEIGHT];
    BINARY bin_frame[HEIGHT];          //binary version of frame
    BINARY old_bin_frame[HEIGHT];      //pointer to previous input binary frame
    BINARY diff_bin_frame[HEIGHT];     //temporary binary frame
    BINARY bg_bin_mask[HEIGHT];        //binary version of mask
    COORD *centroid;            //current centroid
    COORD *cent_old;            //old centroid
    COORD *predicted;           //predicted centroid
    int found_head = 0;         //flag to indicated a head was not found (initial)
    int i,j;                    //general purpose array index
    int v_thold;                //current velocity threshold
    int v_old, v_older, v;      //three stored velocity values
    int dx, dy;                 //change in x and y

    centroid = malloc(sizeof(COORD));
    cent_old = malloc(sizeof(COORD));
    predicted = malloc(sizeof(COORD));

    /* Initial Conditions */
    centroid->x = BOTTOMCROP;   //starting position of search camera and tracking camera
    centroid->y = WIDTH>>1;
    cent_old->x = centroid->x;
    cent_old->y = centroid->y;
    predicted->x = centroid->x;
    predicted->y = centroid->y;
    v_old = 200;
    v_older = 200;


    /* The following two lines of code creates the lookup table used for
     * SkinToneFilter.  1/i is stored as a float in the reciprocal array
     * at location Reciprocal[i].
     */
     printf("%f\n", 0.5);
    for (i = 0; i < 256; ++i){
        Reciprocal[i] = (float) (1/((float) i));  //Global variable reciprocal
        //printf("reciprocal of %d: %f\n", i, Reciprocal[i]);
        }
    TARGET_INITIALIZE();

    /* enable the input channel                                 */
    RTDX_enableInput( &ichan );
    printf("Input channel enabled.\n");

    printf("Receive background raw data...\n");
    ReadRawData(bg_raw_data);
    printf("Background raw data read successfully.\n");

    BW_RawToBinary(bg_raw_data,bg_bin_mask);

    ////////////////////////////
    //      READ FRAMES       //
    ////////////////////////////
    for (i = 1; i <= NUMPICS; ++i) {

        printf("Receive red raw data...\n");
        ReadRawData(raw_red);
        printf("Receive green raw data...\n");
        ReadRawData(raw_green);

#if 0
    for (i = 0; i < HEIGHT; ++i) {
        for (j = 0; j < WIDTH; ++j) {
	        printf("%03d, %03d: %08x\n",i, j, red_raw_data[i][j]);
	    }
	}
#endif

        RawToBinary(raw_red, raw_green, bin_frame);

        //perform differencing to detect motion
        DifferenceFilter(bin_frame, old_bin_frame,diff_bin_frame);

        //update old binary frame
        CopyBinary(bin_frame, old_bin_frame);

        //mask off the background regions and update binary frame
        MaskFilter(bin_frame, bg_bin_mask, bin_frame);

        //find head and compute centroid
        found_head = FindHead(bin_frame, centroid);
        if (!found_head) {
            printf("No head found.\n");
            //Compute velocity
            dx = (centroid->x - cent_old->x) >> FPSPOW2;
            dy = (centroid->y - cent_old->y) >> FPSPOW2;

            v = (int) fabs(dx) +  (int) fabs(dy);//approximate manhattan distance (velocity)
            printf("velocity\t\t\t%d\n",v);

            /* Adaptive Velocity Threshold */
            v_thold = (v_old + v_older) >> 1;
            v_thold += VOFFSET;
            printf("v_thold\t\t\t\t%d\n", v_thold);
            if (v < v_thold) {
                centroid->x = predicted->x; //cent_old->x;
                centroid->y = predicted->y; //cent_old->y;
                //UPDATE
                v_old = v;
                v_older = v_old;
            }
            else {
                centroid->x = cent_old->x;
                centroid->y = cent_old->y;
            } /* END */

        } else {
            //Compute velocity
            dx = (centroid->x - cent_old->x) >> FPSPOW2;
            dy = (centroid->y - cent_old->y) >> FPSPOW2;
            //dx_avg = (dx + dx_1 + dx_2)*1/n;

            /* use a VTHOLDMIN of 3.0 & VOFFSET 15 */
            //v = sqrt(pow(dx,2) + pow(dy,2));
            /* approximate manhattan distance (velocity)
             * use VTHOLDMIN =      VOFFSET =
             */
            v = (int)fabs(dx) + (int)fabs(dy);

            printf("velocity\t\t\t%d\n",v);

            /* Adaptive Velocity Threshold */
            v_thold = (v_old + v_older) >> 1;
            v_thold += VOFFSET;
            printf("v_thold\t\t\t\t%d\n", v_thold);
            // Intelligence
            if ( (v > VTHOLDMIN)           //minimum velocity threshold, don't move bb
              && (v < v_thold) ) {       //threshold above maximum, don't follow

                //UPDATE
                v_old = v;
                v_older = v_old;

                // Predictive Threshold
                if ( (abs(centroid->x - predicted->x) <= XPTHOLD)
                  || (abs(centroid->y - predicted->y) <= YPTHOLD) ) {
                    printf("Predicted (y,x)\t\t\t%d,%d\n",predicted->y,predicted->x);
                    #if 1
                    printf("Difference (y,x)\t\t%d,%d\n",abs(centroid->y - predicted->y),abs(centroid->x - predicted->x));
                    printf("Manhattan Difference: \t\t%d\n",abs(abs(centroid->y - predicted->y)-abs(centroid->x - predicted->x)));
                    #endif
                } else {
                    centroid->x = predicted->x;
                    centroid->y = predicted->y;
                }
            } else {                        //normal movement, track
                centroid->x = cent_old->x;  //restore previous values
                centroid->y = cent_old->y;
            }

            //Predicted Coordinates
            predicted->x = centroid->x + ((int)dx << FPSPOW2);
            predicted->y = centroid->y + ((int)dy << FPSPOW2);

            // Update history
            cent_old->x = centroid->x;
            cent_old->y = centroid->y;

        }//if (!found_head)

        #if DEBUGF
            printf("centroid for frame %d (c,r):\t%d,%d\n", i, centroid->y, centroid->x);
            printf("---------------------------------------\n");
        #endif

    }

    /* disable the input channel                                */
    printf("Disable Input Channel.\n");
    RTDX_disableInput(&ichan);

#if 0
    RTDX_enableOutput( &ochan );

    /* Send data to the host;
     * the parameter name in this example must be "ochan."
     */
    status = RTDX_write( &ochan, &result, sizeof(result) );

    if ( status == 0 ) {
        puts( "ERROR: RTDX_write failed!\n" );
        exit( -1 );
    }

    while ( RTDX_writing != NULL ) {
#if RTDX_POLLING_IMPLEMENTATION
    RTDX_Poll();
#endif
    }

    /* disable the output channel                               */
    RTDX_disableOutput( &ochan );
#endif

    printf("Program Complete!\n");
}
