#include "ece4006header.h"

unsigned char skin_tone_filter(unsigned char r, unsigned char g){
	const double skin_min = 1.2;  // sets min value for ratio
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
