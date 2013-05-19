/*
********************************************************************************
* ECE4006 - Real-Time DSP with Dr. Barnwell - Fall 2005
* Georgia Institute of Technology
*
* (c) Attribution-NonCommercial-ShareAlike 2.5
*     You are free: to copy, distribute, display, and perform the work
*                   to make derivative works
*
* Teacher_with_student images - May require renaming of files or change the
* %02d to %03d in the code. Save images in a relative images directory.
* Create the output directory.
*
* Filename: head_tracker.c
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
#include <stdio.h>
#include <stdlib.h>             //used for memory allocation operations
#include <math.h>               //used for velocity and time
#include <string.h>             //used for string sprintf and filenames
#include <time.h>               //used for algorithm timing
#include "bitmap24.h"           //handwritten BITMAP24 library

/*
********************************************************************************
*                               GLOBAL CONSTANTS
********************************************************************************
*/
/* DEBUG FLAGS */
#define DEBUGF 1                //general debug constant
#define PDEBUG 1                //timer debug flag

/* VISION PARAMETERS */
#define NUMPICS       58        //number of pictures in input directory images/
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

/*
********************************************************************************
*                               GLOBAL VARIABLES
********************************************************************************
*/
float Reciprocal[256];          //used to store the reciprocal lookup table

/*
********************************************************************************
*                               DATA TYPES
********************************************************************************
*/
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
*                               PROTOTYPES
********************************************************************************
*/

/* BITMAP24 host functions */
void BitmapToBinary(BITMAP24*, BINARY*);     //converts a bitmap into a binary
void BinaryToBitmap(BINARY*, BITMAP24*);     //converts a binary into a bitmap
void BW_BitmapToBinary(BITMAP24*, BINARY*);  //takes a B&W bitmap and creates a binary mask
void PrintBinary(BINARY*);               //displays a binary to the screen
void CopyBinary(BINARY*, BINARY*);       //used to copy a binary image into new struct
void DrawBox(BITMAP24*, COORD*);         //draw a bounding box around coordinates

/* RAW_DATA host functions */
void BitmapToRaw(BITMAP24*, RAW_DATA*, int);  //store a channel of a BMP into a raw_data

/* RAW_DATA target functions */
void BW_RawToBinary(RAW_DATA*, BINARY*);  //convert a B&W (0,255) to a binary (0,1) Binary
double RawToBinary(RAW_DATA*, RAW_DATA*, BINARY*);  //convert RAW_DATA to Binary

/* target filters */
BYTE SkinToneFilter(BYTE,BYTE);            //computes an r-g ratio and returns a boolean
void MaskFilter(BINARY*, BINARY*, BINARY*);  //performs the masking of an image, returns new BINARY
BINARY* DifferenceFilter(BINARY*, BINARY*);  //finds the difference returns pointer to new Binary mask
                                             // between two frames
/* target tracking functions */
int FindHead(BINARY*, COORD*);          //find coordinates of the head

/*
********************************************************************************
*                               FUNCTIONS
********************************************************************************
*/

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
* RETURNS : BINARY*
*
* NOTES : img1 - img2
********************************************************************************
*/
BINARY* DifferenceFilter (BINARY *img1, BINARY *img2) {
    int i, j;
    BINARY *output = malloc(sizeof(BINARY[HEIGHT]));


    for ( i = BOTTOMCROP ; i <= TOPCROP ; ++i )
        for ( j = 0 ; j < WIDTH ; ++j )
            output[i][j] = img1[i][j] - img2[i][j];


    return output;
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
* RETURNS : double
*
* NOTES : the return value is for clock cycle measurments
********************************************************************************
*/
double RawToBinary (RAW_DATA *r, RAW_DATA *g, BINARY *skinmask) {
    int i, j;
    clock_t start, finish;


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

    start = clock();

    /* perform SkinToneFilter'ing on the rest */
    for ( i = BOTTOMCROP ; i <= TOPCROP ; ++i)
        for( j = 0 ; j < WIDTH ; ++j )
            skinmask[i][j] = SkinToneFilter( r[i][j],
                                               g[i][j]);

    finish = clock();

    #if  PDEBUG
        printf("skin tone filter cycles\t\t%.0f\n", (double) (finish - start));
    #endif


    #if BDEBUG
        printf("Finished skin tone filtering.");
    #endif


    return (double) (finish - start);
}

/*
********************************************************************************
* DESCRIPTION: convert a RGB bitmat into a binary mask using a skin filter
*
* ARGUMENTS : BITMAP24*
*
* RETURNS : BINARY*
*
* NOTES :
********************************************************************************
*/
void BitmapToBinary (BITMAP24 *b, BINARY *skinmask) {
    int i, j;


    #if BDEBUG
        printf("Converting BITMAP24 to BINARY using skin filter...\n");
    #endif

    /* write black pixels of bottom fobidden region */
    for ( i = 0 ; i < BOTTOMCROP; ++i)
        for( j = 0; j < WIDTH; ++j)
            skinmask[i][j] = 0;

    /* write black pixels of top forbidden region */
    for ( i = HEIGHT-1 ; i > TOPCROP ; --i )
        for( j = 0 ; j < WIDTH ; ++j )
            skinmask[i][j] = 0;

    /* perform SkinToneFilter'ing on the rest */
    for ( i = BOTTOMCROP ; i <= TOPCROP ; ++i)
        for( j = 0 ; j < WIDTH ; ++j )
            skinmask[i][j] = SkinToneFilter( b->image[i][j][RED],
                                               b->image[i][j][GREEN]);

    #if BDEBUG
        printf("Finished BitmapToBinary conversion.");
    #endif
}

/*
********************************************************************************
* DESCRIPTION: convert a B&W BITMAP24 into a BINARY image
*
* ARGUMENTS : BITMAP24*, BINARY*
*
* RETURNS : void
*
* NOTES : The first channel is used from the 24 bit bitmap since all channels
*         are equal for BW BMPs
********************************************************************************
*/
void BW_BitmapToBinary (BITMAP24 *b, BINARY *mask) {
    int i, j;


    #if BDEBUG
        printf("Converting B&W bitmap to binary...\n");
    #endif

    for ( i = BOTTOMCROP ; i < TOPCROP ; ++i )
        for ( j = 0 ; j < WIDTH ; ++j ) {
            if ( b->image[i][j][RED] )
                /* anything other than "black" = 1 */
                mask[i][j] = 1;
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
        printf("Finished conversion.");
    #endif
}

/*
********************************************************************************
* DESCRIPTION: convert a selected channel RGB bitmap into RAW_DATA
*
* ARGUMENTS : BITMAP24*, RAW_DATA, int
*
* RETURNS : void
*
* NOTES :
********************************************************************************
*/
void BitmapToRaw (BITMAP24 *b, RAW_DATA *r, int channel) {
    int i, j;

    #if BDEBUG
        printf("Converting bitmap channel %d to RAW_DATA...\n",channel);
    #endif

    /* write black pixels of bottom fobidden region */
    for ( i = 0 ; i < BOTTOMCROP; ++i)
        for( j = 0; j < WIDTH; ++j)
            r[i][j] = 0;

    /* write black pixels of top forbidden region */
    for ( i = HEIGHT-1 ; i > TOPCROP ; --i )
        for( j = 0 ; j < WIDTH ; ++j )
            r[i][j] = 0;

    /* perform SkinToneFilter'ing on the rest */
    for ( i = BOTTOMCROP ; i <= TOPCROP ; ++i)
        for( j = 0 ; j < WIDTH ; ++j )
            r[i][j] = b->image[i][j][channel];

    #if BDEBUG
        printf("Finished conversion.");
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
* DESCRIPTION: BINARY to BITMAP24 conversion
*
* ARGUMENTS : BINARY*, BINARY24*
*
* RETURNS : void
*
* NOTES : use an already opened BITMAP24 as input
********************************************************************************
*/
void BinaryToBitmap (BINARY *bn, BITMAP24 *b) {
    int i, j;


    #if BDEBUG
        printf("Converting binary to bitmap...\n");
    #endif

#if 0
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
#endif

    /* overwrite bitmap's image data with that of binary's */
    for (i = 0; i < HEIGHT; ++i) {
        for (j = 0; j < WIDTH; ++j) {
            if (bn[i][j]) {
                b->image[i][j][RED] = 255;
                b->image[i][j][GREEN] = 255;
                b->image[i][j][BLUE] = 255;
            } else {
                b->image[i][j][RED] = 0;
                b->image[i][j][GREEN] = 0;
                b->image[i][j][BLUE] = 0;
            }
        }//for j
    }//for i
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
* DESCRIPTION: draw a bounding box around coordinates
*
* ARGUMENTS : BITMAP24*, COORD*
*
* RETURNS : void
*
* NOTES :
********************************************************************************
*/
void DrawBox (BITMAP24 *img, COORD *centroid) {
    BYTE r = 255;
    BYTE g = 255;
    BYTE b = 0;
    BYTE thickness = 2;
    BYTE dim_x = 30;
    BYTE dim_y = 40;
    BYTE half_x = (BYTE)(int)(dim_x>>1);  //casting as an int performs rounding w/o math.h
    BYTE half_y = (BYTE)(int)(dim_y>>1);
    int i,t;


    /* draw vertical sides */
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


    /* draw horizontal sides */
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
}

/*
********************************************************************************
*                               MAIN FUNCTION
********************************************************************************
*/
int main() {
    BITMAP24 *frame;            //pointer to input frame (RGB)
    BINARY *bin_frame;          //binary version of frame
    BINARY *old_bin_frame;      //pointer to previous input binary frame
    BINARY *diff_bin_frame;     //temporary binary frame
    BITMAP24 *bg_mask;          //background mask (read in as BMP)
    BINARY *bg_bin_mask;        //binary version of mask
    RAW_DATA *bg_raw_data;      //raw version of BG
    RAW_DATA *raw_red;          //raw data of frame RED channel
    RAW_DATA *raw_green;        // "                GREEN "
    COORD *centroid;            //current centroid
    COORD *cent_old;            //old centroid
    COORD *predicted;           //predicted centroid
    int found_head = 0;         //flag to indicated a head was not found (initial)
    int i;                      //general purpose array index
    int v_thold;                //current velocity threshold
    int v_old, v_older, v;      //three stored velocity values
    //int xp, yp;                 //predicted x and y
    int dx, dy;                 //change in x and y
    double time_to_compute;     //timer.h variable
    double cycles = 0, tempc;   //clock cycles computation variables
    char input[20], output[20];
    time_t t1, t2;              //time for determining algorithm time
    //clock_t start, finish;

    centroid = malloc(sizeof(COORD));
    cent_old = malloc(sizeof(COORD));
    predicted = malloc(sizeof(COORD));

    /* The following two lines of code creates the lookup table used for
     * SkinToneFilter.  1/i is stored as a float in the reciprocal array
     * at location Reciprocal[i].
     */
    for (i = 0; i < 256; ++i)
        Reciprocal[i] = 1/(float) i;  //Global variable reciprocal

    /* Initial Conditions */
    centroid->x = BOTTOMCROP;   //starting position of search camera and tracking camera
    centroid->y = WIDTH>>1;
    cent_old->x = centroid->x;
    cent_old->y = centroid->y;
    predicted->x = centroid->x;
    predicted->y = centroid->y;
    v_old = 200;
    v_older = 200;

    //create the backgroung mask
    bg_mask = malloc(sizeof(BITMAP24));
    BITMAP24_Read(bg_mask, "images/BGmask.bmp");
    bg_raw_data = malloc(sizeof(RAW_DATA[HEIGHT]));
    BitmapToRaw(bg_mask,bg_raw_data,RED);

    //send to bg_raw_data target over RTDX
    free(bg_mask);  //free the BG mask (bitmap)

    //target code
    bg_bin_mask = malloc(sizeof(BINARY[HEIGHT]));
    BW_RawToBinary(bg_raw_data,bg_bin_mask);

    //host code
    //allocate BMP frame of the scene
    //host
    frame = malloc(sizeof(BITMAP24));
    raw_red = malloc(sizeof(RAW_DATA[HEIGHT][WIDTH]));
    raw_green = malloc(sizeof(RAW_DATA[HEIGHT][WIDTH]));

    //target code
    //allocate binary image based on skin tone threshold
    bin_frame = malloc(sizeof(BINARY[HEIGHT]));
    //allocate old binary frame
    old_bin_frame = malloc(sizeof(BINARY[HEIGHT]));

    t1 = time(&t1);
    for (i = 1; i <= NUMPICS; ++i) {
        //read image
        sprintf(input,"images/img%02d.bmp",i);
        BITMAP24_Read(frame, input);
        BitmapToRaw(frame,raw_red,RED);
        BitmapToRaw(frame,raw_green,GREEN);

        //transfer to target raw_red & raw_green

        tempc = RawToBinary(raw_red, raw_green, bin_frame);
        cycles = tempc + cycles;
        //perform differencing to detect motion
        diff_bin_frame = DifferenceFilter(bin_frame, old_bin_frame);

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

            #if  0
                start = clock();
            #endif
            /* use a VTHOLDMIN of 3.0 & VOFFSET 15 */
            //v = sqrt(pow(dx,2) + pow(dy,2));
            /* approximate manhattan distance (velocity)
             * use VTHOLDMIN =      VOFFSET =
             */
            v = (int)fabs(dx) + (int)fabs(dy);

            #if 0
            finish = clock();
            printf("velocity computation cycles\t\t%4.0f\n", (double) (finish - start));
            #endif
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
                    #if 0
                    printf("Difference (y,x)\t\t%d,%d\n",abs(centroid->y - yp),abs(centroid->x - xp));
                    printf("Manhattan Difference: \t\t%d\n",abs(abs(centroid->y - yp)-abs(centroid->x - xp)));
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

        //draw bounding box to illustrate the second cameras view
        #if COLOROUTPUT
            DrawBox(frame, centroid); //to print color image
        #else
            //write out the frame for testing purposes
            BinaryToBitmap(bin_frame,frame); //comment out this line for color image
            DrawBox(frame, centroid); //to print bounding box on RGB image image
        #endif

        free(diff_bin_frame);
        //output stage
        sprintf(output,"output/img%02d_out.bmp",i);
        BITMAP24_Write(frame, output);
    }//for i

    t2 = time(&t2);
    time_to_compute = fabs(difftime(t2, t1));
    #if PDEBUG
        printf("Time to compute skintone filter in CPU Cycles %4.lf\n", cycles);
        printf("Average cycles per frame is %4.lf\n", cycles/NUMPICS);
    #endif

    free(centroid);
    free(cent_old);
    free(predicted);
    free(bg_bin_mask);
    free(bg_raw_data);
    free(frame);
    free(bin_frame);
    free(old_bin_frame);
    free(raw_red);
    free(raw_green);


    return 1;
}//main
