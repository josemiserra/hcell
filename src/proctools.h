#ifndef _PROCTOOLS_
#define _PROCTOOLS_

#include <opencv2/opencv.hpp>
#include "utils.h"


using namespace cv;

 class proctools
{
public:
	proctools(void);
	~proctools(void);

	/***********************************************************************************
	** OTSU_THRESH
	** Given a matrix returns the Otsu threshold values **
	************************************************************************************/
	double static otsu_thresh(Mat &ref, loP &objs)
	{
		float  w = 0;                // first order cumulative
		float  u = 0;                // second order cumulative
		float  uT = 0;               // total mean level

		// int    k = 255;              // maximum histogram index
		int    threshold = 0;        // optimal threshold value

		int	   hist[256];
		float  histNormalized[256];  // normalized histogram values

		float  work1, work2;		// working variables
		double work3 = 0.0;
		
		Mat ref2;
	/*	double min;
		double max;
	    minMaxIdx(ref, &min, &max);
		ref.convertTo(ref2,CV_8UC1,255.0/(max - min), -min * 255.0/(max - min)); */

		ref.convertTo(ref2,CV_8UC1,255,0);
		int i, j;
		int val;
		// Create histogram 
		for (int i=0; i<256; i++) hist[i] = 0;
		int count=0;
		for (loP::iterator it = objs.begin(); it!=objs.end(); ++it)
				{			    
				   j = it->x;
				   i = it->y;
				   val = (int) ref2.at<uchar>(i,j);
				   hist[val]++;
				   count++;
				   // 
				}
		// Create normalised histogram values
		// Calculate total mean level
		for (int i=0; i<256; i++){
									histNormalized[i]=hist[i]/(count*1.0);
									uT+=i*histNormalized[i]; 							
								}		

		// Find optimal threshold value
		for (int i=0; i<256; i++) {
			w+= histNormalized[i];
			u+= i*histNormalized[i];
			work1 = (uT * w - u);
			if (w != 0.0 && w != 1.0) work2 = (work1 * work1) / ( w * (1.0f-w) );
			else work2 = 0.0;
			if (work2>work3){ work3=work2;  threshold = i;  }

		}

		return threshold/255.0; 	
	};

};

#endif  // PROCTOOLS
