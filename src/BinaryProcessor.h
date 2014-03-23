#ifndef _BINARY_PROCESSOR_
#define _BINARY_PROCESSOR_

#include <opencv2/opencv.hpp>
#include "utils.h"

#define FOREGROUND 1
#define BACKGROUND 0



class BinaryProcessor
{
protected:
	 double parameterI;
	 double parameterII;
	 int _bsizex;
	 int _bsizey;
	 int iterations;
public:
	BinaryProcessor(void)
	{
		_bsizex = 3;
		_bsizey = 3;
		parameterI = 0;
		parameterII = 0;
		iterations = 1;
	};
	~BinaryProcessor(void){};

	void virtual run(Mat &in, Mat &out){};
	void setIterations(int iter){ iterations = iter;};
	void setParameterI(double pI){ parameterI = pI; };
	void setParameterII(double pII){ parameterII = pII;};
	void setBlockSize(int bsize){ _bsizex = bsize;  _bsizey = bsize; }
};

/** 
 * @brief Implements Bernsen's thresholding method
 *
 * @param[in] in_img Image pointer { grayscale }
 * @param[in] win_size Dimension of the thresholding window { positive-odd }
 * @param[in] contrast_threshold Contrast threshold { non-negative }
 *            
 * @return Pointer to the resulting binary image or NULL
 *
 * @reco Bernsen recommends WIN_SIZE = 31 and CONTRAST_THRESHOLD = 15.
 *
 * @ref 1) Bernsen J. (1986) "Dynamic Thresholding of Grey-Level Images" 
 *      Proc. of the 8th Int. Conf. on Pattern Recognition, pp. 1251-1255
 *      2) Sezgin M. and Sankur B. (2004) "Survey over Image Thresholding 
 *         Techniques and Quantitative Performance Evaluation" Journal of 
 *         Electronic Imaging, 13(1): 146-165 
 *         http://citeseer.ist.psu.edu/sezgin04survey.html
 *
 * @author M. Emre Celebi
 * @date 07.26.2007
 * @FROM FOURIER 0.8 
 */
class Bernsen:BinaryProcessor
{

public:
	Bernsen(int win_size, int contrast_threshold)
	{
		this->_bsizex = win_size;	
		this->_bsizey = win_size;
		this->parameterI = contrast_threshold;
	}
	~Bernsen(){};

	
   virtual void run(Mat &in, Mat &out)
	{
		switch(in.type())
		{
		case(CV_32F):  
			       Bernsen::thresh<float>(in,out,_bsizex,parameterI);
					break;
		case(CV_16U):
				   Bernsen::thresh<ushort>(in,out,_bsizex,parameterI);
					break;
		default:  // 8 bit
			       Bernsen::thresh<uchar>(in,out,_bsizex,parameterI);
		
		}
	}

	void setBlockSize(int win_size){ this->_bsizex = win_size;  this->_bsizey = win_size;}
	void setContrastThreshold(int contrast_thresh) { this->parameterI = contrast_thresh; }

	template <typename mydata> 
	static void thresh(Mat &x, Mat &thresholded, int &bsize, double &contrast)
	{
		ut::Trace tr=ut::Trace("__Threshold Bernsen",__FILE__);
		

		int num_rows, num_cols;
	    int half_win;
		int win_count;			/* number of pixels in the filtering window */
		int ir, ic;
		int iwr, iwc;
		int r_begin, r_end;		/* vertical limits of the filtering operation */
		int c_begin, c_end;		/* horizontal limits of the filtering operation */
		int wr_begin, wr_end;		/* vertical limits of the filtering window */
		int wc_begin, wc_end;		/* horizontal limits of the filtering window */
		mydata gray_val;
		mydata min_gray, max_gray;	/* min and max gray values in a particular window */
		mydata mid_gray;			/* mid range of gray values in a particular window */
		mydata local_contrast;		/* contrast in a particular window */
		mydata contrast_threshold;


		mydata MAX_GRAY, MID_GRAY;
		if(x.depth() == CV_8U)  
		{ MAX_GRAY = 255; MID_GRAY = 128; contrast_threshold = (int) contrast;  thresholded = Mat::zeros(x.rows,x.cols,CV_8U);}
		if(x.depth() == CV_16U) 
		{ MAX_GRAY = 65535; MID_GRAY = 32768; contrast_threshold = (int) contrast; thresholded = Mat::zeros(x.rows,x.cols,CV_16U);}
		if(x.depth() == CV_32F) 
		{ MAX_GRAY = 1.0; MID_GRAY = 0.5; contrast_threshold = contrast; thresholded = Mat::zeros(x.rows,x.cols,CV_32F); }


		half_win = bsize / 2;
		win_count = bsize * bsize;

		num_rows = x.rows;
		num_cols = x.cols;

		/* 
		Determine the limits of the filtering operation. Pixels
		in the output image outside these limits are set to 0.
		 */
		r_begin = half_win;
		r_end = num_rows - half_win;
		c_begin = half_win;
		c_end = num_cols - half_win;

		/* Initialize the vertical limits of the filtering window */
		wr_begin = 0;
		wr_end = bsize;

		/* For each image row */
		for ( ir = r_begin; ir < r_end; ir++ )
		{
			/* Initialize the horizontal limits of the filtering window */
			wc_begin = 0;
			wc_end = bsize;
			/* For each image column */
			for ( ic = c_begin; ic < c_end; ic++ )
			 {
			  min_gray = MAX_GRAY;  // 
			  max_gray = 0;
				/* For each window row */
				for ( iwr = wr_begin; iwr < wr_end; iwr++ )
				{
				/* For each window column */
				for ( iwc = wc_begin; iwc < wc_end; iwc++ )
				{
				 /* Determine the min and max gray values */
				 gray_val = x.at<mydata>(iwr,iwc);
				 if ( gray_val < min_gray )
				 {
					min_gray = gray_val;
				 }
				if ( max_gray < gray_val )
				 {
				 max_gray = gray_val;
				}
				}
			 }

     /* Calculate the local contrast and mid range */
     local_contrast = max_gray - min_gray;
     mid_gray = 0.5 * ( min_gray + max_gray );

     /* Determine the output pixel value */
     if ( local_contrast < contrast_threshold )
      {
       /* Low contrast region */
       thresholded.at<mydata>(ir,ic) = ( mid_gray >=(MID_GRAY) ) ? MAX_GRAY : 0;
      }
     else
      {
       thresholded.at<mydata>(ir,ic) = (x.at<mydata>(ir,ic) >= mid_gray ) ? MAX_GRAY : 0;
      }

     /* Update the horizontal limits of the filtering window */
     wc_begin++;
     wc_end++;
    }

   /* Update the vertical limits of the filtering window */
   wr_begin++;
   wr_end++;
  }

 return;
}


};

class Adaptive:BinaryProcessor
{

	public:
	Adaptive(int bsizex,int bsizey, double offset)
	{
		this->_bsizex = bsizex;	
		this->_bsizey = bsizey;
		this->parameterI = offset;
	}
	~Adaptive(){};

	
   virtual void run(Mat &in, Mat &out)
	{
			 
	   switch(in.type())
		{
		case(CV_32F):  
			       Adaptive::thresh<float>(in,out,_bsizex,_bsizey,parameterI,1.0);
					break;
		case(CV_16U):
					Adaptive::thresh<ushort>(in,out,_bsizex,_bsizey,parameterI,65535);
					break;
		default:  // 8 bit
			        Adaptive::thresh<uchar>(in,out,_bsizex,_bsizey,parameterI,255);
		
		}

	}

 template <typename mydata> 
 static void thresh(Mat &x, Mat &thresholded, int &dx, int &dy, double &offset,mydata foreground)
 {
 
	int  xi, yi, u, v;
    int sx, ex, sy, ey;
	mydata mean;
	int nFramePix;
	double sum;
	x.copyTo(thresholded);

	int nx = x.cols;
    int ny = x.rows;

    nFramePix = (2 * dx + 1) * (2 * dy + 1);
	
	
    for ( yi = dy; yi < ny - dy; yi++ ) {
            sum = 0.0;
            for ( xi = dx; xi < nx - dx; xi++ ) {
                if ( xi == dx) {
                /* first position in a row -- collect new sum */
                    for ( u = xi - dx; u <= xi + dx; u++ )
                        for ( v = yi - dy; v <= yi + dy; v++ )
							   sum += x.at<mydata>(v,u);
						        // sum += *(src+(u + v * nx));
                }
                else {
                /* frame moved in the row, modify sum */
                    for ( v = yi - dy; v <= yi + dy; v++ )
							 //  sum += src [xi + dx + v * nx] - src [ xi - dx - 1 + v * nx];
							 sum += x.at<mydata>(v,xi+dx) - x.at<mydata>(v,xi-dx-1);
                }
               
				/* calculate threshold and update tgt data */
			
                mean = (mydata)( (sum/(1.0*nFramePix))+(mydata)offset );
                sx = xi;
                ex = xi;
                sy = yi;
                ey = yi;
                if ( xi == dx ) {
                    /* left */
                    sx = 0.0;
                    ex = dx;
                }
                else
                if ( xi == nx - dx - 1 ) {
                    /* right */
                    sx = nx - dx - 1;
                    ex = nx - 1;
                }
                if ( yi == dy ) {
                    /* top */
                    sy = 0.0;
                    ey = dy;
                }
                else
                if ( yi == ny - dy - 1 ) {
                    /* bottom */
                    sy = ny - dy - 1;
                    ey = ny - 1;
                }
                if ( ex - sx > 0 || ey - sy > 0 ) {
                    for ( u = sx; u <= ex; u++ )
                        for ( v = sy; v <= ey; v++ )
						     thresholded.at<mydata>(v,u) =  ( x.at<mydata>(v,u)< mean ) ? 0 : foreground;
                }
                else /* thresh current pixel only */
				   thresholded.at<mydata>(yi,xi) =  ( x.at<mydata>(yi,xi)< mean ) ? 0: foreground;
            }
        }
    
    return;
 }
 
 };
#endif // _BINARY_PROCESSOR_