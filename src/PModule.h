
#ifndef _PMODULE_
#define _PMODULE_

#include <iostream>
#include <string>
#include <map>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "PoolManager.h"
#include "MType.h"
#include "MAllTypes.h"
using namespace cv;



class PModule
{
private:
	static vector<Scalar> colorTab;
	static int colorcounter;
protected:
	shared_ptr<PoolManager> pool;

public:

    PModule()
    {
		pool = PoolManager::getInstance();

		PModule::colorTab.reserve(256);
		int i;
	            for( i = 0; i < 256; i++ )
	            {
	            int b = theRNG().uniform(0, 255);
                int g = theRNG().uniform(0, 255);
                int r = theRNG().uniform(0, 255);
               
	             PModule::colorTab.push_back(Scalar(b,g,r));
				 PModule::colorcounter=0;
	            }

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


/****************************************************************************
 * getRandomColor
 * ----------------------------
 *  
 *
 ***************************************************************************/
	static Scalar getRandomColor(){ return PModule::colorTab[++PModule::colorcounter%256];  }

	
/********************************************************************
*     string to Color
* More info about colors:
* http://www.javascripter.net/faq/rgbtohex.htm 
*
*
***********************************************************************/

	Scalar* toColor(const char* col){
	    string color = col;

		if(col[0]=='#'){
			int red, green, blue;
			red=hexToR(color);
		    green=hexToG(color);
			blue = hexToB(color);
			return new Scalar(red,green,blue);
		}
		// if RGB(0,0,0) -> trim image	
		if(color.substr(0,2).compare("RGB")==0){
			const char * pch;
			char tmp[3];
			pch = color.c_str();
			int i,j;
			int c[3];
			j=0;
		    while (pch!=NULL)
			{
				i=0;
				
				while(isdigit(*pch)) { tmp[++i]=*pch; pch++;} 
				if(i>0){ c[j]=atoi(tmp); j++; }
				pch++;
			 }
			return new Scalar(c[0],c[1],c[2]);
			
		}
		
     if(color.compare("GREY")==0) return new Scalar(128,128,128);
	 if(color.compare("BROWN")==0) return new Scalar(205,135,65);
	 if(color.compare("BLACK")==0) return new Scalar(0,0,0);
	 switch(col[0]){
			case('B'): return new Scalar(255,0,0); break;
			case('G'): return new Scalar(0,255,0); break;
			case('R'): return new Scalar(0,0,255); break;
			case('Y'): return new Scalar(0,255,255); break; // Yellow
			case('M'): return new Scalar(255,0,255); break; // Magenta
			case('C'): return new Scalar(255,255,0); break; // Cyan	
			case('P'): return new Scalar(139,0,139); break; // Purple
			case('W'): return new Scalar(255,255,255); break; // Purple
			default:   ;
	 }
	// if black
		return new Scalar(0,0,0);

	}

/********************************************************************
*    DISPLAYS
*
*    Displays are mapped to 8 bit
***********************************************************************/

	void displayTo8C1(const char* output,const char* windowName){
	
		  Mat *timg=pool->getImage(output); 
		  double minVal, maxVal;
		  minMaxLoc(*timg, &minVal, &maxVal); 
		  Mat st;
		  (*timg).convertTo(st,CV_8UC1,255.0/(maxVal - minVal), -minVal * 255.0/(maxVal - minVal));
		   this->display(st,windowName);
	}


	 void display(Mat image,const char* windowsName="",int width=800,int height=600)
	 {
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
		 Mat *timg=pool->getImage(output);
		 this->display(*timg,windowsName.c_str(),width,height);

	}

	 void display(string output,string windowsName,int width=800,int height=600){
			 Mat *timg=pool->getImage(output.c_str());
			 this->display(*timg,windowsName.c_str(),width,height);

		}


	string cutHex(string h) { return (h.at(0) =='#') ? h.substr(1,6) : h; }
	int hexToR(string h) { 
							char * p;
							long n = strtol( h.substr(1,2).c_str(), & p, 16 );
							return n;};
   	int hexToG(string h) { 
							char * p;
							long n = strtol( h.substr(3,2).c_str(), & p, 16 );
							return n;};
   
    int hexToB(string h) { 
							char * p;
							long n = strtol( h.substr(5,2).c_str(), & p, 16 );
							return n;
							};
/*
	int inthash(int key)
	{
	key += ~(key << 15);
	key ^=  (key >> 10);
	key +=  (key << 3);
	key ^=  (key >> 6);
	key += ~(key << 11);
	key ^=  (key >> 16);
	return key;
}
*/

	static inline void combnames(string name,unsigned int n,string &newname)
	{
	/* char *buffer= new char[50];
	 sprintf(buffer,"%d", n);
	 strcat(buffer,name);
	 return buffer; */
	string _name;
#ifdef __linux__
	#if GCC_VERSION > 40600
		 _name = std::to_string(n);
	#else
		  _name = std::to_string(static_cast<long long>(n));
	#endif
#else
	       _name = to_string(static_cast<long long>(n));
#endif
	 _name.append(name);
	 newname=_name;
	 return;

	}

};




#endif
