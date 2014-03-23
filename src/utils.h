#ifndef _UTILS_
#define _UTILS_

#if __linux__
	#include <X11/Xlib.h>
	#define GCC_VERSION (__GNUC__ * 10000 \
                               + __GNUC_MINOR__ * 100 \
                               + __GNUC_PATCHLEVEL__)

	#if GCC_VERSION < 40600
	const                        // this is a const object...
		class {
			public:
				template<class T>          // convertible to any type
				operator T*() const      // of null non-member
				{ return 0; }            // pointer...
				template<class C, class T> // or any type of null
				operator T C::*() const  // member pointer...
				{ return 0; }
			private:
				void operator&() const;    // whose address can't be taken
				} nullptr = {};              // and whose name is nullptr
	#endif
#endif
// #define _MY_DEBUG_

#ifdef WIN32
#include <Windows.h>
#else
#include <sys/time.h>
#include <ctime>
#endif

#include <time.h>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <opencv2/opencv.hpp>
// #include <opencv/highgui.h>

#define TOTAL_COLOR_NAMES 12

using namespace cv;
using namespace std;

typedef vector<Point> loP;
typedef vector<loP> vloP;



namespace ut{

	class utils
	{
			
		private:
			static vector<Scalar> colorTab;
			static int colorcounter;	
			

		public:
		static const string colourNames[TOTAL_COLOR_NAMES];
		utils(void)
			{
		
			
			}
		~utils(void){};
	
		static void initialise()
		{
			 utils::colorTab.reserve(1024);
			  int i;
	            for( i = 0; i < 1024; i++ )
	            {
	            int b = theRNG().uniform(0, 255);
                int g = theRNG().uniform(0, 255);
                int r = theRNG().uniform(0, 255);
               
				utils::colorTab.push_back(Scalar(b,g,r));
				utils::colorcounter=0;
				}
	
		}
/*###########  HELPER CLASS with static methods ############### */
/**********SEARCHING METHODS***********************************/

		template <class T>
		static bool  contains(const std::vector<T> &vec, const T &value)
		{
		return find(vec.begin(), vec.end(), value) != vec.end();
		};

		static bool abs_compare(int a, int b)
		{
		 return (std::abs(a) < std::abs(b));
		}

	/**********STRING MERGING WITH INTEGER***********************************/
	
   static inline void combnames(string str,unsigned int n,string &newname)
	{
	string _str1;
#ifdef __linux__
	#if GCC_VERSION > 40600
		 _str1 = std::to_string(n);
	#else
		  _str1 = std::to_string(static_cast<long long>(n));
	#endif
#else
	  _str1 = to_string(static_cast<long long>(n));
#endif
	 _str1.append(str);
	 newname=_str1;
	 return;
	}
/**********STRING UTILITIES***********************************/
		 static void trim(string& str)
		{
		 string::size_type pos = str.find_last_not_of(' ');
		 if(pos != string::npos) {
				 str.erase(pos + 1);
				 pos = str.find_first_not_of(' ');
				 if(pos != string::npos) str.erase(0, pos);
		}
		 else str.erase(str.begin(), str.end());
		}

/**********DISPLAY MANAGEMENT***********************************/		
// Obtain from the system the maximum horizontal resolution		
		static int getMaxScreenWidth(){
	
	#ifdef WIN32
		int fResult;
		fResult = GetSystemMetrics(SM_CXSCREEN); 
	    if(fResult==0)
		{
				cout<<"Monitor Resolution could not been obtained"<<endl;
				return 640;  // Minimum resolution nowadays...
		}
		else
		return fResult;	
	#else
		#if __linux__
			Display* disp = XOpenDisplay(NULL);
			Screen*  scrn = DefaultScreenOfDisplay(disp);
			int width  = scrn->width;
			return width;
		#endif
	#endif
		}
// Obtain from the system the maximum vertical resolution		
	static int getMaxScreenHeight(){
	
	#ifdef WIN32
		int fResult;
		fResult = GetSystemMetrics(SM_CYSCREEN); 
	    if(fResult==0)
		{
				cout<<"Monitor Resolution could not been obtained"<<endl;
				return 480; // Minimum resolution nowadays
		}
		else
		return fResult;	
	#else
		#if __linux__
			Display* disp = XOpenDisplay(NULL);
			Screen*  scrn = DefaultScreenOfDisplay(disp);
			int height = scrn->height;
			return height;
		#endif
	#endif
		}

/**********COLOUR MANAGEMENT***********************************/

/****************************************************************************
 * getRandomColor
 * ----------------------------
 *  
 *
 ***************************************************************************/
	static Scalar getRandomColor(){ return utils::colorTab[++utils::colorcounter%256];  }

	
/********************************************************************
*     string to Color
* More info about colors:
* http://www.javascripter.net/faq/rgbtohex.htm 
*
*
***********************************************************************/

	static Scalar* toColor(string color){

		if(color[0]=='#'){
			int red, green, blue;
			red=hexToR(color);
		    green=hexToG(color);
			blue = hexToB(color);
			return new Scalar(red,green,blue);
		}
		// if RGB(0,0,0) -> trim image	


		if(color.substr(0,3).compare("RGB")==0){
			const char * pch;
			char tmp[3];
			pch = color.c_str();
			int i,j;
			int c[3];
			j=0;

			while (*pch!=')')
			{
				i=0;
				pch++;
				tmp[0]='x';
				tmp[1]='x';
				tmp[2]='x';
				while(isdigit(*pch)) { tmp[i++]=*pch; pch++;} 
				if(i>0)
					{
					c[j]=atoi(tmp); 
					j++; 
					}
				 }
			return new Scalar(c[0],c[1],c[2]);
			
		}
		
     if(color.compare("GREY")==0) return new Scalar(128,128,128);
	 if(color.compare("BROWN")==0) return new Scalar(205,135,65);
	 if(color.compare("BLACK")==0) return new Scalar(0,0,0);
	 switch(color[0]){
			case('B'): return new Scalar(255,0,0); break;
			case('G'): return new Scalar(0,255,0); break;
			case('R'): return new Scalar(0,0,255); break;
			case('Y'): return new Scalar(0,255,255); break; // Yellow
			case('M'): return new Scalar(255,0,255); break; // Magenta
			case('C'): return new Scalar(255,255,0); break; // Cyan	
			case('P'): return new Scalar(139,0,139); break; // Purple
			case('W'): return new Scalar(255,255,255); break; // White
			default:   ;
	 }
	// if black
		return new Scalar(0,0,0);

	}

	static string cutHex(string h) { return (h.at(0) =='#') ? h.substr(1,6) : h; }
	static int hexToR(string h) { 
							char * p;
							long n = strtol( h.substr(1,2).c_str(), & p, 16 );
							return n;};
   	static int hexToG(string h) { 
							char * p;
							long n = strtol( h.substr(3,2).c_str(), & p, 16 );
							return n;};
   
    static int hexToB(string h) { 
							char * p;
							long n = strtol( h.substr(5,2).c_str(), & p, 16 );
							return n;
							}

	};

/****************Tracing class VERBOSING*****************************************/
	class Trace {

		private:
			string *theFunctionName;
		public:
			static bool traceIsActive;
		    static bool brief;

		 inline Trace (const char *name,const char* file) : theFunctionName(0)
		 {
			 if (traceIsActive) {
			#ifdef _MY_DEBUG_
				 cout << "Entering function :" << name <<endl;
				 cout << " File :" << file <<endl;
				 cout << " at " << __DATE__ << __TIME__ << endl;
			#endif
				 theFunctionName = new string(name);
		} 
	} 
	inline  ~Trace()
	{ 
		if (traceIsActive)
		{
				#ifdef _MY_DEBUG_
					cout << "Exit function " << *theFunctionName << endl;
				#endif
				delete theFunctionName;
		};
	}; 

/****************************************************************************
 * PRINT MESSAGES
 * ----------------------------
 *  
 *
 ***************************************************************************/

	inline void message(const char *message)
	{
			if (traceIsActive)
			{
			#ifdef _MY_DEBUG_
					cout << "This is the line number " << __LINE__ <<endl;
			#endif
					cout << message << endl;
			}
	};



   inline void message(const char *message, const int &m)
	{
			if (traceIsActive)
			{
			cout << message << m << endl; 
			}
	}

    inline void message(const char *message, float &m)
	{
			if (traceIsActive)
			{
			cout << message << m << endl; 
			} 
	}
	inline void message(const char *message,double &m)
	{
			if (traceIsActive) { 
			cout << message << m << endl; 
			} 
	}
	   
	inline void message(const char *m1,const char *m2)
	{
			if (traceIsActive)
			{
			cout << m1 << m2 << endl; 
			}

	}
/****************************************************************************
 * PRINT MATRIX
 * ----------------------------
 *  
 *
 ***************************************************************************/
	void printMatrixInfo(const char *m1,Mat &mat){
	
			
			if (traceIsActive && !brief) { 
			double minVal, maxVal;
			minMaxLoc(mat, &minVal, &maxVal); 

			cout << "--MATRIX NAME: "<< m1 << " --"<< endl; 
			cout << "Depth:";
			switch(mat.depth()){
// enum { CV_8U=0, CV_8S=1, CV_16U=2, CV_16S=3, CV_32S=4, CV_32F=5, CV_64F=6 };
				case(CV_8U):{ cout << " CV_8U."<< endl; break;   }
				case(CV_8S):{ cout << " CV_8S."<< endl; break;  }
				case(CV_16U):{ cout << " CV_16U."<< endl; break;  }
				case(CV_16S):{ cout << " CV_16S."<< endl; break;  }
				case(CV_32S):{ cout << " CV_32S."<< endl; break;  }
				case(CV_32F):{ cout << " CV_32F."<< endl; break;  }
				case(CV_64F):{ cout << " CV_64F."<< endl; break;  }
				default:{ cout << " " <<endl;}
			
			}
			cout << "Channels:" << mat.channels() << endl; 
			cout << " Min Val:" << minVal << " Max Val:" << maxVal <<endl;
			cout << "RES w x h: " << mat.cols << "x" << mat.rows <<endl; 
			cout << "##############################################################" <<endl;
	} 
	
	}

/****************************************************************************
 * PRINT MATRIX region
 * ----------------------------
 *  
 *
 ***************************************************************************/

	void printMatrixWH(const char *m1,Mat &mat,int width, int height){

		
		Mat mat2;
		Rect myROI(0,0, width, height);
	    Mat(mat, myROI ).copyTo(mat2);
		cout << m1 <<" = "<< endl << " "  << mat2 << endl << endl;

	}
/****************************************************************************
 * PRINT MATRIX
 * ----------------------------
 *  
 *
 ***************************************************************************/
	void printMatrix(cv::Mat &M){
	
		if(traceIsActive && !brief){
			cout << "M = "<< endl << " "  << M << endl << endl;
		}
	}

/* Returns the amount of milliseconds elapsed since the UNIX epoch. Works on both
 * windows and linux. */

int64 GetTimeMs64()
{
#ifdef WIN32
 /* Windows */
 FILETIME ft;
 LARGE_INTEGER li;

 /* Get the amount of 100 nano seconds intervals elapsed since January 1, 1601 (UTC) and copy it
  * to a LARGE_INTEGER structure. */
 GetSystemTimeAsFileTime(&ft);
 li.LowPart = ft.dwLowDateTime;
 li.HighPart = ft.dwHighDateTime;

 uint64 ret = li.QuadPart;
 ret -= 116444736000000000LL; /* Convert from file time to UNIX epoch time. */
 ret /= 10000; /* From 100 nano seconds (10^-7) to 1 millisecond (10^-3) intervals */

 return ret;
#else
 /* Linux */
 struct timeval tv;

 gettimeofday(&tv, NULL);

 uint64 ret = tv.tv_usec;
 /* Convert from micro seconds (10^-6) to milliseconds (10^-3) */
 ret /= 1000;

 /* Adds the seconds (10^0) after converting them to milliseconds (10^-3) */
 ret += (tv.tv_sec * 1000);

 return ret;
#endif
}

void printTime(double t1,double t2)
{
  cout << "Total time :" << (t2-t1)/1000 << " seconds" << endl;

}


// Get current date/time, format is YYYY-MM-DD.HH:mm:ss
const string currentDateTime() {
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);
    // Visit http://www.cplusplus.com/reference/clibrary/ctime/strftime/
    // for more information about date/time format
    strftime(buf, sizeof(buf), "%Y-%m-%d.%H_%M_%S", &tstruct);

    return buf;
}

void checkObject(const char* obj, vloP obj1, Mat ref)
{
		cout << "Evaluating object:"<< obj << endl;
		Mat test(ref.rows,ref.cols, CV_8UC1, Scalar(0));
		bool consistency = true;
		for(auto myIterator =obj1.begin();  myIterator != obj1.end(); myIterator++)
		{
			for(auto myIt2 = myIterator->begin(); myIt2!= myIterator->end();myIt2++)
			{
				if(test.at<uchar>(*myIt2)== 0)
				{ 
					test.at<uchar>(*myIt2)= 1;
				}
				else
				{
					consistency = false;
					cout<<"WARNING: Inconsistent object. Pixel intersection found." <<endl;
					cout<< "x:"<< (*myIt2).x <<"y:"<< (*myIt2).y <<endl;
				}
			}
			
		}
		if(consistency) cout << "----     Good Object consistency!" << endl;
		return;
}


};





}
#endif
