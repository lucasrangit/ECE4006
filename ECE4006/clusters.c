#include "ece4006header.h"

/*INPUTS:
    Frame *image: a pointer to an image, which is a 2D array of integers ranging from 0 to 255, and a time value
    int threshold: how many white pixels must be found consecutively to consider it a cluster
    char dir: either 'x' or 'y', depending on whether you are sweeping the x- or y-dimension for clusters

(0,0)---------------y
|
|
|       image
|
x

OUTPUT:
    passes image by reference, so no returned image (changes the actual image in memory)
*/
void find_clusters(Frame *image, int threshold, char dir){
	int i, j, k, white_count;

	switch (dir) {
	case 'x':
		for(i = 0; i < Y_DIM; ++i){
			white_count = 0;
			for(j=0; j<X_DIM; ++j){
				if((*image).pixel[j][i] == 255)
					++white_count;
				else{
					if(white_count <= threshold){
						for (k = j-white_count; k < j-1; ++k)
							(*image).pixel[k][i] = 0;
						white_count = 0;
					}
				}
			}
		}
		break;
	case 'y':
		for(i=0; i<X_DIM; ++i){
			white_count = 0;
			for(j=0; j<Y_DIM; ++j){
				if((*image).pixel[i][j] == 255)
					++white_count;
				else{
					if(white_count <= threshold){
						for (k = i-white_count; k < i-1; ++k)
							(*image).pixel[k][j] = 0;
						white_count = 0;
					}
				}
			}
		}
		break;
	default:
		printf("error in find_clusters\n");
	}

}
