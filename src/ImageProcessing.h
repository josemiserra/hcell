#ifndef _IMAGE_PROCESSING_
#define _IMAGE_PROCESSING_


#include <math.h>
#include "PModule.h"
#include <opencv2/opencv.hpp>
#include "MType.h"
#include "utils.h"
#include "proctools.h"
#include <limits>
#include "BinaryProcessor.h"

using namespace cv;
using namespace std;
using namespace ut;

class ImageProcessing :
	public PModule
{ 

public:
	 typedef  void(ImageProcessing::*Function)(vector<MType *>,unsigned int pid); // function pointer type
	 static map<string, Function > tFunc;
	 static map<string, int> eMap;
	 static proctools tools;

	 enum FLAGS{ THRESHFLAGS=0, ADAPT_THRESHFLAGS=50, BRUSHFLAGS= 100, MORPHFLAGS=200, CONTOURFLAGS=300, 
			CONTOURFLAGS_2=350, SCALAR_FLAGS = 450, MAT_FLAGS = 500, MTS_FLAGS = 550, OTHERFLAGS = 600 };

	enum THRESH{ THRESH_16=15,THRESH_16_INV, BERNSEN };
	
	enum SHAPE{ GAUSSIAN=25, DIAMOND=26 };

	enum OPS{ LAPLACIAN, SOBEL, SCHARR, CANNY, BOX, MEDIAN, DESPECKLE};

	enum SCALAR_OPS{ SUM, DIFF, DIV, MULT, POWER, INV, TRANSPOSE};

	enum MAT_OPS{ MOR, MXOR, MAND, MSUM, MDIFF, MDIV, MMULT, MMULT_P, EYE, ZEROS, ONES};

	enum MTS_OPS{ MTSSUM, MTSMEAN, MTSSD, MTSTRACE};


	ImageProcessing(){

		///------------------------------------------------/////
	    tFunc["THRESHOLD"] = &ImageProcessing::_threshold;
		tFunc["ERODE"] = &ImageProcessing::_erode;
		tFunc["DILATE"] = &ImageProcessing::_dilate;
		tFunc["MORPHOLOGICAL OPERATION"] = &ImageProcessing::_morphOp;
		tFunc["BOUNDARY EXTRACTION"] = &ImageProcessing::_boundary_extraction;
		tFunc["FILLHULL"] = &ImageProcessing::_fillHull;
		tFunc["MAKE BRUSH"] = &ImageProcessing::_makeBrush;
		tFunc["FIND CONTOURS"]=&ImageProcessing::_contours;
		tFunc["SOBEL"]=&ImageProcessing::_sobel;
		tFunc["CANNY"]=&ImageProcessing::_canny;
		tFunc["LAPLACIAN"]=&ImageProcessing::_laplace;
		tFunc["GAUSSIAN BLUR"]=&ImageProcessing::_gblur;
		tFunc["BLUR"]=&ImageProcessing::_blur;
		tFunc["CREATE KERNEL"]=&ImageProcessing::_createkernel;
		tFunc["FILTER2D"]=&ImageProcessing::_filter2D;
		tFunc["ADAPTIVE THRESHOLD"]=&ImageProcessing::_adaptivethreshold;
		tFunc["ADAPTATIVE THRESHOLD"]=&ImageProcessing::_adaptivethreshold; // TO REMOVE in future versions!!!
		tFunc["FLOODFILL"]=&ImageProcessing::_floodfill;
		tFunc["SCALAR OPERATION"]=&ImageProcessing::_scalarOp;
		tFunc["MATRIX OPERATION"]=&ImageProcessing::_matOp;
		tFunc["MATRIX TO SCALAR OPERATION"]=&ImageProcessing::_mtsOp;
		tFunc["GBLOB"] = &ImageProcessing::_gblob;
		// -----Add here your new function---


		///---- FLAGS MAP----///
		/**THRESHOLDING**/
		eMap["THRESH_BINARY"]=THRESH_BINARY+FLAGS::THRESHFLAGS;
		eMap["BINARY"]=THRESH_BINARY+FLAGS::THRESHFLAGS;
		eMap["THRESH_TRUNC"]=THRESH_TRUNC+FLAGS::THRESHFLAGS;
		eMap["THRESH_BINARY_INV"]=THRESH_BINARY_INV+FLAGS::THRESHFLAGS;
		eMap["BINARY_INV"]=THRESH_BINARY_INV+FLAGS::THRESHFLAGS;
		eMap["THRESH_TOZERO"]=THRESH_TOZERO+FLAGS::THRESHFLAGS;
		eMap["THRESH_TOZERO_INV"]=THRESH_TOZERO_INV+FLAGS::THRESHFLAGS;
		eMap["THRESH_OTSU"]=THRESH_OTSU+FLAGS::THRESHFLAGS;
		eMap["THRESH_16"]=THRESH_16+FLAGS::THRESHFLAGS;
		eMap["THRESH_16_INV"]=THRESH_16_INV+FLAGS::THRESHFLAGS;
		eMap["BINARY_16"]=THRESH_16+FLAGS::THRESHFLAGS;
		eMap["BERNSEN"]= BERNSEN+FLAGS::THRESHFLAGS;
		/***MAKE BRUSH***/
		eMap["BOX_SHAPE"]=MORPH_RECT+FLAGS::BRUSHFLAGS;
		eMap["CROSS_SHAPE"]=MORPH_CROSS+FLAGS::BRUSHFLAGS;
		eMap["ELLIPSE_SHAPE"]=MORPH_ELLIPSE+FLAGS::BRUSHFLAGS; //EQUIVALENT TO DISC
		eMap["GAUSSIAN_SHAPE"]=GAUSSIAN + FLAGS::BRUSHFLAGS;
		eMap["DIAMOND_SHAPE"]=DIAMOND+FLAGS::BRUSHFLAGS;
//		eMap["MORPH LINE"]=MORPH_LINE+FLAGS::BRUSHFLAGS;
		/***MORPHOLOGICAL OPERATORS***/
		eMap["OPENING"]=MORPH_OPEN+FLAGS::MORPHFLAGS;
		eMap["CLOSING"]=MORPH_CLOSE+FLAGS::MORPHFLAGS;
		eMap["GRADIENT"]=MORPH_GRADIENT+FLAGS::MORPHFLAGS;
		eMap["TOP HAT"]=MORPH_TOPHAT+FLAGS::MORPHFLAGS;
		eMap["BLACK HAT"]=MORPH_BLACKHAT+FLAGS::MORPHFLAGS;
		/***FIND CONTOURS****/
		eMap["EXTERNAL"]=CV_RETR_EXTERNAL+FLAGS::CONTOURFLAGS;
		eMap["LIST"]=CV_RETR_LIST+FLAGS::CONTOURFLAGS;
		eMap["CONNECTED COMPONENTS"]=CV_RETR_CCOMP+FLAGS::CONTOURFLAGS;
		eMap["TREE"]=CV_RETR_TREE+FLAGS::CONTOURFLAGS;
		
		eMap["NONE"]=CV_CHAIN_APPROX_NONE +FLAGS::CONTOURFLAGS_2;
		eMap["SIMPLE"]=CV_CHAIN_APPROX_SIMPLE+FLAGS::CONTOURFLAGS_2;
		eMap["TEH CHIN 1"]=CV_CHAIN_APPROX_TC89_L1+FLAGS::CONTOURFLAGS_2;
		eMap["TEH CHIN 2"]=CV_CHAIN_APPROX_TC89_KCOS+FLAGS::CONTOURFLAGS_2;
		
	
		/**ADAPTATIVE THRESHOLDING**/
		eMap["MEAN"]=ADAPTIVE_THRESH_MEAN_C +FLAGS::ADAPT_THRESHFLAGS;
		eMap["GAUSSIAN"]=ADAPTIVE_THRESH_GAUSSIAN_C+FLAGS::ADAPT_THRESHFLAGS;

		/**OPERATORS**/
		eMap["SUM"] = SCALAR_FLAGS + SUM;
		eMap["DIFF"] = SCALAR_FLAGS + DIFF;
		eMap["DIV"] = SCALAR_FLAGS + DIV;
		eMap["MULT"] = SCALAR_FLAGS + MULT;
		eMap["POWER"] = SCALAR_FLAGS + POWER;
		eMap["INV"] = SCALAR_FLAGS + INV;
		eMap["TRANSPOSE"] = SCALAR_FLAGS + TRANSPOSE;

	    eMap["MOR"] = MAT_FLAGS + MOR;
		eMap["MXOR"] = MAT_FLAGS + MXOR;
		eMap["MAND"] = MAT_FLAGS + MAND;
		eMap["MSUM"] = MAT_FLAGS + MSUM;
		eMap["MDIFF"] = MAT_FLAGS + MDIFF;
		eMap["MDIV"] =  MAT_FLAGS + MDIV;
		eMap["MMULT"] = MAT_FLAGS + MMULT;
		eMap["MMULT_P"] = MAT_FLAGS + MMULT_P;
		
		eMap["MTSSUM"] = MTS_FLAGS + MTSSUM;
		eMap["MTSMEAN"] = MTS_FLAGS + MTSMEAN;
		eMap["MTSSD"] = MTS_FLAGS + MTSSD;
		eMap["MTSTRACE"] = MTS_FLAGS + MTSTRACE;

		eMap["SOBEL"]=OPS::SOBEL;
		eMap["SCHARR"]=OPS::SCHARR;
		eMap["BOX"]= OPS::BOX;
		eMap["MEDIAN"]= OPS::MEDIAN;
		eMap["DESPECKLE"]= OPS::DESPECKLE;

		eMap["EYE"] = MAT_OPS::EYE;
		eMap["ONES"] = MAT_OPS::ONES;
		eMap["ZEROS"] = MAT_OPS::ZEROS;

		eMap["DEFAULT_COLOR_SEED"] = FLAGS::OTHERFLAGS;
	};
	~ImageProcessing(void){};
	

	virtual void executeAction(string current_action,vector<MType *> params,unsigned int pid)
	{
	
		std::map<std::string, Function >::iterator x =  tFunc.find(current_action);
	if (x != tFunc.end()) {
		(*this.*(x->second))(params,pid);
    }
	return;
	
	}

/****************************************************************************
 * THRESHOLD
 * ----------------------------
 *  Binary threshold to an image
 *
 ***************************************************************************/

void _threshold(std::vector<MType *> parValues, unsigned int pid)
{
	
	 
	 bool a0 =  (dynamic_cast<MBoolType*>(parValues[0]))->getValue(); // showImage (ASHOW)
	 int ttype;
	

	 string input	= (dynamic_cast<MIdentifierType*>(parValues[1]))->getValue(pid); // imageName
     double max_thresh	= (dynamic_cast<MDoubleType*>(parValues[2]))->getValue(pid);  // maximum value for threshold
     MIdentifierType* out = dynamic_cast<MIdentifierType*>(parValues[3]);
	 string output	= out->getValue(pid); //output
	 double thresh	= (dynamic_cast<MDoubleType*>(parValues[4]))->getValue(pid);  //  value for threshold
	 string thresh_type = (dynamic_cast<MStringType*>(parValues[5]))->getValue(); // type of threshold
	 const char* wName	= (dynamic_cast<MStringType*>(parValues[6]))->getValue(); // windowName
	 ttype = eMap[thresh_type];

	 if((max_thresh!=255 ))
	 {
	  __threshold(input.c_str(),output.c_str(),thresh,ttype,max_thresh);
	 }
	 else
	 {
	 __threshold(input.c_str(),output.c_str(),thresh,ttype);
	 }
	  if(a0)
	  {
		 Mat *timg=pool->getImage(output.c_str());
		 this->display(*timg,wName);
	  }

	  // refresh outputs
	  out->refresh(pid);
	}
 

/* FROM OpenCV documentation
Parameters:	
src � input array (single-channel, 8-bit or 32-bit floating point).
dst � output array of the same size and type as src.
thresh � threshold value.
maxval � maximum value to use with the THRESH_BINARY and THRESH_BINARY_INV thresholding types.
type � thresholding type (see the details below).
*/

//***************************************************************************************************//////
int __threshold(const char* input,const char* output, double threshold_value, int threshold_type,double max_BINARY_value=255.0 ){  

	ut::Trace tr = ut::Trace("Threshold",__FILE__);

	Mat *img = pool->getImage(input);
	Mat thresholded;
	Mat image;
	img->copyTo(image);

	if(image.channels()>1)
	{
		cvtColor(image,image, CV_RGB2GRAY);
	}
	if((img->type() != CV_8U))
	{
		image.convertTo(image,CV_32FC1);	
	}
	else
	{
		if(max_BINARY_value <= 1)
		{ 
			max_BINARY_value= floor(max_BINARY_value*255);
			threshold_value = floor(threshold_value*255+0.5);
		}
	}

	tr.printMatrixInfo("Threshold Input image info:",image);
	
	if(threshold_type == cv::THRESH_OTSU)
	{
	    if(image.type()!=CV_8U)
		{
				 double min;double max;
				 minMaxIdx(image, &min, &max);
				 image.convertTo(image,CV_8UC1,255.0/(max - min), -min * 255.0/(max - min));
		}
		max_BINARY_value = 255;
		threshold_value = -1;
	}
	try
	{
		cv::threshold(image, thresholded, threshold_value,max_BINARY_value,threshold_type );
	}
	catch( cv::Exception& e )
	{
	    
		tr.message("Thresholding boundaries are not correct.\n E.g.:Range between  0-255 for 8 bit, 0-65535 for 16 bit images or between 0-1 if normalized.");
		#ifdef _MY_DEBUG_	
			tr.message("Error :",e.what());
		#endif
		 exit(-1921);
	}
	if((img->type() != CV_8U)) thresholded.convertTo(thresholded,CV_8UC1,255,0);
	pool->storeImage(thresholded,output);
	tr.printMatrixInfo("thresh",thresholded);
	return 0;
}

/****************************************************************************
 * ADAPTATIVE THRESHOLD
 * ----------------------------
 *  Binary threshold to an image
 *
 ***************************************************************************/

void _adaptivethreshold(std::vector<MType *> parValues, unsigned int pid){
	
	 ut::Trace tr=ut::Trace("Adaptative Thresholding",__FILE__);
	 int ttype, method;

	 bool a0 =  (dynamic_cast<MBoolType*>(parValues[0]))->getValue(); // showImage (ASHOW)
	 int blockSize	= (dynamic_cast<MIntType*>(parValues[1]))->getValue(pid);  //
	 double constant	= (dynamic_cast<MDoubleType*>(parValues[2]))->getValue(pid);  // 
	 string input	= (dynamic_cast<MIdentifierType*>(parValues[3]))->getValue(pid); // imageName
     string method_type = (dynamic_cast<MStringType*>(parValues[4]))->getValue(); // type of method
	 MIdentifierType* out = dynamic_cast<MIdentifierType*>(parValues[5]);
	 string output	= out->getValue(pid); //output
     
	
	 string thresh_type = (dynamic_cast<MStringType*>(parValues[6]))->getValue(); // type of threshold
	 
	 
	 const char* wName	= (dynamic_cast<MStringType*>(parValues[7]))->getValue(); // windowName
	 
	 ttype = eMap[thresh_type]-FLAGS::THRESHFLAGS;
	 method = eMap[method_type]-FLAGS::ADAPT_THRESHFLAGS;
		
	 __adaptivethreshold(input.c_str(),output.c_str(), method, ttype,blockSize,constant);

	  if(a0){
		 this->display(output,wName);
	  }

	  out->refresh(pid);
	}
 

int __adaptivethreshold(const char* input,const char* output,int &adaptativeMethod, int &threshold_type,int &blockSize,double &cval,double maxValue=255.0){  

	ut::Trace tr=ut::Trace("__Adaptative Thresholding",__FILE__);

	Mat *img = pool->getImage(input);

	Mat thresholded;
	Mat image;
	
	if(img->channels()>1)
	{
		cvtColor(*img,image, CV_RGB2GRAY);
	}
	else
	{
	img->copyTo(image);
	}
	// Now use different methods.
	// OPENCV default method is only 8 bits, which is less precisse but faster

	 tr.printMatrixInfo("Initial image info:",*img);
	 tr.printMatrixInfo("Thresholded image info:",image);

	 switch(threshold_type)
	 {
		case(THRESH_16):
			{				 
					  Adaptive myThresholder(blockSize,blockSize,cval);
					  myThresholder.run(image,thresholded);				 
					  break;
			}
       case(THRESH_16_INV):
			{				 
					  Adaptive myThresholder(blockSize,blockSize,cval);
					  myThresholder.run(image,thresholded);		
					  thresholded = cv::abs(Scalar::all(1) - thresholded);
					  break;
			}

		case(BERNSEN):
			{
			   		  Bernsen myThresholder(blockSize,cval);
					  myThresholder.run(image,thresholded);	
					  break;
			}
		default:
			{
			if(image.type()!=CV_8UC1) image.convertTo(image,CV_8UC1,255,0);
			// C++: void adaptiveThreshold(InputArray src, OutputArray dst, double maxValue, int adaptiveMethod, int thresholdType, int blockSize, double C)
			cv::adaptiveThreshold( image, thresholded, maxValue,adaptativeMethod, threshold_type, blockSize, cval);
			}
	 }
	thresholded.convertTo(thresholded,CV_8UC1,255,0);
	pool->storeImage(thresholded,output);
	return 0;
}

/*****MORPHOLOGICAL OPERATORS************************************************/
/****************************************************************************
 * ERODE
 * ----------------------------
 *  
 *
 ***************************************************************************/

void _erode(std::vector<MType *> parValues, unsigned int pid){

	 bool a0 =  (dynamic_cast<MBoolType*>(parValues[0]))->getValue(); // showImage (ASHOW)

	 const char* brush_name	= (dynamic_cast<MStringType*>(parValues[1]))->getValue(); // brushName
     string input	= (dynamic_cast<MIdentifierType*>(parValues[2]))->getValue(pid); // input image
	 int iter = (dynamic_cast<MIntType*>(parValues[3]))->getValue(pid); 
	 MIdentifierType* out = dynamic_cast<MIdentifierType*>(parValues[4]);
	 string output	= out->getValue(pid);  // output name
	 const char* wName	= (dynamic_cast<MStringType*>(parValues[5]))->getValue(); // windows name

	 this->__erode(input.c_str(),output.c_str(),brush_name,iter);

	  if(a0){
		 
		 this->display(output,wName);

	  }

	  out->refresh(pid);
};

int __erode(const char* input,const char* output,const char* strc_element,int iter){
	Mat *src, *element;
	Mat erosion;

	src=pool->getImage(input);
	element=pool->getImage(strc_element);
	
	cv::erode( *src, erosion, *element,Point(-1,-1),iter );
    pool->storeImage(erosion,output);

	return 0;
};

/****************************************************************************
 * DILATE
 * ----------------------------
 *  
 *
 ***************************************************************************/
void _dilate(std::vector<MType *> parValues, unsigned int pid)
{

	 bool a0 =  (dynamic_cast<MBoolType*>(parValues[0]))->getValue(); // showImage (ASHOW)
	 const char* brush_name	= (dynamic_cast<MStringType*>(parValues[1]))->getValue(); // brushName
     string input	= (dynamic_cast<MIdentifierType*>(parValues[2]))->getValue(pid); // input image
	 int iter = (dynamic_cast<MIntType*>(parValues[3]))->getValue(pid);  
	 MIdentifierType* out = dynamic_cast<MIdentifierType*>(parValues[4]);
	 string output	= out->getValue(pid);  // output name
	 string wName	= (dynamic_cast<MStringType*>(parValues[5]))->getValue(); // windows name

	 this->__dilate(input.c_str(),output.c_str(),brush_name,iter);

	  if(a0)
	  {

		 this->display(output,wName);
	  }

	  out->refresh(pid);
}


int __dilate(const char* input,const char* output, const char* strc_element,int iter){
	Mat *src, *element;
	
	src = pool->getImage(input);
	element = pool->getImage(strc_element);

	Mat dilation;
	cv::dilate( *src, dilation, *element,Point(-1,-1), iter);
	pool->storeImage(dilation,output);
	return 0;
};

/************MISCELLANEOUS MORPHOLOGICAL OPERATIONS***************************/
/****************************************************************************
 * OPENING, CLOSING, BLACK HAT, TOP HAT and GRADIENT
 * -------------------------------------------------------------------------
 *  
 *
 ***************************************************************************/

void _morphOp(std::vector<MType *> parValues, unsigned int pid)
{

     bool a0 =  (dynamic_cast<MBoolType*>(parValues[0]))->getValue(); // showImage (ASHOW)

	 string brush_name	= (dynamic_cast<MIdentifierType*>(parValues[1]))->getValueAsString(); // brushName
     string input	= (dynamic_cast<MIdentifierType*>(parValues[2]))->getValue(pid); // input image
	  int iter = (dynamic_cast<MIntType*>(parValues[3]))->getValue(pid); 
	 string soperation = (dynamic_cast<MStringType*>(parValues[4]))->getValue(); // operation type
	 MIdentifierType* out = dynamic_cast<MIdentifierType*>(parValues[5]);
	 string output	= out->getValue(pid);  // output name
	 
	 const char* wName	= (dynamic_cast<MStringType*>(parValues[6]))->getValue(); // windows name

	 int operation = eMap[soperation];
	 this->__morphOp(input.c_str(),output.c_str(),brush_name.c_str(),operation,iter);

	  if(a0){
		 this->display(output,wName);
	  }

	  out->refresh(pid);
}
/*
Opening: MORPH_OPEN : 2
Closing: MORPH_CLOSE: 3
Gradient: MORPH_GRADIENT: 4
Top Hat: MORPH_TOPHAT: 5
Black Hat: MORPH_BLACKHAT: 6
*/
int __morphOp(const char* input,const char* output, const char* strc_element,int operation,int iter){
	 ut::Trace tr = ut::Trace("Morph op",__FILE__);

	Mat *src, *element;

	operation = operation-FLAGS::MORPHFLAGS;
	src = pool->getImage(input);
	element = pool->getImage(strc_element);

	Mat op;
	tr.message("Using kernel:");
	tr.printMatrixInfo("Kernel",*element);
	tr.printMatrix(*element);
	cv::morphologyEx( *src, op, operation, *element,Point(-1,-1),iter);

	pool->storeImage(op,output);

	return 0;
};


/****************************************************************************
 * BOUNDARY EXTRACTION USING MORPHOLOGICAL OPERATIONS
 * ----------------------------
 *  
 *
 ***************************************************************************/


void _boundary_extraction(std::vector<MType *> parValues, unsigned int pid)
{
     
	bool a0 =  (dynamic_cast<MBoolType*>(parValues[0]))->getValue(); // showImage (ASHOW)
     const char* colour = (dynamic_cast<MStringType*>(parValues[1]))->getValue(); // colour
	 string input	= (dynamic_cast<MIdentifierType*>(parValues[2]))->getValue(pid); // input image
	 MIdentifierType* out = dynamic_cast<MIdentifierType*>(parValues[3]);
	 string output	= out->getValue(pid);  // output name
	 int thickness	= (dynamic_cast<MIntType*>(parValues[4]))->getValue(pid);  //line thickness of contour
	 const char* wName	= (dynamic_cast<MStringType*>(parValues[5]))->getValue(); // windows name

	 Scalar *color = utils::toColor(colour); 

	 this->__boundary_extraction(input.c_str(),output.c_str(),thickness,*color);

	  if(a0){
		 this->display(output,wName);
	  }
	  delete color;
	  out->refresh(pid);

};

void __boundary_extraction(const char* input, const char* output, int thickness,Scalar colour){

	
	Mat *src;
	src=pool->getImage(input);

	cv::Mat kernel = cv::Mat::ones(thickness, thickness, CV_8U);


	Mat erosion;
	cv::erode( *src, erosion, kernel );
	cv::Mat dst = *src - erosion;

	Mat mat;

	if((colour[0]!=255 && colour[1]!=255 && colour[2]!=255)||(colour[0]!=65536 && colour[1]!=65536 && colour[2]!=65536))
	{
	
		cvtColor(dst,mat,CV_GRAY2BGR);
		Mat3b mat1 = mat;
	
		mat1=255*mat1;
		for (Mat3b::iterator it = mat1.begin(); it != mat1.end(); it++)
		{
			if (*it == Vec3b(255, 255, 255))  // If it´s white, substitute by colour values
			{
			*it = Vec3b(colour[0], colour[1], colour[2]);
			}
		}
		mat=mat1;
	}
	else
	{
	  mat = dst;	
	}
	pool->storeImage(mat,output);

}



/****************************************************************************
 * FILL HULL
 * ----------------------------
 *  
 *
 ***************************************************************************/

void _fillHull(std::vector<MType *> parValues, unsigned int pid)
{

     bool show =  (dynamic_cast<MBoolType*>(parValues[0]))->getValue(); // showImage (ASHOW)

	 
     string input	= (dynamic_cast<MIdentifierType*>(parValues[1]))->getValue(pid); // input image
	 MIdentifierType* out = dynamic_cast<MIdentifierType*>(parValues[2]);
	 string output	= out->getValue(pid);  // output name	

	 const char* wName	= (dynamic_cast<MStringType*>(parValues[3]))->getValue(); // windows name

	 this->__fillHull(input.c_str(),output.c_str(),wName,show);

	 if(show){
		 this->displayTo8C1(output.c_str(),wName);

	  }

	 out->refresh(pid);

}

int __fillHull(const char* input, const char* output, const char* wName, bool &draw){

	Mat *src;
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;
	
	src=pool->getImage(input);
	Mat cont;
	(*src).convertTo(cont,CV_8U);
	  /// Find contours
     findContours(cont, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE, Point(0,0) );

   
  /// Draw contours and fill them
		Mat drawing = Mat::zeros( (*src).size(), CV_8UC3 );
		
		for( size_t i = 0; i< contours.size(); i++ )
		{	  
		 drawContours( drawing, contours, i, CV_RGB( 255, 255, 255 ), CV_FILLED, 8, hierarchy, 0, Point() );
		}
	   
		cvtColor(drawing,drawing, CV_RGB2GRAY);
		drawing.convertTo(drawing,CV_8UC1,255,0);
		pool->storeImage(drawing,output);
	
		return 0;
}


/****************************************************************************
 * MAKE BRUSH 
 * ----------------------------
 *  
 *
 ***************************************************************************/

void _makeBrush(std::vector<MType *> parValues, unsigned int pid)
{

	
	 MIdentifierType* out = dynamic_cast<MIdentifierType*>(parValues[4]);
	 string output	= out->getValueAsString(); //output
	 if(pool->containsImage(output.c_str()))
								return;  // Dont create twice the brush if already exists

	
	 bool a0 =  (dynamic_cast<MBoolType*>(parValues[0]))->getValue(); // showImage (ASHOW)
	 int brush_size_x	= (dynamic_cast<MIntType*>(parValues[1]))->getValue(pid);  // brush_size_x or brush_size
	 int brush_size_y	= (dynamic_cast<MIntType*>(parValues[2]))->getValue(pid);  // brush_size_y
	 string brush_type = (dynamic_cast<MStringType*>(parValues[3]))->getValue(); // brush shape
	 double sigma	= (dynamic_cast<MDoubleType*>(parValues[5]))->getValue(pid);  // sigma
	 const char* wName	= (dynamic_cast<MStringType*>(parValues[6]))->getValue(); // windowName
	
	
	 int btype;
	 btype = eMap[brush_type];
	 btype -= FLAGS::BRUSHFLAGS;

	 __makeBrush(btype,brush_size_x,brush_size_y,output.c_str(),(float)sigma);

	  if(a0){
	
		 this->display(output,wName,brush_size_x,brush_size_y);
	  }
	  out->refresh(pid);
}


/*
// C++: Mat getStructuringElement(int shape, Size ksize, Point anchor=Point(-1,-1))
Anchor point needed for cross but so far not implemented.
*/
int __makeBrush(int shape,int size_x,int size_y,const char* brushName,float sigma){
	
	ut::Trace tr = ut::Trace("makeBrush",__FILE__);

	Mat kernel;
	Size ksize = Size(size_x, size_y);
	
	switch(shape){
		case(GAUSSIAN):{
			  kernel=getStructuringElement(MORPH_RECT, ksize,Point(-1,-1));
			  kernelToGaussian(kernel,sigma);
		//	kernel=getGaussianKernel(size_x,sigma, CV_64F);		
			tr.printMatrixInfo("Gaussian Kernel:",kernel);
			break;
		}
		case(DIAMOND):{
			kernel=getStructuringElement(MORPH_RECT, ksize,Point(-1,-1));
			kernelToDiamond(kernel);			 
			break;
							 }
		default:{
			kernel=getStructuringElement(shape, ksize,Point(-1,-1));
			break;
				}
		}
	pool->storeImage(kernel,brushName);
	return 0;

}

/****************************************************************************
 * SOBEL, CANNY AND LAPLACE DERIVATIVE FILTERS
 * ----------------------------
 *  
 *
 ***************************************************************************/
/****************************************************************************
 * SOBEL
 * ----------------------------
 *  
 *
 ***************************************************************************/

void _sobel(std::vector<MType *> parValues, unsigned int pid)
{

	 bool a0 =  (dynamic_cast<MBoolType*>(parValues[0]))->getValue(); // showImage (ASHOW)

	 string input	= (dynamic_cast<MIdentifierType*>(parValues[1]))->getValue(pid); // input image
	 string operation	= (dynamic_cast<MStringType*>(parValues[2]))->getValue();  // output name
	 MIdentifierType* out = dynamic_cast<MIdentifierType*>(parValues[3]);
	 string output	= out->getValue(pid);  // output name
	 
	 int size	= (dynamic_cast<MIntType*>(parValues[4]))->getValue(pid);  // 1,3,5,7
	 const char* wName	= (dynamic_cast<MStringType*>(parValues[5]))->getValue(); // windows name
	 int dx	= (dynamic_cast<MIntType*>(parValues[6]))->getValue(pid);  // 
	 int dy	= (dynamic_cast<MIntType*>(parValues[7]))->getValue(pid);  // 

	 this->__sobel(input.c_str(),output.c_str(),operation,dx,dy,size);

	  if(a0){

		  Mat *timg = pool->getImage(output.c_str());
		  double minVal, maxVal;
		  minMaxLoc(*timg, &minVal, &maxVal); 
		  Mat st;
		  (*timg).convertTo(st,CV_8UC3,255.0/(maxVal - minVal), -minVal * 255.0/(maxVal - minVal));
		  this->display(st,wName);
	  }
	  out->refresh(pid);
};

int __sobel(const char* input,const char* output,string operation, int dx=1,int dy=1, int size=3){
		
	Mat *src_gray;	
	src_gray = pool->getImage(input);
	Mat grad,grad_x,grad_y;
	Mat abs_grad_x, abs_grad_y;
	if(operation.compare("SCHARR")==0) 
	{
	   /// Gradient X
	   Scharr( *src_gray, grad_x, CV_16S, 1, 0, 1, 0, BORDER_DEFAULT );
       convertScaleAbs( grad_x, abs_grad_x);
		/// Gradient Y
	    Scharr( *src_gray, grad_y, CV_16S, 0, 1, 1, 0, BORDER_DEFAULT );
		convertScaleAbs( grad_y, abs_grad_y );
		 /// Total Gradient (approximate)
        addWeighted( abs_grad_x, 0.5, abs_grad_y, 0.5, 0, grad );
	}
	else{		
		//	 void Sobel(InputArray src, OutputArray dst, int ddepth, int dx, int dy, int ksize=3, double scale=1, double delta=0, int borderType=BORDER_DEFAULT )
			   /// Gradient X
	   Sobel( *src_gray, grad_x, CV_16S, dx, 0, 1, size, 0, BORDER_DEFAULT );
       convertScaleAbs( grad_x, abs_grad_x);
		/// Gradient Y
	    Sobel( *src_gray, grad_y, CV_16S, 0, dy, 1, size, 0, BORDER_DEFAULT );
		convertScaleAbs( grad_y, abs_grad_y );
		 /// Total Gradient (approximate)
        addWeighted( abs_grad_x, 0.5, abs_grad_y, 0.5, 0, grad );
		}
	pool->storeImage(grad,output);
	return 0;
};

/****************************************************************************
 * CANNY
 * ----------------------------
 *  See manual for further instructions
 *
 ***************************************************************************/
void _canny(std::vector<MType *> parValues, unsigned int pid)
{

	 bool a0 =  (dynamic_cast<MBoolType*>(parValues[0]))->getValue(); // showImage (ASHOW)
	 double ht	= (dynamic_cast<MDoubleType*>(parValues[1]))->getValue(pid);  // high threshold
	 string input	= (dynamic_cast<MIdentifierType*>(parValues[2]))->getValue(pid); // input image
	 int ksize = (dynamic_cast<MIntType*>(parValues[3]))->getValue(pid);  // kernel size
	 bool l2gradient =  (dynamic_cast<MBoolType*>(parValues[4]))->getValue(); //l2 gradient	
	 double lt	= (dynamic_cast<MDoubleType*>(parValues[5]))->getValue(pid);  // low threshold
	 MIdentifierType* out = dynamic_cast<MIdentifierType*>(parValues[6]);
	 string output	= out->getValue(pid);  // output name
	
	 const char* wName	= (dynamic_cast<MStringType*>(parValues[7]))->getValue(); // windows name

	 this->__canny(input.c_str(),output.c_str(),lt,ht,ksize,l2gradient);

	if(a0)
	{
		 display(output,wName);
	}
    out->refresh(pid);	  
}

int __canny(const char* input,const char* output, double lt,double ht, int apsize,bool L2g = false){
	

	Mat *src_gray;
	Mat detected_edges;
	src_gray = pool->getImage(input);

	double minVal, maxVal;
	minMaxLoc(*src_gray, &minVal, &maxVal); 
	(*src_gray).copyTo(detected_edges);

	detected_edges.convertTo(detected_edges,CV_8UC1,255.0/(maxVal - minVal), -minVal * 255.0/(maxVal - minVal));
	 /// Canny detector
	// void Canny(InputArray image, OutputArray edges, double threshold1, double threshold2, int apertureSize=3, bool L2gradient=false )
     Canny(detected_edges, detected_edges, lt, ht,apsize,L2g);

	pool->storeImage(detected_edges,output);
	return 0;
}
/****************************************************************************
 * LAPLACE
 * ----------------------------
 *  
 *
 ***************************************************************************/
void _laplace(std::vector<MType *> parValues, unsigned int pid)
{
	
	 bool a0 =  (dynamic_cast<MBoolType*>(parValues[0]))->getValue(); // showImage (ASHOW)
	 string input	= (dynamic_cast<MIdentifierType*>(parValues[1]))->getValue(pid); // input image
	 int ksize = (dynamic_cast<MIntType*>(parValues[2]))->getValue(pid);  // kernel size
	 MIdentifierType* out = dynamic_cast<MIdentifierType*>(parValues[3]);
	 string output	= out->getValue(pid);  // output name
	 const char* wName	= (dynamic_cast<MStringType*>(parValues[4]))->getValue(); // windows name

	 this->__laplace(input.c_str(),output.c_str(),ksize);

	if(a0)
	{
	 this->displayTo8C1(output.c_str(),wName);
	}
	out->refresh(pid);

}

int __laplace(const char* input,const char* output,int kernel_size=1){

	Mat *src;
	src=pool->getImage(input);

	/// Apply Laplace function
	Mat dst,abs_dst;
    // void Laplacian(InputArray src, OutputArray dst, int ddepth, int ksize=1, double scale=1, double delta=0, int borderType=BORDER_DEFAULT )	
	Laplacian( *src, dst, -1, kernel_size);
	convertScaleAbs( dst, abs_dst );
	pool->storeImage(abs_dst,output);							 			
	return 0;
}

/****************************************************************************
 * GAUSSIAN BLUR
 * ----------------------------
 *  
 *
 ***************************************************************************/

void _gblur(std::vector<MType *> parValues, unsigned int pid)
{

	 bool a0 =  (dynamic_cast<MBoolType*>(parValues[0]))->getValue(); // showImage (ASHOW)

	 int height = (dynamic_cast<MIntType*>(parValues[1]))->getValue(pid); // height
     
	 string input	= (dynamic_cast<MIdentifierType*>(parValues[2]))->getValue(pid); // input image
	 MIdentifierType* out = dynamic_cast<MIdentifierType*>(parValues[3]);
	 string output	= out->getValue(pid);  // output name
	 
	 double sigma_x = (dynamic_cast<MDoubleType*>(parValues[4]))->getValue(pid); // sx
	 double sigma_y = (dynamic_cast<MDoubleType*>(parValues[5]))->getValue(pid); // sy
	
	 int width = (dynamic_cast<MIntType*>(parValues[6]))->getValue(pid); // width
	 string wName	= (dynamic_cast<MStringType*>(parValues[7]))->getValue(); // windows name


	 __gblur(input.c_str(),output.c_str(),width,height,sigma_x,sigma_y);

	  if(a0){ 
		  display(output,wName);
	  }

	  out->refresh(pid);
}

int __gblur(const char* input, const char* output, int width, int height, double sigma_x, double sigma_y){


	Mat *src=pool->getImage(input);
	Mat op;
	
	GaussianBlur( *src, op, Size(width,height), sigma_x, sigma_y, BORDER_DEFAULT );

	pool->storeImage(op,output);
	return 0;
}

/****************************************************************************
 *  BLUR
 * ----------------------------
 *  
 *
 ***************************************************************************/

void _blur(std::vector<MType *> parValues, unsigned int pid)
{

	 bool a0 =  (dynamic_cast<MBoolType*>(parValues[0]))->getValue(); // showImage (ASHOW)

	 int height = (dynamic_cast<MIntType*>(parValues[1]))->getValue(pid); // height
     
	string input	= (dynamic_cast<MIdentifierType*>(parValues[2]))->getValue(pid); // input image
	string operation = (dynamic_cast<MStringType*>(parValues[3]))->getValue(); // input type
	MIdentifierType* out = dynamic_cast<MIdentifierType*>(parValues[4]);
	string output	= out->getValue(pid);  // output name
	 
	 int width = (dynamic_cast<MIntType*>(parValues[5]))->getValue(pid); // width
	 string wName	= (dynamic_cast<MStringType*>(parValues[6]))->getValue(); // windows name

	 int type = eMap[operation];

	 __blur(input.c_str(),output.c_str(),width,height,type);

	  if(a0){ display(output,wName);
	  }

	  out->refresh(pid);
}

int __blur(const char* input, const char* output, int width, int height, int operation){

	
	Mat *src;
	src=pool->getImage(input);
	Mat op;


	switch(operation)
	{
		case(OPS::BOX):
			{
			cv::blur( *src, op, Size(width,height));
			break;
			}
		case(OPS::DESPECKLE):
			{
		    medianBlur(*src,op,3);
			break;
			}
		default:{ //MEDIAN blur
			// when ksize is 3 or 5, the image depth should be CV_8U, CV_16U, or CV_32F, for larger aperture sizes, it can only be CV_8U.
			Mat dst(*src);
			if(height>5 && src->depth()!=CV_8U)
			{ 
				dst.convertTo(dst,CV_8U,255,0);  
			}
			medianBlur(dst,op, height);
			break;
				}
		}

	pool->storeImage(op,output);
	return 0;
}



/****************************************************************************
 * FIND AND DRAW CONTOURS
 * ----------------------------
 *  
 *
 ***************************************************************************/
void _contours(std::vector<MType *> parValues, unsigned int pid){

	 string approximation	= (dynamic_cast<MStringType*>(parValues[0]))->getValue(); // Approximation type
     bool show =  (dynamic_cast<MBoolType*>(parValues[1]))->getValue(); // showImage (ASHOW)

	 const char* colour = (dynamic_cast<MStringType*>(parValues[2]))->getValue(); // colour
	 
     string input	= (dynamic_cast<MIdentifierType*>(parValues[3]))->getValue(pid); // input image
	 MIdentifierType* out = dynamic_cast<MIdentifierType*>(parValues[4]);
	 string output	= out->getValue(pid);  // output name
	 
	 string retrieval	= (dynamic_cast<MStringType*>(parValues[5]))->getValue();  // retrieval name
	 string sobj	= (dynamic_cast<MIdentifierType*>(parValues[6]))->getValue(pid);  //
	 
	 int thickness	= (dynamic_cast<MIntType*>(parValues[7]))->getValue(pid);  //line thickness of contour

	 const char* wName	= (dynamic_cast<MStringType*>(parValues[8]))->getValue(); // windows name

	 int opret = eMap[retrieval]-FLAGS::CONTOURFLAGS;
	 int app = eMap[approximation]-FLAGS::CONTOURFLAGS_2;

	 Scalar *color;
	 if(strcmp(colour,"RANDOM")==0)  color = new Scalar( 255, 0, 0 );
	 else color = utils::toColor(colour); 
	 // colour to RGB function
	 
	 
	 this->__contours(input.c_str(),output.c_str(),sobj.c_str(),opret,app,*color,thickness,wName,show);

	 delete color;

	 out->refresh(pid);
}

/***

Parameters:	
image � Source, an 8-bit single-channel image. Non-zero pixels are treated as 1�s. Zero pixels remain 0�s, so the image is treated as binary . You can use compare() , inRange() , threshold() , adaptiveThreshold() , Canny() , and others to create a binary image out of a grayscale or color one. The function modifies the image while extracting the contours.
contours � Detected contours. Each contour is stored as a vector of points.
hierarchy � Optional output vector, containing information about the image topology. It has as many elements as the number of contours. For each i-th contour contours[i] , the elements hierarchy[i][0] , hiearchy[i][1] , hiearchy[i][2] , and hiearchy[i][3] are set to 0-based indices in contours of the next and previous contours at the same hierarchical level, the first child contour and the parent contour, respectively. If for the contour i there are no next, previous, parent, or nested contours, the corresponding elements of hierarchy[i] will be negative.
mode �
Contour retrieval mode (if you use Python see also a note below).

CV_RETR_EXTERNAL retrieves only the extreme outer contours. It sets hierarchy[i][2]=hierarchy[i][3]=-1 for all the contours.
CV_RETR_LIST retrieves all of the contours without establishing any hierarchical relationships.
CV_RETR_CCOMP retrieves all of the contours and organizes them into a two-level hierarchy. At the top level, there are external boundaries of the components. At the second level, there are boundaries of the holes. If there is another contour inside a hole of a connected component, it is still put at the top level.
CV_RETR_TREE retrieves all of the contours and reconstructs a full hierarchy of nested contours. This full hierarchy is built and shown in the OpenCV contours.c demo.
method �
Contour approximation method

CV_CHAIN_APPROX_NONE stores absolutely all the contour points. That is, any 2 subsequent points (x1,y1) and (x2,y2) of the contour will be either horizontal, vertical or diagonal neighbors, that is, max(abs(x1-x2),abs(y2-y1))==1.
CV_CHAIN_APPROX_SIMPLE compresses horizontal, vertical, and diagonal segments and leaves only their end points. For example, an up-right rectangular contour is encoded with 4 points.
CV_CHAIN_APPROX_TC89_L1,CV_CHAIN_APPROX_TC89_KCOS applies one of the flavors of the Teh-Chin chain approximation algorithm. See [TehChin89] for details.
offset � Optional offset by which every contour point is shifted. This is useful if the contours are extracted from the image ROI and then they should be analyzed in the whole image context.

**/
void __contours(const char* input, const char* output, const char* sobj,int &opret,int &app,Scalar color,int &thickness, const char* &wName, bool &draw){
  
	Mat *src;
	vloP contours;
	vector<Vec4i> hierarchy;
	bool random_color;
	
	src=pool->getImage(input);
	Mat cont;
	(*src).convertTo(cont,CV_8UC1);
	  /// Find contours
     findContours(cont, contours, hierarchy, opret, app, Point(0,0) );

   
  /// Draw contours

	    random_color=(color.isReal());
		Mat drawing = Mat::zeros( (*src).size(), CV_8UC3 );
		
		if(random_color){
			for( size_t i = 0; i< contours.size(); i++ )
			{	  
			  color = utils::getRandomColor();
			  //	void drawContours(Mat& image, const vector<vector<Point> >& contours, int contourIdx, const Scalar& color, int thickness=1, int lineType=8, const vector<Vec4i>& hierarchy=vector<Vec4i>(), int maxLevel=INT_MAX, Point offset=Point())
			 drawContours( drawing, contours, i, color, thickness, 8, hierarchy, 0, Point() );
			}
		}
		else
		{
			for( size_t i = 0; i< contours.size(); i++ )
			{	  
			  //	void drawContours(Mat& image, const vector<vector<Point> >& contours, int contourIdx, const Scalar& color, int thickness=1, int lineType=8, const vector<Vec4i>& hierarchy=vector<Vec4i>(), int maxLevel=INT_MAX, Point offset=Point())
			 drawContours( drawing, contours, i, color, thickness, 8, hierarchy, 0, Point() );
			}
//		  drawContours( drawing, contours, -100, color, thickness, 8, hierarchy, 0, Point() );  // Doesn�t work properly
	
		}

		if(draw)
		{	   
			this->display(drawing,wName);
		}
	   
		pool->storeImage(drawing,output);
		pool->storelObj(contours,sobj);

}

/************FILTERS***********************************************************/
/****************************************************************************
 * CREATE KERNEL
 * -------------------------------------------------------------------------
 *  
 *
 ***************************************************************************/
void _createkernel(std::vector<MType *> parValues, unsigned int pid)
{
	  
	
	 MIdentifierType* out = dynamic_cast<MIdentifierType*>(parValues[4]);
	 string output = out->getValueAsString(); // output
	 if(pool->containsImage(output.c_str()))
								return;

	 //  //
	 double factor;
     factor = (dynamic_cast<MDoubleType*>(parValues[1]))->getValue(pid); 

	 int kernelsize = (dynamic_cast<MIntType*>(parValues[2]))->getValue(pid);  // kernel size
	 string matrix = (dynamic_cast<MStringType*>(parValues[3]))->getValue(); // input matrix

	

	int elem=kernelsize*kernelsize;
	float *ker;
	ker = (float *) malloc(sizeof(float)*elem);
	
	 if(matrix.compare("ONES")==0)
	 {
	   for(int i=0;i<elem;i++) ker[i]=1.0;
	 }
	 if(matrix.compare("ZEROS")==0)
	 {
	   for(int i=0;i<elem;i++) ker[i]=0.0;
	 }

	 if(matrix.compare("EYE")==0)
	 {
		 Mat eye = Mat::eye( kernelsize, kernelsize, CV_32F );
		 for(int i=0;i<kernelsize;i++)
			  for(int j=0;j<kernelsize;j++) {
				   ker[i*kernelsize+j]=eye.at<float>(j,i);
			  }
	 }
	 if(matrix.find_first_of(";")!=string::npos)
	 {
		int i=0;
		char *pch;
		char *next;
		pch = strtok((char*)matrix.c_str(),",;");
		double onenumber;
		while (pch != NULL)
		{
			onenumber =  strtod(pch,&next);
			ker[i++]  = onenumber;
			pch = strtok (NULL, ",;");
		}
	 }
	 else
	 {
		 for(int i=0;i<elem;i++) ker[i]=atof(matrix.c_str());
	 }


	 __createkernel(output.c_str(),kernelsize,ker,factor);

	 delete [] ker;

	 out->refresh(pid);
}

int __createkernel(const char* output,int &kernelsize,float *ker,double &div)
{

	Mat kernel = Mat::ones( kernelsize, kernelsize, CV_32F );
	// uchar* p = kernel.data;
	// int max = kernelsize*kernelsize;
	
	int i=0;
	 MatIterator_<float> it, end;
     for( it = kernel.begin<float>(), end = kernel.end<float>(),i=0; it != end; ++it,i++)
	 {
		 (*it) = ker[i]/div;
	 }
	pool->storeImage(kernel,output);

	return 0;
}




/****************************************************************************
 * FILTER 2D
 * -------------------------------------------------------------------------
 *  
 *
 ***************************************************************************/

void _filter2D(std::vector<MType *> parValues, unsigned int pid)
{
	 bool a0 =  (dynamic_cast<MBoolType*>(parValues[0]))->getValue(); // showImage (ASHOW)
	string input = (dynamic_cast<MIdentifierType*>(parValues[1]))->getValue(pid);
	const char* kernelName = (dynamic_cast<MStringType*>(parValues[2]))->getValue();
	
	MIdentifierType* out = dynamic_cast<MIdentifierType*>(parValues[3]);
	string output	= out->getValue(pid); //output
	const char* wName	= (dynamic_cast<MStringType*>(parValues[4]))->getValue(); // windowName
	
	__filter2D(input.c_str(),output.c_str(),kernelName);

	  if(a0)
	  {
		  this->displayTo8C1(output.c_str(),wName);
	  }

	  out->refresh(pid);
}

int __filter2D(const char* input, const char* output, const char *kernelName)
{
	ut::Trace tr = ut::Trace("filter2D",__FILE__);

	Mat *src, *krn;
	src = pool->getImage(input);
	krn = pool->getImage(kernelName);
	Mat dst;
	Mat krn2;
	// tr.printMatrix(*krn);
	/// Apply filter
	// tr.printMatrixInfo("function",*src);  

	 Point anchor(krn->cols - krn->cols/2 - 1, krn->rows - krn->rows/2 - 1);
     int borderMode = BORDER_CONSTANT;
	 flip(*krn,krn2,0);
	 filter2D(*src, dst, -1, krn2, anchor, 0, borderMode);

	// filter2D(*src, dst, -1 , *krn,Point();
	 tr.printMatrixInfo("Kernel",krn2);
	 tr.printMatrixInfo("function",dst);
	pool->storeImage(dst,output);

	return 0;

}


/****************************************************************************
 * FLOODFILL
 * -------------------------------------------------------------------------
 *  
 *
 ***************************************************************************/
void _floodfill(std::vector<MType *> parValues, unsigned int pid)
{
	bool a0 =  (dynamic_cast<MBoolType*>(parValues[0]))->getValue(); // showImage (ASHOW)
	string colour = (dynamic_cast<MStringType*>(parValues[1]))->getValue();
	string input = (dynamic_cast<MIdentifierType*>(parValues[2]))->getValue(pid);
	double loDiff = (dynamic_cast<MDoubleType*>(parValues[3]))->getValue(pid); 	
	MIdentifierType* out = dynamic_cast<MIdentifierType*>(parValues[4]);
	string output	= out->getValue(pid); //output
	string seeds = (dynamic_cast<MIdentifierType*>(parValues[5]))->getValue(pid);
	double upDiff = (dynamic_cast<MDoubleType*>(parValues[6]))->getValue(pid); 
	const char* wName	= (dynamic_cast<MStringType*>(parValues[7]))->getValue(); // windowName

	__floodfill(input.c_str(),output.c_str(),seeds.c_str(),colour,loDiff,upDiff);
	 
	if(a0){ display(output,wName); }

	out->refresh(pid);

}

void __floodfill(const char*input,const char*output,const char*seeds,string colour, double loDiff, double upDiff)
{

	Mat *img = pool->getImage(input); 
	Mat filled;
	if(img->channels()<3) cvtColor(*img,filled,CV_GRAY2BGR);
	else img->copyTo(filled);

	Scalar *fillingColor;
	Scalar _loDiff=Scalar(loDiff); 
	Scalar _upDiff=Scalar(upDiff);

	bool def = false;
	bool random = false;
	if(colour.compare("DEFAULT_COLOR_SEED")==0)
	{ 
		def=true;
	}
	else
	{
		if(colour.compare("RANDOM")==0)
		{
			random=true;
		}
		else
		{ 
			fillingColor = utils::toColor(colour);
		}
	}
	vloP *listP = pool->getlObj(seeds);
	vloP::iterator myvloP;
	
	for(myvloP = listP->begin(); myvloP != listP->end(); myvloP++)
	{
		loP *seedPoints;
		seedPoints = &(*myvloP);
		loP::iterator itsp;
		Point seed = (*seedPoints)[seedPoints->size()/2];
		if(random)
		{ // pick color of filling from the same color of seed point
			Scalar a = utils::getRandomColor();
			fillingColor = &a;
		}		
		else
		{
				if(def)
				{
					
					int b= filled.at<cv::Vec3b>((*itsp))[0];
					int g= filled.at<cv::Vec3b>((*itsp))[1];
					int r= filled.at<cv::Vec3b>((*itsp))[2];
					fillingColor = new Scalar(b,g,r);
					
				}
			}
		// int floodFill(InputOutputArray image, Point seedPoint, Scalar newVal, Rect* rect=0, Scalar loDiff=Scalar(), Scalar upDiff=Scalar(), int flags=4 )
		   floodFill(filled,seed,*fillingColor,0,_loDiff,_upDiff);
	   }
	pool->storeImage(filled,output);
}




/****************************************************************************
 * Kernel generators
 * -------------------------------------------------------------------------
 *  Warning: Kernels are boxes and OpenCV doesn�t accept 0 (non-rect shapes)
 *
 ***************************************************************************/
void kernelToGaussian(Mat &ker,float sigma=0.3){
		ut::Trace tr = ut::Trace("Gaussian brush",__FILE__);
		ker.convertTo(ker,CV_32FC1);
		int size = ker.size().height;
		float temp_val=0.0;
		for(int j=0;j<size;j++)
			 for(int i=0;i<size;i++)
			{
				ker.at<float>(i,j)  = - floor(static_cast<float>((size-1)/2)+0.5)+i;
			} 
		// tr.printMatrix(ker);
		Mat tker;
		transpose(ker,tker);
		float sum = 0;
        Mat temp1,temp2,kertemp;
		cv::pow(ker,2,ker);
		cv::pow(tker,2,tker);

		for(int j=0;j<size;j++)
			for(int i=0;i<size;i++)
			{
				float elem=   ker.at<float>(j,i);
				float telem = tker.at<float>(j,i);
				temp_val= exp(-((elem+telem)/(2*sigma*sigma)));
				if(temp_val<=0) temp_val=0.0000000000000000001; 
				ker.at<float>(j,i)=temp_val;
				sum+=ker.at<float>(j,i);
			} 
	  ker /= sum; 
			 
		/*	 tr.printMatrixInfo("Kernel",ker);
			 tr.printMatrix(ker);  
			*/
	/*	Mat temp1,temp2,kertemp;
		temp1 = -(kertemp+tker);
		temp1 /= (2*sigma*sigma);
		exp(temp1,temp2);
		Scalar s = cv::sum(temp2);
		kertemp = temp2/s[0]; 
		ker = abs(kertemp);
		ker.convertTo(ker,CV_32FC1); */
	 // 
}
void kernelToDiamond(Mat &ker){
	ut::Trace tr = ut::Trace("Diamond brush",__FILE__);
		ker.convertTo(ker,CV_32FC1);
		int size = ker.size().height;
		float temp_val=0.0;
		for(int j=0;j<size;j++)
			 for(int i=0;i<size;i++)
			 {
				 ker.at<float>(i,j)  = - floor(static_cast<float>((size-1)/2)+0.5)+i;
			 } 
	    float mz = size/2.0;
		Mat tker;
		ker.copyTo(tker);
		for(int j=0;j<size;j++)
			 for(int i=0;i<size;i++)
			{
				 temp_val = abs(ker.at<float>(j,i))+ abs(tker.at<float>(i,j));
				 temp_val  = (mz-temp_val)/mz;
				 if(temp_val<=0) temp_val=0.0000000001; 
				 ker.at<float>(j,i)=temp_val;
			} 
	//    tr.printMatrix(ker);

}

/*****OPERATORS************************************************/
/****************************************************************************
 * SCALAR 
 * ----------------------------
 *  
 *
 ***************************************************************************/

void _scalarOp(std::vector<MType *> parValues, unsigned int pid){

	  bool a0 =  (dynamic_cast<MBoolType*>(parValues[0]))->getValue(); // showImage (ASHOW)
      
	  double factor = (dynamic_cast<MDoubleType*>(parValues[2]))->getValue(pid); 
      string input	= (dynamic_cast<MIdentifierType*>(parValues[2]))->getValueAsString(); // input image
	  int iter = (dynamic_cast<MIntType*>(parValues[3]))->getValue(pid); 
	  string soperation = (dynamic_cast<MStringType*>(parValues[4]))->getValue(); // operation type
	  MIdentifierType* out = dynamic_cast<MIdentifierType*>(parValues[5]);
	  string output	= out->getValueAsString();  // output name
	  const char* wName	= (dynamic_cast<MStringType*>(parValues[6]))->getValue(); // windows name

	  Mat *src = pool->getImage(input.c_str());

	  if(src==0)
	  {
		  input		= (dynamic_cast<MIdentifierType*>(parValues[2]))->getValue(pid);
		  output	= (dynamic_cast<MIdentifierType*>(parValues[5]))->getValue(pid); 
	  }
     int operation = eMap[soperation];
	 this->__scalarOp(input.c_str(),output.c_str(),factor,operation,iter);

	  if(a0){
		 this->display(output,wName);
	  }

	  out->refresh(pid);
};

/*

*/

int __scalarOp(const char* input,const char* output,double factor, int operation, int iter){
	 
	ut::Trace tr = ut::Trace("Scalar op",__FILE__);
	

	Mat *src;
	operation = operation-FLAGS::SCALAR_FLAGS;
	src = pool->getImage(input);
	
	Mat op;
	src->copyTo(op);
	
	int count=0;
	while(count<iter){
		
		switch(operation)
		{
			case(SUM): 
						op += factor;
						break;
			case(DIFF):
						absdiff(op,factor,op);
					   break;
			case(DIV):
						op /= factor;
						break;
			case(MULT):
					   op *=  factor;
					   break;
			case(POWER):{
						if(count==0) op.convertTo(op, CV_32F);
						if(src->type()==CV_16U) op/=65535;
						else op/=255;
						pow(op,factor,op);
					    double min;
						double max;
						minMaxIdx(op, &min, &max);
						if(src->type()==CV_8U) op.convertTo(op,CV_8UC1,255.0/(max - min), -min * 255.0/(max - min));
						if(src->type()==CV_16U) op.convertTo(op,CV_16UC1,65535.0/(max - min), -min * 65535.0/(max - min));				
						break;
						}
			 case(INV):{
					   if(op.type()==CV_8U)  op= 255 - op;
					   if(op.type()==CV_16U) op= 65535 - op;
					   if(op.type()==CV_32F) op=1 - op ;
					   break;
						}
            case(TRANSPOSE):{
				        op=op.t();
					   break;
						}
			default:
					cout<<"ERROR: Non existent operation";
					exit(-1946);
					break;
		}
		count++;
	}
	tr.message("Factor used in scalar operation: ",factor);
	tr.printMatrixInfo("Op Out",op);
	pool->storeImage(op,output);
	return 0;
};

/****************************************************************************
 * Matrix Operations
 * ----------------------------
 *  
 *
 ***************************************************************************/

void _matOp(std::vector<MType *> parValues, unsigned int pid){

	  bool a0 =  (dynamic_cast<MBoolType*>(parValues[0]))->getValue(); // showImage (ASHOW)
      
	  string input1	= (dynamic_cast<MIdentifierType*>(parValues[1]))->getValue(pid); // brushName
      string input2	= (dynamic_cast<MIdentifierType*>(parValues[2]))->getValue(pid); // input image
	  int iter = (dynamic_cast<MIntType*>(parValues[3]))->getValue(pid); 
	  string soperation = (dynamic_cast<MStringType*>(parValues[4]))->getValue(); // operation type
	  
	  MIdentifierType* out = dynamic_cast<MIdentifierType*>(parValues[5]);
	  string output	= out->getValue(pid);  // output name
	  const char* wName	= (dynamic_cast<MStringType*>(parValues[6]))->getValue(); // windows name

     int operation = eMap[soperation];
	 this->__matOp(input1.c_str(),input2.c_str(),output.c_str(),operation,iter);

	  if(a0)
	  {
		 this->display(output,wName);
	  }
	  out->refresh(pid);
};

/*

*/

int __matOp(const char* input1,const char* input2,const char* output, int operation, int iter){
	 ut::Trace tr = ut::Trace("Matrix op",__FILE__);

	Mat *src1, *src2; 
	operation = operation-FLAGS::MAT_FLAGS;
	src1 = pool->getImage(input1);
	src2 = pool->getImage(input2);
	Mat op1,op2;
	src1->copyTo(op1);
	src2->copyTo(op2);
	Mat op3;
	int count=0;
	while(count<iter){
		
		switch(operation)
		{
			case(MOR):{
					   bitwise_or(op1,op2,op3);
					   break;
						}
			case(MAND):{
						op3 = op1 & op2;
					   break;
						}
			case(MXOR):{
						bitwise_xor(op1,op2,op3);
					   break;
						}	
		    case(MSUM): 
						op3 = op1 + op2;
						break;
			case(MDIFF):
						absdiff(op1,op2,op3);
					   break;
			case(MDIV):  // Performs per-element division of two arrays.
						op1.convertTo(op1,CV_32F);
					    op2.convertTo(op2,CV_32F);
						divide(op1,op2,op3);
						break;
			case(MMULT):
						
					   op1.convertTo(op1,CV_32F);
					   op2.convertTo(op2,CV_32F);
					   multiply(op1,op2,op3);
					   break;	
			case(MMULT_P):
						// Calculates the per-element product of two arrays.
					   op1.convertTo(op1,CV_32F);
					   op2.convertTo(op2,CV_32F);
					   op3 = op1*op2;
					   break;	
			default:
					cout<<"ERROR: Non existent operation";
					exit(-1958);
					break;
		}
		count++;
		op3.copyTo(op1); // op1 is now the input1 
	}
	
	pool->storeImage(op3,output);
	return 0;
};


/****************************************************************************
 * Matrix to Scalar Operations
 * ----------------------------
 *  
 *
 ***************************************************************************/

void _mtsOp(std::vector<MType *> parValues, unsigned int pid){

    
	  string input = (dynamic_cast<MIdentifierType*>(parValues[0]))->getValueAsString(); // brushName
	  string soperation = (dynamic_cast<MStringType*>(parValues[1]))->getValue(); // operation type
	  MIdentifierType* out = dynamic_cast<MIdentifierType*>(parValues[2]);
	  string foutput	= out->getValue(pid);  // output factor
	  
	  // Kernels can have Scalar Operations, but kernels are static structures
	  // thread independent...
	  if(!pool->containsImage(input.c_str()))
	  {
		 input = (dynamic_cast<MIdentifierType*>(parValues[0]))->getValue(pid); 
	  }
	  
	  int operation = eMap[soperation];
	  this->__mtsOp(input.c_str(),foutput.c_str(),operation);
	  out->refresh(pid);
};

/*
Matrix to Scalar
*/

int __mtsOp(const char* input,const char* output, int operation){
	 ut::Trace tr = ut::Trace("Matrix to Scalar op",__FILE__);

	Mat *src1; 
	double factor;
	operation = operation-FLAGS::MTS_FLAGS;
	src1 = pool->getImage(input);
	Mat op1;
	src1->copyTo(op1);
	Scalar s;
	int count=0;
		switch(operation)
		{
			case(MTSSUM):{
					   s = sum(op1);
					   break;
						}
			case(MTSMEAN):{
						s = mean(op1);
					   break;
						}
			case(MTSTRACE):{
						s= cv::trace(op1);
					   break;
						}	
		    case(MTSSD):{ // diagonal of covariance matrix 
						Mat mean;
						Mat stddev;
						meanStdDev(op1, mean, stddev);
						s = cv::mean(stddev);
						break; }
			default:{
					cout<<"ERROR: Non existent operation";
					exit(-1946);
					break;}
		}
		factor = s[0];
		tr.message("Factor: ",factor);
		pool->storeFactor(factor,output);
		return 0;
};
/*

GBLOB granulometry transform.

*/
void _gblob(std::vector<MType *> parValues, unsigned int pid)
{
	double alpha = (dynamic_cast<MDoubleType*>(parValues[0]))->getValue(pid); 
	bool ashow   = (dynamic_cast<MBoolType*>(parValues[1]))->getValue(); 
	double beta  = (dynamic_cast<MDoubleType*>(parValues[2]))->getValue(pid);
    string input	= (dynamic_cast<MIdentifierType*>(parValues[3]))->getValue(pid); 
	MIdentifierType* out = dynamic_cast<MIdentifierType*>(parValues[4]);
	string output	= out->getValue(pid);	
	int sequence = (dynamic_cast<MIntType*>(parValues[5]))->getValue(pid); 
    size_t size = (dynamic_cast<MIntType*>(parValues[6]))->getValue(pid);
	const char* wName = (dynamic_cast<MStringType*>(parValues[7]))->getValue();

	  __gblob(input.c_str(),output.c_str(), size, sequence, alpha, beta);

	  if(ashow)
	  {
		 this->display(output,wName);
	  }

	  out->refresh(pid);

}

void __gblob(const char* input,const char* output, size_t size,int sequence, double alpha, double beta)
{
	 ut::Trace tr = ut::Trace("GBLOB",__FILE__);
	
	Mat *src; 
	src = pool->getImage(input);
     

	// sequence from -15 to 15, 49 times
	vector<double> xx;

	double by = (2.0*sequence)/((float)size-1.0);
	float init = (float) -sequence;
	while(init<=sequence)
	{
	  xx.push_back(init); 
	  init+=by;
	}
	// Create our matrix
	 Mat kernel = Mat::zeros( size, size, CV_64F );
	// and initialize it
	 MatIterator_<double> it;
	 vector<double>::iterator itxx;
	 

	double aux;
    for( size_t i = 0; i < size; i++ )
	{
		 aux = xx[i];
         for( size_t j = 0; j < size; j++ )
		 {
			 kernel.at<double>(i,j)=aux;
         }
    }
	
	Mat tker;
	transpose(kernel,tker);
	cv::pow(kernel,2,kernel);
	cv::pow(tker,2,tker);
	cv::add(kernel,tker,kernel);
	cv::sqrt(kernel,kernel);

    double x;
	double sum=0.0;

    for( size_t i = 0; i < size; i++ )
	{
         for( size_t j = 0; j < size; j++ )
		 {
			 x = kernel.at<double>(i,j);
			 kernel.at<double>(i,j)=tools.normal_pdf(x,0,alpha)-0.65*tools.normal_pdf(x,0,beta);
			 sum+= kernel.at<double>(i,j);
         }
    }
	kernel /=sum;
	Point anchor(kernel.cols - kernel.cols/2 - 1, kernel.rows - kernel.rows/2 - 1);
    int borderMode = BORDER_CONSTANT;
	flip(kernel,kernel,0);
	Mat final_image;
	filter2D(*src, final_image, -1,kernel, anchor, 0, borderMode);
	final_image/=2;

	pool->storeImage(final_image,output);
	return;
}

};


#endif  // _IMAGE_PROCESSING
