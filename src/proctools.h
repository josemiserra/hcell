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


	double static normal_pdf(double x, double m, double s)
{
    static const double inv_sqrt_2pi = 0.3989422804014327;
    double a = (x - m) / s;

    return inv_sqrt_2pi / s * std::exp(-0.5 * a * a);
}

	/**********************************************
	* "Auto contrast adjustment" button of "Brightness/Contrast" 
	*  Auto contrast adjustment base on the one present in ImageJ
	*  Code idea from Kota Miura:
	*  http://cmci.embl.de/documents/120206pyip_cooking/python_imagej_cookbook#automatic_brightnesscontrast_button
	*
	*  Only valid for 8/16 bit images.
	*********************************************/

	 void static autocontrast(Mat &image, Mat &contrasted)
	{
	  
	  int pixelCount = (image.rows*image.cols);
	  int limit = int(pixelCount/10); // set limit number. We will set 0.05 percent of pixels
	  int threshold = int(pixelCount/(pixelCount*0.012));  // why 5000 ? 
	  int hmin,hmax;
	  image.copyTo(contrasted);
	 //  get an array of pixel intensity histogram of the current image.
	  bool nes;
	  if(image.depth()==CV_8U) nes = true;
	  else nes = false;
	  /// Set the ranges 
	  int *hist;
	  int foreground;
	  int histSize;

	 if(nes)
	  {
		  hist = new int[256];	
		  histSize = 256;
		  foreground = 255;  
	  }
	  else // is 16 bits
	  {
	   hist = new int[65536];	
	   histSize = 65536;
	   foreground = 65535;    
	  }
	  for( int i = 0; i<histSize; i++ ) hist[i]=0;
	  
	  for( int y = 0; y < image.rows; y++ )  
			for( int x = 0; x < image.cols; x++ )
				{
				 if(nes) hist[image.at<uchar>(y,x)]++;
				 else {
					 hist[image.at<ushort>(y,x)]++;
				     }
				}
      
	  int i = 0;
      bool found = false;
	  int counts;
	  do {
         counts =     hist[i];
		 ++i;
         if (counts > limit) counts = 0;
         found = counts > threshold;
		}while ((!found) && (i < histSize-1));
 
	  hmin=i-1;
 
	  i = histSize-1;
	  do {
         counts =hist[i];
		 --i;
         if (counts > limit) counts = 0; 
         found = counts > threshold;
		} while ((!found) && (i > 0));
      
	   hmax =i-1;
	    // Now we have hmin and hmax. 
	   image.convertTo(contrasted,image.depth(),foreground/(hmax - hmin), -hmin * foreground/(hmax - hmin));
	   delete hist;
	};
	

  void static binaryOp(Mat op1,Mat op2,Mat &op3, string operation){
	 
	     ut::Trace tr = ut::Trace("Binary op",__FILE__);
		// matrices are converted to the more restrictive type!!! 8,16,32,64
		// enum { CV_8U=0, CV_8S=1, CV_16U=2, CV_16S=3, CV_32S=4, CV_32F=5, CV_64F=6 };
		int type  = CV_32F;
		if(op1.depth()!=op2.depth())
		{
		 switch(op2.depth()){

			    case(CV_8U):{ op1.convertTo(op1,CV_8U);
							  type = CV_8U;
							  break;}
				case(CV_8S):{ op1.convertTo(op1,CV_8S); 
							  type = CV_8S;
							  break;}
				
				case(CV_16U):{  
								if(op1.depth()==CV_8U)
								{ 
								  op2.convertTo(op2,CV_8U); type = CV_8U;  break;
								}
								else
								if(op1.depth()==CV_8S)
								{ 
								 op2.convertTo(op2,CV_8S); type = CV_8S; break; }
								else{
								op1.convertTo(op1,CV_16U);
								type = CV_16U;
								}
								break;
							 }
				case(CV_16S):{  
								if(op1.depth()==CV_8U)
								{ op2.convertTo(op2,CV_8U); type = CV_8U; break; }
								else
								if(op1.depth()==CV_8S)
								{ 
								  op2.convertTo(op2,CV_8S); type = CV_8S;
								  break; }
								else
								{
								op1.convertTo(op1,CV_16S); type = CV_16S;
								}
								break;
							 }
				case(CV_32S):{ 
								if(op1.depth()==CV_8U)
								{ op2.convertTo(op2,CV_8U); 
								  type = CV_8U;
								  break; }
								else
								if(op1.depth()==CV_8S)
								{ op2.convertTo(op2,CV_8S); type = CV_8S; break; }
								else
								if(op1.depth()==CV_16U)
								{ op2.convertTo(op2,CV_16U); type = CV_16U; break; }
								else
							    if(op1.depth()==CV_16S)
								{ op2.convertTo(op2,CV_16S); type = CV_16S; break; }
								else
								{
								op1.convertTo(op2,CV_32S);
								type = CV_32S;
								}
								break;
							 }
				case(CV_32F):{ 								
								if(op1.depth()==CV_8U)
								{ op2.convertTo(op2,CV_8U); type = CV_8U; break; }
								else
								if(op1.depth()==CV_8S)
								{ op2.convertTo(op2,CV_8S); type = CV_8S; break; }
								else
								if(op1.depth()==CV_16U)
								{ op2.convertTo(op2,CV_16U); type = CV_16U; break; }
								else
							    if(op1.depth()==CV_16S)
								{ op2.convertTo(op2,CV_16S); type = CV_16S; break; }
								else
								op1.convertTo(op2,CV_32F);
								break; 
							 }
				case(CV_64F):{  
								if(op1.depth()==CV_8U)
								{ op2.convertTo(op2,CV_8U); type = CV_8U; break; }
								else
								if(op1.depth()==CV_8S)
								{ op2.convertTo(op2,CV_8S); type = CV_8S; break; }
								else
								if(op1.depth()==CV_16U)
								{ op2.convertTo(op2,CV_16U); type = CV_16U; break; }
								else
							    if(op1.depth()==CV_16S)
								{ op2.convertTo(op2,CV_16S); type = CV_16S; break; }
								else
								if(op1.depth()==CV_32S)
								{ op2.convertTo(op2,CV_32S);  type = CV_32S; break; }
								else
							    if(op1.depth()==CV_32F)
								{ op2.convertTo(op2,CV_32F);  break; }
								else
								{
								op2.convertTo(op2,CV_64F); type = CV_64F;
								}
								break; }
				default:{ cout << "ERROR" <<endl; exit(-1967);}
			
			}
		  
	  }
	  double a = 0.0;
	  switch(operation[0])
		{
			case('|'):{
						if((op1.rows==1 && op1.cols==1)||(op2.rows==1 && op2.cols==1))
						{
							return;  // it only can be done with same size arrays
						}
					    op3 = op1 | op2;
					    break;
					   }
			case('&'):{
						if((op1.rows==1 && op1.cols==1)||(op2.rows==1 && op2.cols==1))
						{
							return;  // it only can be done with same size arrays
						}
						op3 = op1 & op2;
					   break;
						}
		    case('+'):   // SAFE
						op3 = op1 + op2;
						break;
			case('-'):   // SAFE
						op3 = op1 - op2;
						break;
			case('.'):  // SAFE
						if(operation[1]=='-'){ absdiff(op1,op2,op3); break; }
						if(operation[1]=='|')
						{
							if((op1.rows==1 && op1.cols==1)||(op2.rows==1 && op2.cols==1)) return;  // it only can be done with same size arrays
							bitwise_xor(op1,op2,op3); break;
						}
						if(operation[1]=='*')
						{
							// SAFE
							// Calculates the per-element product of two arrays.
							op1.convertTo(op1,CV_32F);
							op2.convertTo(op2,CV_32F);
							 if((op1.rows==1 && op1.cols==1)||(op2.rows==1 && op2.cols==1))
							{
							return;  // it only can be done with same size arrays
							 }
							 else
							 { 
								multiply(op1,op2,op3,type);
								}
							break;
						}
					    break;
			case('/'):  // Performs per-element division of two arrays.

					   op1.convertTo(op1,CV_32F);
					   op2.convertTo(op2,CV_32F);
					   if(op1.rows==1 && op1.cols==1)
					   {
					   a = op1.at<float>(0,0);
					   op3 = a*op2;
					   }
					   else
					   { 
							if(op2.rows==1 && op2.cols==1)
							{
							a = op2.at<float>(0,0);
							op3 = a*op1;
							} 
							else
							{
								op3 = op1*op2;
							}
					   }
					   op3.convertTo(op3,type);
					   break;
			case('*'):  // SAFE
						// Calculates product of two arrays.
					   op1.convertTo(op1,CV_32F);
					   op2.convertTo(op2,CV_32F);
					   if(op1.rows==1 && op1.cols==1)
					   {
					   a = op1.at<float>(0,0);
					   op3 = a*op2;
					   }
					   else
					   { 
							if(op2.rows==1 && op2.cols==1)
							{
							a = op2.at<float>(0,0);
							op3 = a*op1;
							} 
							else
							{
								op3 = op1*op2;
							}
					   }
					   op3.convertTo(op3,type);
					   break;
			case('^'):{
					   if(op2.rows!=1 || op2.cols!=1)
					   {
					     cout<<"ERROR: Exponent in operation must be a value."<<endl;
						 exit(-1965);
					   }
					   pow(op1,op2.at<float>(0,0),op3);
					   break;
					  }
			default:
					cout<<"ERROR: Non existent operation";
					exit(-1958);
					break;
		}

	return;
};


  int static unaryOp(Mat op1,Mat &op3, string operation){
	 

	  ut::Trace tr = ut::Trace("Unary op",__FILE__);
	 // if(!(op1.rows>1 && op1.cols>1))
	  Mat op2;

	  switch(operation[0])
		{
			case('-'):{
					     op1.copyTo(op3);
						 op3 = -op3;
						 break;
					  }
			case('L'):{
					  log(op1,op3);
					    break;
					  }
			case('M'):{
					  if(operation.compare("MAX")==0)
					  {
						double minv,maxv;
						minMaxLoc(op1, &minv,&maxv);
						op3.at<double>(0,0)=maxv;
						break;
					  }
					  if(operation.compare("MIN")==0)
					  {
					    double minv,maxv;
						minMaxLoc(op1, &minv,&maxv);
						op3.at<double>(0,0)=minv;
					    break;
					  }
					  if(operation.compare("MEAN")==0)
					  {
					    op3 = mean(op1);
						break;
					   }
					   }
			case('S'):{
						if(operation.compare("SQRT")==0)
						{
						sqrt(op1,op3);
						break;
						}
						else  // SD
						{	
						Mat v_mean;
						Mat v_stddev;
						meanStdDev(op1, v_mean, v_stddev);
						op3=mean(v_stddev);
						}
					    break;
						}
			case('T'):{
						op3= cv::trace(op1);
					   break;
						}	
		    case('!'): 
			case('N'): 
					   if(op1.type()==CV_8U)  op3= 255 - op1;
					   if(op1.type()==CV_16U) op3= 65535 - op1;
					   if(op1.type()==CV_32F) op3=1 - op1;
					   else op3 = 1-op1;
					   break;
			default:
					cout<<"ERROR: Non existent operation";
					exit(-1960);
					break;
		}

	return 0;
};
  	template <typename mydata> 
  void static convertImageToVlop(Mat &Image,vloP &objects)
  {
	        double min,max;
	        minMaxIdx(Image, &min, &max);
			for(int i=0;i<=(int)max;i++) 
			{ 
				objects.push_back(loP());
			}

			if(max==0) return;

   			for( int i = 0; i < Image.rows; i++ )
					for( int j = 0; j < Image.cols; j++ )
						{	
							if(Image.at<mydata>(i,j)!=0)
							{
								objects[(int)Image.at<mydata>(i,j)].push_back(Point(j,i));
							}
						}
 
					objects.erase(objects.begin(),objects.begin()+1);
  }
// TO DO
  void static convertVlopToImage(Mat &Output,vloP &objects)
  {
  return;
  }
};

#endif  // PROCTOOLS
