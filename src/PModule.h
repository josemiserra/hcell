
#ifndef _PMODULE_
#define _PMODULE_

#include <iostream>
#include <string>
#include <map>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "PoolManager.h"
#include "MType.h"
using namespace cv;



class PModule
{

protected:
     shared_ptr<PoolManager> pool;
	 static bool noshow ;
public:

    PModule()
    {	
		pool = PoolManager::getInstance();
    }
	
     virtual ~PModule()
     {
     }

	/* executeAction: �Should be decoupled? The reason of having this method here is because:
		
		- the pointer function is dependent on the scope.  NameModule::Function or doesn�t work. Polymorphism is not allowed...
		- Each module can then perform specialized operations from the own module, having only one general class Action and not
		  one Action type for each module. -> which in the future can change, will see. 
	
	*/
	virtual void executeAction(string _current_action,vector<MType *> _realParams,unsigned int pid)=0;

	void static setShowOn() {  noshow = false; }
	void static setShowOff(){  noshow =true; }


/*****************************************************************************************************
*  GETCONTOURS
*  Given a set of objects creates the contours.
*  The algorithm is very simple: populate a matrix with the objects 
*   and remove each pixel surrounded by 8 pixels.
*******************************************************************************************************/
static void getContours(vloP &objects_in,vloP &contours,unsigned int rows, unsigned int cols)
{
	
	// void distanceTransform(const Mat& src, Mat& dst, int distanceType, int maskSize)
	// distanceTransform(const Mat& src, Mat& dst, int distanceType, int maskSize);

	Mat populated = Mat::zeros(Size(cols,rows),CV_32SC1);
	// Populate a matrix
	unsigned int i;
	unsigned int j;


	// Fill a matrix with the objects
	int count = 1;
	for (vloP::iterator it = objects_in.begin(); it!=objects_in.end(); ++it,++count)
	{	
		for (loP::iterator obj = it->begin(); obj!=it->end(); ++obj)
		{
			j = obj->x;
			i = obj->y;
			populated.at<int>(i,j) = count;				
		}
		contours.push_back(loP());
	}
	// Create binary mask

	int val,i_minus,j_minus,i_plus,j_plus;
	bool inner;
    for( i = 0; i < rows; ++i)
    {
        for ( j = 0; j < cols; ++j)
        {
			inner=false;
			val = populated.at<int>(i,j); 
		

		    if( val != 0) // if my pixel belongs to an object
			{  
				 if((i+1) == rows) i_plus = i;
				 else i_plus = i+1;
				 if( populated.at<int>(i_plus,j) != 0 && populated.at<int>(i_plus,j)==val) 
				 {
					if((j+1) == cols) j_plus = j;
					else j_plus = j+1;
						if( populated.at<int>(i,j_plus) != 0 && populated.at<int>(i,j_plus)==val)
						{
							if(i==0)  i_minus = i;
							else i_minus = i-1;
								if( populated.at<int>(i_minus,j) != 0 &&  populated.at<int>(i_minus,j)==val)
								{
								    
									if(j==0) j_minus = j;
									else j_minus = j-1;
											if( populated.at<int>(i,j_minus) != 0 &&  populated.at<int>(i,j_minus)==val)
											{
											 inner=true;
											}

								}

						}
					}
			if(!inner)
			{	
				contours[val-1].push_back(Point(j,i));
			}
		  }
        }
    }


	return;
}




/********************************************************************
*    DISPLAYS
*
*    Displays are mapped to 8 bit
  Whit noshow, any of this is executed;
***********************************************************************/

	void displayTo8C1(const char* output,const char* windowName){
	
		if(this->noshow) return;
		  Mat *timg=pool->getImage(output); 
		  double minVal, maxVal;
		  minMaxLoc(*timg, &minVal, &maxVal); 
		  Mat st;
		  timg->convertTo(st,CV_8UC1,255.0/(maxVal - minVal), -minVal * 255.0/(maxVal - minVal));
		  this->display(st,windowName);
	}


	 void display(Mat image,const char* windowsName="",int width=800,int height=600)
	 {
		if(this->noshow) return;
	 #ifdef  __linux__
		namedWindow( windowsName,  CV_WINDOW_AUTOSIZE );
		imshow(windowsName, image );
     #else
		namedWindow( windowsName,  CV_WINDOW_NORMAL );
		imshow(windowsName, image );
		cvResizeWindow(windowsName,width,height);
	 #endif	
		waitKey(0);
    
		}

	 void display(const char* output,string windowsName,int width=800,int height=600){
		  if(this->noshow) return;
		 Mat *timg=pool->getImage(output);
		 this->display(*timg,windowsName.c_str(),width,height);

	}

	 void display(string output,string windowsName,int width=800,int height=600){
			 if(this->noshow) return;
		     Mat *timg=pool->getImage(output.c_str());
			 this->display(*timg,windowsName.c_str(),width,height);
	}



};




#endif
