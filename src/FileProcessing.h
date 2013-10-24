#ifndef _FILE_PROCESSING_
#define _FILE_PROCESSING_

#include <iostream>
#include <map>
#include <opencv2/opencv.hpp>
#include "PModule.h"
#include "MType.h"
#include "MAllTypes.h"
#include "Action.h"
#include "utils.h"

#define PERCENT_USED 0.001

using namespace cv;
using namespace std;


class FileProcessing :
	public PModule
{ 

	public:
		typedef  void(FileProcessing::*Function)(vector<MType *>,unsigned int pid); // function pointer type
		static map<string, Function > tFunc;
		static map<string, int> strtoFormat;
		static map<string, int> normType;
	 
		enum FILEFORMATS {
							PNG,
							JPG,
							JP2,
							TIFF,
							PPM,
							PGM,
							PBM
						 };
			 
		enum NORM {			
							MAX,
							MAXMIN,
							LOG
						 };


	FileProcessing(){

		/*-----------FUNCTIONS-------------------------------------*/
	    tFunc["LOAD FILE"] = &FileProcessing::_loadFile;
		tFunc["NORMALIZE"] = &FileProcessing::_normalize;
		tFunc["WRITE FILE"] = &FileProcessing::writeFile;
		tFunc["CROP IMAGE"] = &FileProcessing::_cropImage;
		tFunc["RGB IMAGE"] = &FileProcessing::_rgbImage;
		tFunc["BLEND"] = &FileProcessing::_blend;
		tFunc["PAINT OBJECTS"] = &FileProcessing::_paintObjects;
		tFunc["SHOW"] = &FileProcessing::_show;
		tFunc["PRINT"] = &FileProcessing::_print;
		
		/*-----------FLAGS-------------------------------------*/
		strtoFormat["PNG"]=FILEFORMATS::PNG;
		strtoFormat["JPG"]=FILEFORMATS::JPG;
		strtoFormat["JPEG"]=FILEFORMATS::JPG;
		strtoFormat["TIFF"]=FILEFORMATS::TIFF;
		strtoFormat["TIF"]=FILEFORMATS::TIFF;
		strtoFormat["JP2"]=FILEFORMATS::JP2;
		strtoFormat["PGM"]=FILEFORMATS::PGM;
		strtoFormat["PBM"]=FILEFORMATS::PBM;
		strtoFormat["PPM"]=FILEFORMATS::PPM;

		normType["MAXMIN"]=NORM::MAXMIN;
		normType["LOG"]=NORM::LOG;
	};

	~FileProcessing(void)
	{	
	};
	
	virtual void executeAction(string current_action,vector<MType *> params,unsigned int pid)
	{	
		std::map<std::string, Function >::iterator x =  tFunc.find(current_action);
		if (x != tFunc.end())
		{
			(this->*(x->second))(params,pid);
		}
		return;
	}

/***************************************************************************
*
* SPECIFIC FUNCTIONS IMPLEMENTATION
*
***************************************************************************/

	void _show(std::vector<MType *> parValues, unsigned int pid)
	{
	
	   int height = (dynamic_cast<MIntType*>(parValues[0]))->getValue(); // HEIGHT 
	   string input= (dynamic_cast<MStringType*>(parValues[1]))->getValue(); //input
	   string itype	= (dynamic_cast<MStringType*>(parValues[2]))->getValue(); // Type of showing BROWSER or WINDOW (raster)	  
	   int width = (dynamic_cast<MIntType*>(parValues[3]))->getValue(); // WIDTH
	   const char *wName	= (dynamic_cast<MStringType*>(parValues[4]))->getValue(); // windowName
	   
	   // If name FILE put filename in window

	   // Select type and display with window or with raster

	    this->combnames(input,pid,input);
		Mat *img = pool->getImage(input.c_str());
		this->display(*img,wName,width,height); 
	
	
	}

	void _print(std::vector<MType *> parValues, unsigned int pid)
	{
	   
	   string message = (dynamic_cast<MStringType*>(parValues[0]))->getValue(); // HEIGHT 
	   cout<< message <<endl;
	}


/****************************************************************************
 * LOAD FILE
 * ----------------------------
 *  Loads a file into a image object
 *
 ***************************************************************************/

void _loadFile(std::vector<MType *> parValues, unsigned int pid)
{	

	   const char* filename	= (dynamic_cast<MStringType*>(parValues[0]))->getValue(); //filename
	   string output	= (dynamic_cast<MStringType*>(parValues[1]))->getValue();  // imageName (output>
	   
	   // string a4	= (dynamic_cast<MStringType*>(parValues[2]))->getValue(); // regexp
		
		 this->combnames(output,pid,output);


		 __loadFile(filename, output.c_str());

}

int __loadFile(const char* fileName, const char* imageName)
{
		 
		ut::Trace tr = ut::Trace("loadFile", __FILE__);
		Mat img;

			img = imread(fileName, CV_LOAD_IMAGE_ANYCOLOR | CV_LOAD_IMAGE_ANYDEPTH); //filename 
			if( !img.data )
			{ 
			   tr.message("Error reading file. Is the right directory?");
			   exit(-1910);
			}

			tr.message("\n In Load File:");
			tr.printMatrixInfo("Reading:",img);

			if(img.channels()>1)
			{
			cvtColor(img,img, CV_RGB2GRAY);
			}
			/**** Admitted types : CV_8UC1, CV_16U_C1 ********/
			/** FIRST version of the program only admitts 8U or 16U formats with only one channel***/
			if(!((img.type()==CV_8UC1)|(img.type()==CV_16UC1)))
			{
			img.convertTo(img,CV_16UC1); // Possible max/min adaptation
			}		

		 pool->storeImage(img,imageName);
		 return 0;		 
}


/****************************************************************************
 * DETECT
 * ----------------------------
 *  INPUT : Image
 *	OUTPUT : True if is empty and false if is blank
 *  Procedure is simple: if there are more than 0.001 % of white pixels in the 
 *  image, there is something in the image. 
 ***************************************************************************/
bool detectBlank(Mat &im)
{
	Mat thresholded;
	float max_size = floor(PERCENT_USED* im.rows * im.cols);
	Mat element = getStructuringElement( MORPH_ELLIPSE,
                                       Size( 2*3 + 1, 2*3+1 ),
                                       Point( 3, 3 ) );
	im.convertTo(thresholded,CV_8UC1);
	adaptiveThreshold( thresholded, thresholded, 255,ADAPTIVE_THRESH_GAUSSIAN_C,THRESH_BINARY, 21, 0);
	// display(thresholded,"afterAdapt");
	erode(thresholded, thresholded,element,Point(-1,-1),1);
	// display(thresholded,"afterErosion");
	int acum=0;
	for(int i = 0; i < thresholded.rows; i++)
	{
		 const uchar* Mi = thresholded.ptr<uchar>(i);
		 for(int j = 0; j < thresholded.cols; j++)
			if(Mi[j]>0) acum++;
		    if (acum>max_size) return false;
	}

	return true;
}


/****************************************************************************
 * NORMALIZE
 * ----------------------------
 *  Types : MAX MIN: common normalization using the maximum and minimum
 *			LOG : Logarithmic constrat normalization, costly, but brighter
 ***************************************************************************/
void _normalize(std::vector<MType *> parValues, unsigned int pid)
{
	   bool ashow =  (dynamic_cast<MBoolType*>(parValues[0]))->getValue(); // showImage (ASHOW)
	   string input= (dynamic_cast<MStringType*>(parValues[1]))->getValue(); //input
	   double maxint = (dynamic_cast<MDoubleType*>(parValues[2]))->getValue(); 
	   double minint = (dynamic_cast<MDoubleType*>(parValues[3]))->getValue(); 
	   string output	= (dynamic_cast<MStringType*>(parValues[4]))->getValue();  // imageName (output>
	   string itype	= (dynamic_cast<MStringType*>(parValues[5]))->getValue(); // Type of normalization (max/min, log...)
	   const char *wName	= (dynamic_cast<MStringType*>(parValues[6]))->getValue(); // windowName
	  

	   this->combnames(input,pid,input);
	   this->combnames(output,pid,output);


	   int type = normType[itype];
	   __normalize(input.c_str(),output.c_str(),type,maxint,minint);

	   if(ashow)
		{
		 Mat *img = pool->getImage(output.c_str());
		 this->display(*img,wName); 
	   }


}
void __normalize(const char* input, const char* output,int opt,double maxint=1.0,double minint=0.0){
	 
	 ut::Trace tr = ut::Trace("normalize",__FILE__);

	 Mat *image=pool->getImage(input);

	 if(image->channels()>1)
	 { 
			tr.message("Normalization cannot be done. Only images with 8 and 16 bits  allowed.");
			exit(-1911);
			return;
	 }

	 double min;
	 double max;

	 minMaxIdx(*image, &min, &max);

 // The convertScaleAbs function performs 3 operations: scale, compute absolute value, and convert to unsigned 8-bit type.
//  That's why the factor 255/max ensures the full range ([0-255] for unsigned 8-bit) is used. 
// convertScaleAbs(image, new_image, 255 / max);

    Mat image2;
	int type = image->type();
	
	tr.printMatrixInfo("Input before NORMALIZE",*image);

	if(maxint>1) tr.message("WARNING: maxint must be between 0 and 1");
	if(minint>1 || minint > maxint) tr.message("WARNING: maxint must be between 0 and 1 and less than maxint.");


	if(opt == NORM::MAX)
	{

		image->convertTo(image2, CV_32F);
		if(type==CV_8U)
			{
			for( int y = 0; y < image->rows; y++ )
			 for( int x = 0; x < image->cols; x++ )
              {	
				image2.at<float>(y,x)=image2.at<float>(y,x)/255.0;
			 }
			}
		if(type==CV_16U)
		{
		for( int y = 0; y < image->rows; y++ )
			 for( int x = 0; x < image->cols; x++ )
              {	
				image2.at<float>(y,x)=image2.at<float>(y,x)/65535.0;
			  }
		}
		else
		{
		  image->convertTo(image2,CV_32F,1.0/max,0);
		}
	
	}
	if(opt == NORM::MAXMIN)
	{ 

		if(maxint<1 || minint>0)
		{
			double top=max;
			if(type==CV_8U) top = 255.0;
			if(type==CV_16U)  top = 65535.0;
			image->convertTo(image2, CV_32F);

			for( int y = 0; y < image->rows; y++ )
			 for( int x = 0; x < image->cols; x++ )
              {	
				image2.at<float>(y,x)=image2.at<float>(y,x)/top;
				if(image2.at<float>(y,x)>maxint) image2.at<float>(y,x)=maxint;
				if(minint>0.0) image2.at<float>(y,x)=image2.at<float>(y,x)-minint;
				if(image2.at<float>(y,x)<0) image2.at<float>(y,x)=0;
				image2.at<float>(y,x)=image2.at<float>(y,x)/(maxint-minint);
			   } 
		
		if(type==CV_8U)	  image2.convertTo(*image,CV_8UC1,255,0);
		if(type==CV_16U)  image2.convertTo(*image,CV_16UC1,65535,0);
		if(type==CV_32F)  image2.copyTo(*image);
		}
		else
		{
			if(type==CV_8U)
			{
			image->convertTo(image2,CV_32F,1.0/(max - min), -min * 1.0/(max - min));
			image->convertTo(*image,CV_8UC1,255.0/(max - min), -min * 255.0/(max - min));
			
			}
			if(type==CV_16U)
			{
			image->convertTo(image2,CV_32F,1.0/(max - min), -min * 1.0/(max - min));
			image->convertTo(*image,CV_16UC1, 65535.0/(max - min), -min * 65535.0/(max- min));
			
			}
			else
			{
			image->convertTo(image2,CV_32F,1.0/(max - min), -min * 1.0/(max - min));
			}
		}
	}

	if(opt == NORM::LOG) 
	{
		image->convertTo(image2, CV_32F);
		for( int y = 0; y < image->rows; y++ )
			for( int x = 0; x < image->cols; x++ )
				{
				image2.at<float>(y,x)=((image2.at<float>(y,x))/(max));
				image2.at<float>(y,x)= 1-(log(image2.at<float>(y,x))/log(min/max));
				}
 
	  if(type==CV_8U) image2.convertTo(*image,type,255,0);
	  if(type==CV_16U) image2.convertTo(*image,type,65535,0);
	  if(type==CV_32F) image2.copyTo(*image);
	}
	tr.message("#####################"); 
	tr.printMatrixInfo("Input AFTER NORMALIZE",image2);
	 pool->storeImage(image2,output);
	 return;	 
	 }


/****************************************************************************
 * WRITE FILE
 * ----------------------------
 *  
 *
 ***************************************************************************/

void writeFile(std::vector<MType *> parValues, unsigned int pid)
{

	  ut::Trace tr = ut::Trace("writefile",__FILE__);
	  // 0,1 APPEND
	 string fileext	= (dynamic_cast<MStringType*>(parValues[2]))->getValue(); //fileext
	 const char* foutput	= (dynamic_cast<MStringType*>(parValues[3]))->getValue();  // output filename (foutput)
	 // string foutput	= (dynamic_cast<MStringType*>(parValues[4]))->getValue(); //imageName(input)
	 string input	= (dynamic_cast<MStringType*>(parValues[5]))->getValue(); //imageName(input)
	
	 this->combnames(input,pid,input);

	 int val=FileProcessing::strtoFormat[fileext];
	 
	 switch(val){
			 case FILEFORMATS::PNG:
								  {
									int paramId1 = (dynamic_cast<MIntType*>(parValues[6]))->getValue(); // ParamId2 P_COMPRESSION_LEVEL	
									use_PNG(foutput,input.c_str(),paramId1);
									break;
								   }
			 case FILEFORMATS::JPG:
								  {
									int paramId1 = (dynamic_cast<MIntType*>(parValues[6]))->getValue(); // ParamId2 P_COMPRESSION_LEVEL	
									use_JPG(foutput,input.c_str(),paramId1);
									break;
								  }
			 case FILEFORMATS::JP2:
								  {
									int paramId1 = (dynamic_cast<MIntType*>(parValues[6]))->getValue(); // ParamId2 P_COMPRESSION_LEVEL	
									use_JP2(foutput,input.c_str(),paramId1);
									break;
								  }
			  case FILEFORMATS::TIFF:
								 {
									// int paramId1 = (dynamic_cast<MIntType*>(parValues[6]))->getValue(); // ParamId2 P_COMPRESSION_LEVEL
								    use_TIFF(foutput,input.c_str());
									break;
								 }	
			  case FILEFORMATS::PPM:
								 {
									// int paramId1 = (dynamic_cast<MIntType*>(parValues[6]))->getValue(); // ParamId2 P_COMPRESSION_LEVEL
								    use_PM(foutput,input.c_str(),FILEFORMATS::PPM);
									break;
								 }
			   case FILEFORMATS::PGM:
								 {
									// int paramId1 = (dynamic_cast<MIntType*>(parValues[6]))->getValue(); // ParamId2 P_COMPRESSION_LEVEL
								    use_PM(foutput,input.c_str(),FILEFORMATS::PGM);
									break;
								 }
			   case FILEFORMATS::PBM:
								 {
									// int paramId1 = (dynamic_cast<MIntType*>(parValues[6]))->getValue(); // ParamId2 P_COMPRESSION_LEVEL
								    use_PM(foutput,input.c_str(),FILEFORMATS::PBM);
									break;
								 }

			 default:	{
							 tr.message("Error: File format for writing not recognised");
							 exit(-1912);
							 break; 
						}
	 };

};

int use_PNG(const char* filename,const char* imageName,int complevel);
int use_JPG(const char* filename,const char* imageName,int complevel);
int use_TIFF(const char* filename,const char* imageName);
int use_PM(const char* filename,const char* imageName,int code);
int use_JP2(const char* filename,const char* imageName,int complevel);

/****************************************************************************
 * CROP IMAGE
 * ----------------------------
 *  
 *
 ***************************************************************************/
void _cropImage(std::vector<MType *> parValues, unsigned int pid)
{
	

	 ut::Trace tr = ut::Trace("cropImage",__FILE__);

	 bool ashow =  (dynamic_cast<MBoolType*>(parValues[0]))->getValue(); // showImage (ASHOW)
	 int cleft = (dynamic_cast<MIntType*>(parValues[1]))->getValue();	//<COORDINATES_LEFT>500</COORDINATES_X>
	 int cright = (dynamic_cast<MIntType*>(parValues[2]))->getValue();	//<COORDINATES_RIGHT>700</COORDINATES_Y>
	 string input	= (dynamic_cast<MStringType*>(parValues[3]))->getValue();  // imageName (INPUT)
	 string output	= (dynamic_cast<MStringType*>(parValues[4]))->getValue();	//output	(OUTPUT)
	 const char* wName	= (dynamic_cast<MStringType*>(parValues[5]))->getValue(); // windowName
	 int wheight = (dynamic_cast<MIntType*>(parValues[6]))->getValue();	 // <WINDOW_HEIGHT>600</WINDOW_HEIGHT>
	 int wwidth = (dynamic_cast<MIntType*>(parValues[7]))->getValue();	 // <WINDOW_WIDTH>800</WINDOW_WIDTH>
		

	 this->combnames(input,pid,input);
	 this->combnames(output,pid,output);

	 Mat *img=pool->getImage(input.c_str());

	 Mat croppedimage;
	
	try
	{
     Rect myROI(cleft,cright, wwidth, wheight);
	 Mat((*img), myROI ).copyTo(croppedimage);	
	}
	catch( cv::Exception& e )
	{
		tr.message("Cropping size exceeding image boundaries.");
		#ifdef _MY_DEBUG_	
		tr.message("Error :",e.what());
		#endif
		exit(-1913);
	}	 
	 
	pool->storeImage(croppedimage,output.c_str()); // using a rectangle
	
	if(ashow)
	{
	this->display(croppedimage,wName);
	}
}
 
/****************************************************************************
 * RGB composition
 * ----------------------------
 *  
 *
 ***************************************************************************/
void _rgbImage(std::vector<MType *> parValues, unsigned int pid)
{ 
	 ut::Trace tr = ut::Trace("rgbImage",__FILE__);

	 bool ashow =  (dynamic_cast<MBoolType*>(parValues[0]))->getValue(); // showImage (ASHOW)
	 string binput	= (dynamic_cast<MStringType*>(parValues[1]))->getValue();  // B
	 string ginput	= (dynamic_cast<MStringType*>(parValues[2]))->getValue();  // G
	 string output	= (dynamic_cast<MStringType*>(parValues[3]))->getValue();	//output	(OUTPUT)
	 string rinput	= (dynamic_cast<MStringType*>(parValues[4]))->getValue();  // R
	 const char* wName	= (dynamic_cast<MStringType*>(parValues[5]))->getValue(); // windowName
	
	 this->combnames(binput,pid,binput);
	 this->combnames(ginput,pid,ginput);
	 this->combnames(rinput,pid,rinput);
	 this->combnames(output,pid,output);

	 __rgbImage(rinput.c_str(),ginput.c_str(),binput.c_str(),output.c_str(),pid);


	 if(ashow)
	 {
		 this->display(output,wName);
	 }
}

int __rgbImage(const char* redim, const char* greenim, const char* blueim,const char*output,unsigned int pid){

	Mat *redchx, *greenchx,*bluechx;
	Mat redch, greench,bluech;
	Mat color;
	bool rt, gt, bt;
	bt=gt=rt=false;
	int rws,cls;
	double minVal, maxVal;
	string nullf;
	this->combnames("NULL",pid,nullf);

	if(nullf.compare(redim)!=0)
	{
		redchx=pool->getImage(redim);
		redchx->copyTo(redch);
		rt=true;
		rws=redch.rows;
		cls=redch.cols;
		minMaxLoc(redch, &minVal, &maxVal); 
		if(redch.channels()>1) cvtColor(redch,redch, CV_RGB2GRAY);
		redch.convertTo(redch,CV_8UC1,255.0/(maxVal - minVal), -minVal * 255.0/(maxVal - minVal));
	}
	if(nullf.compare(greenim)!=0)
	{ 
	   cout << "Green is going to pring :)"<<endl;
	   greenchx=pool->getImage(greenim);
	   greenchx->copyTo(greench);
	   gt = true;
	   rws=greench.rows;
	   cls=greench.cols;	
	   if(greench.channels()>1)
	   {
	   cvtColor(greench,greench, CV_RGB2GRAY);
	   }
	    minMaxLoc(greench, &minVal, &maxVal); 
	   greench.convertTo(greench,CV_8UC1,255.0/(maxVal - minVal), -minVal * 255.0/(maxVal - minVal));
	}
	 if(nullf.compare(blueim)!=0)
	 { 
		 bluechx=pool->getImage(blueim);
		 bluechx->copyTo(bluech);
		 bt = true;
		 rws=bluech.rows;
	     cls=bluech.cols;
		 minMaxLoc(bluech, &minVal, &maxVal); 
		   if(bluech.channels()>1)
		   {
			   cvtColor(bluech,bluech, CV_RGB2GRAY);
		   }
		 bluech.convertTo(bluech,CV_8UC1,255.0/(maxVal - minVal), -minVal * 255.0/(maxVal - minVal));
 
	 }

	 Mat g =  Mat::zeros(Size(rws,cls), CV_8UC1);

	// Create a vector containing the channels of the new colored image
	std::vector<cv::Mat> channels;

	if(bt) channels.push_back(bluech);  // 1st channel
	else channels.push_back(g);

	if(gt) 	channels.push_back(greench);  // 2nd channel
	else channels.push_back(g);

	if(rt) 	channels.push_back(redch);  // 2nd channel
	else channels.push_back(g);
	
	cv::merge(channels, color);

	pool->storeImage(color,output);

	return 0;
}




/****************************************************************************
 * BLEND IMAGES
 * -------------------------------------------------------------------------
 *  
 *
 ***************************************************************************/
void _blend(std::vector<MType *> parValues, unsigned int pid)
{
	double alpha1 = (dynamic_cast<MDoubleType*>(parValues[0]))->getValue(); //output
	double alpha2 = (dynamic_cast<MDoubleType*>(parValues[1]))->getValue(); //output
	bool a0 =  (dynamic_cast<MBoolType*>(parValues[2]))->getValue(); // showImage (ASHOW)
	string input1 = (dynamic_cast<MStringType*>(parValues[3]))->getValue();
	string input2 = (dynamic_cast<MStringType*>(parValues[4]))->getValue();
	string output	= (dynamic_cast<MStringType*>(parValues[5]))->getValue(); //output
	const char* wName	= (dynamic_cast<MStringType*>(parValues[6]))->getValue(); // windowName


	 this->combnames(input1,pid,input1);
	 this->combnames(input2,pid,input2);
	 this->combnames(output,pid,output);

	__blend(input1.c_str(),input2.c_str(),output.c_str(),alpha1,alpha2);
	 
	if(a0){ display(output.c_str(),wName); }


}

void __blend(const char*input1,const char*input2,const char*output, double &alpha1,double &alpha2)
{

	Mat *img = pool->getImage(input1); 
	Mat *img1 = pool->getImage(input2);

	Mat copy1;
	if(img->channels()<3) cvtColor(*img,copy1,CV_GRAY2BGR);
	else img->copyTo(copy1);

	Mat copy2;
	if(img1->channels()<3) cvtColor(*img1,copy2,CV_GRAY2BGR);
	else img1->copyTo(copy2); 
	
	Mat out;
	addWeighted( copy1, alpha1, copy2, alpha2, 0.0, out);

	pool->storeImage(out,output);

}

/****************************************************************************
 *  DRAW CONTOURS from Objects or PAINT them into an image
 * --------------------------------------------------------
 *  
 *
 ***************************************************************************/
void _paintObjects(std::vector<MType *> parValues, unsigned int pid)
{

	 bool show =  (dynamic_cast<MBoolType*>(parValues[0]))->getValue(); // showImage (ASHOW)
	 const char* colour = (dynamic_cast<MStringType*>(parValues[1]))->getValue(); // colour	 
	 int height			= (dynamic_cast<MIntType*>(parValues[2]))->getValue(); 
	 string input	= (dynamic_cast<MStringType*>(parValues[3]))->getValue(); // input image
	 string lobj	= (dynamic_cast<MStringType*>(parValues[4]))->getValue();  // output name
	 string output	= (dynamic_cast<MStringType*>(parValues[5]))->getValue();  // output name
	 bool paintAsContours = (dynamic_cast<MBoolType*>(parValues[6]))->getValue();
	 int thickness	= (dynamic_cast<MIntType*>(parValues[7]))->getValue();  //line thickness of contour
	 int width = (dynamic_cast<MIntType*>(parValues[8]))->getValue(); 
	 const char* wName	= (dynamic_cast<MStringType*>(parValues[9]))->getValue(); // windows name

	 Scalar *color;
	 bool random = (strcmp(colour,"RANDOM")==0);
	 if(!random) color = toColor(colour); 
	 else color = new Scalar(0,0,0);
	 // colour to RGB function
	 
	 if(!(input.compare("BLANK_BINARY")==0 || input.compare("BLANK_COLOUR")==0))
	 {
	    this->combnames(input,pid,input);
	 }
	
	 this->combnames(output,pid,output);
	 this->combnames(lobj,pid,lobj);


	 this->__paintObjects(input.c_str(),output.c_str(),lobj.c_str(),*color,thickness, random,paintAsContours,width,height);
	
	 if(show){ 
		 Mat *img;
		 Mat show_im;
		 img =pool->getImage(output.c_str());
		 img->convertTo(show_im,CV_8UC3);
		 display(show_im,wName,width,height); }
	 delete color;
}

void __paintObjects(const char* input, const char* output, const char* sobj,Scalar color,int &thickness,bool random_color,bool paintAsContours,int width,int height){
  
	ut::Trace tr = ut::Trace("paintObjects",__FILE__);

	Mat *src;
	Mat cont;
	vloP  contours;
	vloP  *objects;

	if(!(strcmp(input,"BLANK_BINARY")==0 || strcmp(input,"BLANK_COLOUR")==0))
	 {
	  src=pool->getImage(input);
	  src->copyTo(cont);
	  if(src->channels()<2)
	  {
		cvtColor(*src,cont,CV_GRAY2BGR);
	   }

	  double minVal, maxVal;
	  minMaxLoc(cont, &minVal, &maxVal); 
	  cont.convertTo(cont,CV_8UC3,255.0/(maxVal - minVal), -minVal * 255.0/(maxVal - minVal));

	  tr.message("Painted matrix : \n");
	  tr.printMatrixInfo("i",*src);
	  }
	else{
	  cont = Mat::zeros(height,width,CV_8UC3);
	}
//------------------------------

	objects = pool->getlObj(sobj);

    if(thickness == 0) 
	{
			unsigned int i,j;
			if(paintAsContours)
			{ 
				this->getContours(*objects,contours,cont.rows,cont.cols);
				
				for (vloP::iterator it = contours.begin(); it!=contours.end(); ++it)
				{
					if(random_color) color = this->getRandomColor(); 
					for (loP::iterator obj = it->begin(); obj!=it->end(); ++obj)
					{
						j = obj->x;
						i = obj->y;				
						cont.at<Point3_<uchar> >(i,j) = Point3_<uchar>(color[0],color[1],color[2]);  
					}
				}
			}
			else
			{
				for (vloP::iterator it = objects->begin(); it!=objects->end(); ++it)
				{
					if(random_color) color = this->getRandomColor(); 
					for (loP::iterator obj = it->begin(); obj!=it->end(); ++obj)
					{
						j = obj->x;
						i = obj->y;				
						cont.at<Point3_<uchar> >(i,j) = Point3_<uchar>(color[0],color[1],color[2]);  
					}
				}			
			}

			if(strcmp(input,"BLANK_BINARY")==0)
			{
			cvtColor(cont,cont,CV_BGR2GRAY);
			cont.convertTo(cont,CV_8UC1,255,0);
			}
			pool->storeImage(cont,output);
			return;
	}	

	/// Draw contours with other sizes and bigger thickness.
	if(random_color){
		for( size_t i = 0; i< contours.size(); i++ )
		{	  
		color = this->getRandomColor(); 
		drawContours( cont, *objects, i, color, thickness-1, 8 );
		}
	}
	else
	{
	 //	void drawContours(Mat& image, const vector<vector<Point> >& contours, int contourIdx, const Scalar& color, int thickness=1, int lineType=8, const vector<Vec4i>& hierarchy=vector<Vec4i>(), int maxLevel=INT_MAX, Point offset=Point())
	 drawContours( cont, *objects, -1, color, thickness, 8 );
	}


	pool->storeImage(cont,output);
	
	return;
}




};
#endif // _FILE_PROCESSING_

