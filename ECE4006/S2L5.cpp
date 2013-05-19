//####################################################################
// S2L5.cpp - (Section 2, Lesson 5) SENDING AN ARRAY OF INTEGERS
//                                  USING SAFEARRAYS
//            This is the RTDX Host Client for Section 2, Lesson 5
//
// So basically, we have an input channel and an output channel,
// created on both the target and the host. The host code is very
// straightforward:
// The function SendRawData() basically opens the input channel,
// creates a safe array to send over it, fills the safe array with
// 3750 integers (4*3750 pixels, since we pack bytes into the integer),
// sends it, waits 2 seconds (a copp-out hard-code to ensure
// syncronization!) and then sends the next 3750 integers, until it
// has sent the whole color channel.
//
//
// This example sends integer values 1-10 to the target
//####################################################################

#import "rtdxint.dll"
#include <iostream.h>
#include <stdio.h>
#include "bitmap24.h"
#include <time.h>

#undef MAX_ELEMENTS
#define MAX_ELEMENTS 3750 //120000/(4*3750)=8 calls to RTDXwrite

/* VISION PARAMETERS */
#define NUMPICS       8        //number of pictures in input directory images/


using namespace RTDXINTLib;

void sleep( clock_t );
int SendRawData(BITMAP24*,int);

/* Pauses for a specified number of milliseconds. */
void sleep( clock_t wait )
{
   clock_t goal;
   goal = wait + clock();
   while( goal > clock() )
      ;
}

int SendRawData(BITMAP24* b, int channel) {
        IRtdxExpPtr rtdx;               // holds pointer to IRtdxExp interface
        long status;                    // holds status of RTDX COM API calls
        HRESULT hr;                     // holds status of generic COM API calls
        VARIANT sa;                     // holds a pointer to a SAFEARRAY
        SAFEARRAYBOUND rgsabound[1];    // set the dimension of the SAFEARRAY
        long bufferstate;               // holds the state of the host's write buffer
        long data;						// holds the element data of SAFEARRAY
		long numBytes;
        int i, j;
		long pack = 0;
        int pack_count = 1;
        long k;

        // initialize COM
        ::CoInitialize( NULL );

        // initialize VARIANT
        ::VariantInit( &sa );

        // instantiate the RTDX COM Object
        hr = rtdx.CreateInstance( __uuidof(RTDXINTLib::RtdxExp) );

        cout.setf(ios::showbase);

        if ( FAILED(hr) ) {
                cerr << hex << hr << " - Error: Instantiation failed! \n";
                return -1;
        }

        // open a channel (ichan) for writing
        status = rtdx->Open( "ichan", "W" );

        if ( status != Success ) {
                cerr << hex << status \
                     << " -  Error: Opening of channel \"ichan\" failed! \n";
                return -1;
        }

        // set the VARIANT to be a SAFEARRAY of 32-bit integers
        sa.vt = VT_ARRAY | VT_I4;

        // set the lower bound of the SAFEARRAY
        rgsabound[0].lLbound = 0;

        // set the number of elements in the SAFEARRAY
        rgsabound[0].cElements = MAX_ELEMENTS;

        // create the SAFEARRAY
        sa.parray = SafeArrayCreate( VT_I4, 1, rgsabound );

		// fill up the SAFEARRAY with values
        k = 0;
		for (i = 0; i < HEIGHT; ++i) {
			for (j = 0; j < WIDTH; ++j) {
			    data = (long)b->image[i][j][channel];
				pack = (pack | data);
				//if ( (i == 37) && (j < 220) && (j > 180) )
				//	printf("data = %08x\npack = %08x at (i,j,k) %d,%d,%d\n",data,pack,i,j,k);
				if (!(pack_count%4)) {
			        hr = ::SafeArrayPutElement( sa.parray, &k, (long*)&pack);
			        k++;
					if (k%MAX_ELEMENTS == 0 && k != 0) {
    		            // send data to the target
                        status = rtdx->Write(sa, &bufferstate);
			            if (status != Success) {
                            printf("Error at (i,j,k) %d,%d,%d\n",i,j,k);
                            cerr << hex << status << " - Error: Write failed!\n";
                            return -1;
						} else {
                            printf("Finished (i,j,k) %d,%d,%d write\n",i,j,k);
                            k = 0;
						}
			            while(rtdx->StatusOfWrite(&numBytes));
						/* Delay for a specified time. */
                        //printf( "Delay for 1 seconds\n" );
                        sleep( (clock_t)2 * CLOCKS_PER_SEC );
                        //printf( "Done!\n" );
					}
				}
				pack = pack<<8;
				++pack_count;
			}
		}

		cout << "Data Sent " << endl;
        // close the channel
        status = rtdx->Close();

        // release the RTDX COM Object
        rtdx.Release();

        // clear VARIANT
        ::VariantClear(&sa);

        // unitialize COM
        ::CoUninitialize();

		return 0;
}


int main()
{
	int i;
	char input[20];
	BITMAP24* frame;

	frame = (BITMAP24*) malloc(sizeof(BITMAP24));
    BITMAP24_Read(frame, "BGmask.bmp");

	SendRawData(frame,RED);

    for (i = 1; i <= NUMPICS; ++i) {
        sprintf(input,"images/img%02d.bmp",i);
        BITMAP24_Read(frame, "../../../images/img01.bmp");

        SendRawData(frame,RED);
		SendRawData(frame,GREEN);

		printf("Press any key when target is ready.\n");
		getchar();
	}

	return 0;
}
