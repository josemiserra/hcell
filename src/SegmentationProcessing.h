#ifndef _SEGMENTATION_PROCESSING_
#define _SEGMENTATION_PROCESSING_



#include <math.h>
#include <queue>
#include <set>
#include <vector>
#include <iostream>
// #include <boost/heap/priority_queue.hpp>
#include "PModule.h"
#include <opencv2/opencv.hpp>
#include "MType.h"
#include "MAllTypes.h"
#include "WatershedSegmenter.h"
#include "utils.h"
#include "PriorityQueue.h"

using namespace cv;
using namespace std;



class SegmentationProcessing :
	public PModule
{ 

public:
	 typedef  void(SegmentationProcessing::*Function)(vector<MType *>,unsigned int pid); // function pointer type
	 static map<string, Function > tFunc;
	 static map<string, int> eMap;

	SegmentationProcessing(){

		///------------------------------------------------/////
	    tFunc["WATERSHED"] = &SegmentationProcessing::_watershed;
	    tFunc["PROPAGATE"] = &SegmentationProcessing::_propagate;
		tFunc["LABEL"]=&SegmentationProcessing::_label;
		tFunc["SPLIT BY WATERSHED"]=&SegmentationProcessing::_splitbywatershed;
		tFunc["SPLIT BY OTSU"]=&SegmentationProcessing::_splitbyOtsu;
		tFunc["SPLIT BY FRAGMENTATION"]=&SegmentationProcessing::_splitbyFragmentation;
		tFunc["MERGE OBJECTS"]=&SegmentationProcessing::_merge;
		

	};
	~SegmentationProcessing(void){};



	virtual void executeAction(string current_action,vector<MType *> params,unsigned int pid)
	{
	
		std::map<std::string, Function >::iterator x =  tFunc.find(current_action);
	if (x != tFunc.end()) {
		(*this.*(x->second))(params,pid);
    }
	return;
	
	}

/****************************************************************************
 * Watershed
 * ----------------------------
 *  
 *
 ***************************************************************************/

void _watershed(std::vector<MType *> parValues, unsigned int pid)
{

	
	 bool ashow =  (dynamic_cast<MBoolType*>(parValues[0]))->getValue(); // showImage (ASHOW)
	 string bginput	= (dynamic_cast<MStringType*>(parValues[1]))->getValue();  // B
	 string fginput	= (dynamic_cast<MStringType*>(parValues[2]))->getValue();  // G
	 string input	= (dynamic_cast<MStringType*>(parValues[3]))->getValue();  // R
	 string output	= (dynamic_cast<MStringType*>(parValues[4]))->getValue();	//output	(OUTPUT)
	 const char* wName	= (dynamic_cast<MStringType*>(parValues[5]))->getValue(); // windowName
			
	 this->combnames(bginput,pid,bginput);
	 this->combnames(fginput,pid,fginput);
	 		
	 this->combnames(input,pid,input);
	 this->combnames(output,pid,output);


	 __watershed(input.c_str(),output.c_str(),fginput.c_str(),bginput.c_str());

	 
  if(ashow){
		 display(output,wName);
	  }
	  
}

int __watershed(const char*input,const char* output,const char* fg_im,const char* bg_im){
	
	ut::Trace tr = ut::Trace("watershed",__FILE__);
	
	Mat *image = pool->getImage(input);
	Mat *fg = pool->getImage(fg_im);
	Mat *bg = pool->getImage(bg_im);

    Mat f1, b1, im1;
	image->copyTo(im1);
    
	double minVal, maxVal;
    minMaxLoc(im1, &minVal, &maxVal); 
    im1.convertTo(im1,CV_8UC1,255.0/(maxVal - minVal), -minVal * 255.0/(maxVal - minVal));
	cvtColor(im1,im1,CV_GRAY2RGB);

    // display(im1,"original image");

	(*fg).convertTo(f1,CV_8U,255, 0);
	
    minMaxLoc(*bg, &minVal, &maxVal); 
    bg->convertTo(b1,CV_8UC1,128.0/(maxVal - minVal), -minVal * 128.0/(maxVal - minVal));

    // (*bg).convertTo(b1,CV_8U,128.0,0);
	// display(b1,"bg1");
	//  display(*bg,"bg");

	 cv::Mat markers(f1.size(),CV_16UC1,cv::Scalar(0));	 
	 add(f1,b1,markers);
//	 tr.printMatrixInfo("markers",markers);
	// display(markers,"markers");
	 
	 WatershedSegmenter segmenter;
     segmenter.setMarkers(markers);
//	tr.printMatrixInfo("im1",im1);
    cv::Mat result = segmenter.process(im1);

	pool->storeImage(result,output);

	return 0;
}


/****************************************************************************
 * Propagation
 * ----------------------------
 *  
 *
 ***************************************************************************/
void _propagate(std::vector<MType *> parValues, unsigned int pid)
{
 
	 string input	= (dynamic_cast<MStringType*>(parValues[0]))->getValue();  // initial image to segment

	 double lambda	= (dynamic_cast<MDoubleType*>(parValues[1]))->getValue();  // lamda value
	 string mask	= (dynamic_cast<MStringType*>(parValues[2]))->getValue();  // binary mask

	 string output	= (dynamic_cast<MStringType*>(parValues[3]))->getValue();	//output	(objects)
	 string seeds	= (dynamic_cast<MStringType*>(parValues[4]))->getValue();  // seeds
	
	 this->combnames(input,pid,input);
	 this->combnames(output,pid,output);
	 this->combnames(mask,pid,mask);
	 this->combnames(seeds,pid,seeds);


	 __propagate(seeds.c_str(), input.c_str(), output.c_str(), mask.c_str(), lambda);

	 return;
}

int __propagate(const char* seeds, const char* input,const char*output, const char* mask, double lambda);

/****************************************************************************
 * Labeling
 * ----------------------------
 *  Given a binary or tagged image, stores each object as a vector of Points
 *
 ***************************************************************************/
void _label(std::vector<MType *> parValues, unsigned int pid)
{

	 string input	= (dynamic_cast<MStringType*>(parValues[0]))->getValue();  // Image, binary or with seeds
	 string output	= (dynamic_cast<MStringType*>(parValues[1]))->getValue();	// Name of vector of Objects
	
	 this->combnames(input,pid,input);
	 this->combnames(output,pid,output);

	 __label(input.c_str(), output.c_str());

}
int __label(const char* input, const char* output);


/****************************************************************************
 *  SPLIT BY WATERSHED
 *  Special watershed application for splitting clumped nuclei
 * ----------------------------
 *  Given a binary or tagged image, stores each object as a vector of Points
 *
 ***************************************************************************/
void _splitbywatershed(std::vector<MType *> parValues, unsigned int pid)
{

	int ext	= (dynamic_cast<MIntType*>(parValues[0]))->getValue();	 
	string input	= (dynamic_cast<MStringType*>(parValues[1]))->getValue();  // Image, binary
	const char* method = (dynamic_cast<MStringType*>(parValues[2]))->getValue();
	string reference	= (dynamic_cast<MStringType*>(parValues[3]))->getValue();	// Reference in intensity values.
	string output	= (dynamic_cast<MStringType*>(parValues[4]))->getValue();	// Save objects -  Name of vector of Objects
	double tol = (dynamic_cast<MDoubleType*>(parValues[5]))->getValue();	
	 
	this->combnames(input,pid,input);
	this->combnames(output,pid,output);
	this->combnames(reference,pid,reference);

	  __splitbywatershed(input.c_str(),reference.c_str(), output.c_str(),ext,tol,method);

}

/*****************************************************************************************************
*  SplitNuclei
*  Given a binary image (8U) uses a watershed transform to segment nuclei.
* 
*******************************************************************************************************/
int __splitbywatershed(const char* input,const char* reference, const char* output,int ext,double tol,const char* method){
	 
 
	 ut::Trace tr = ut::Trace("split by watershed",__FILE__); 
	
	 WatershedSegmenter wat;
	 vloP objects_out;
	 Mat*  bin_im = pool->getImage(input);
	 Mat*  ref = pool->getImage(reference);
	 Mat res,ref2;

	 // distanceTransform(const Mat& src, Mat& dst, int distanceType, int maskSize)
	 distanceTransform(*bin_im, res,  CV_DIST_C, 3);

	 tr.printMatrixInfo("Reading:",*ref);
	//   Nuclei are split averaging normalized intensity values with the distance transform. This gives a
	//   better approach and less loss of information content.
	//   In second place, we negate the image -- filling wells
	switch(ref->type()){
		case(CV_8U): 
					ref->convertTo(ref2,CV_32FC1,255.0,0);
					break;
		case(CV_16U):
					ref->convertTo(ref2,CV_32FC1,65535.0,0);
					break;
		default:
					ref->copyTo(ref2);
			        break;
		
		}

	 MatIterator_<float> it1,it2,end;
	 it2 = ref2.begin<float>();
	 double minVal, maxVal;
	 minMaxLoc(ref2, &minVal, &maxVal); 

	 if(strcmp(method,"INTENSITY")==0)
	 {
		for( it1 = res.begin<float>(), end = res.end<float>(); it1 != end; ++it1,++it2)
		{
		float val= (*it1)*((*it2)/maxVal);
		(*it1) = val;
		}
	 }

	 wat.watershed_nuc(res,ref2,objects_out, tol, ext);

	 pool->storelObj(objects_out,output);


	 return 0;
}


/****************************************************************************
 * SPLIT BY OTSU
 * ----------------------------
 *  Given a list of objects, takes each object and apply Otsu thresholding. 
 *  Values below the thresholded are removed and the object is relabelled.
 * 
 *  1) calculate Otsu threshold T
 *  2) Remove pixels below T from the Object
 *	3) Create an image from the objects
 *  4) Relabel that image
 *
 *  
 ***************************************************************************/
void _splitbyOtsu(std::vector<MType *> parValues, unsigned int pid)
{

	
	 string load_objects	= (dynamic_cast<MStringType*>(parValues[0]))->getValue();	// Name of vector of Objects
	 string reference	= (dynamic_cast<MStringType*>(parValues[1]))->getValue();  // Image, binary or with seeds
	 string save_objects	= (dynamic_cast<MStringType*>(parValues[2]))->getValue();	// Name of vector of Objects

	 this->combnames(reference,pid,reference);
	 this->combnames(load_objects,pid,load_objects);
	 this->combnames(save_objects,pid,save_objects);

	 __splitbyOtsu(reference.c_str(),load_objects.c_str(),save_objects.c_str());

}
int __splitbyOtsu(const char* reference, const char* load_objects, const char* save_objects);



/****************************************************************************
 * SPLIT BY FRAGMENTATION
 * ----------------------------
 *  This is a way of merging fragmented objects with big objects.
 *  Split by otsu or watershed usually give us an original object fragmented in pieces.
 *  Knowing the degree of fragmentation around one determined size, can help us to 
 *  split between objects really fragmented and objects which should be together.
 *   For example, let�s say the common size of a nucleus is 40 pixels.
 *   An object of 200 pixels is then fragmented using watershed or otsu. 200/40 will give us 
 *  5 objects if they are clumpled nuclei. Well, it could happen that we get 4 or 7 instead of 5
 *  but we will never tolerate 1 or 2, or even 3. In this case, our coefficient would be simply 4/200 = 0.02.
 *	 
 *   Our algorithm:
 *		- Takes the coordinates of one big object
 *      - Searches the objects from fragmented objects which belong to it and counts them. 
 *		- If the result  number_of_small_objects / big_object.size() > Coefficient  
 *					 Big Object is saved
 *		- else 
					 Objects fragmented are saved
 ***************************************************************************/
void _splitbyFragmentation(std::vector<MType *> parValues, unsigned int pid)
{

	
	 string big_objects	= (dynamic_cast<MStringType*>(parValues[0]))->getValue();	// Name of vector of Objects
	 double coefficient = (dynamic_cast<MDoubleType*>(parValues[1]))->getValue();
	 string frag_objects	= (dynamic_cast<MStringType*>(parValues[2]))->getValue();	// Name of vector of Objects
	 string reference = (dynamic_cast<MStringType*>(parValues[3]))->getValue();
	 string save_objects	= (dynamic_cast<MStringType*>(parValues[4]))->getValue();	// Name of vector of Objects

	 this->combnames(big_objects,pid,big_objects);
	 this->combnames(frag_objects,pid,frag_objects);
	 this->combnames(save_objects,pid,save_objects);
	 this->combnames(reference,pid,reference);

	 __splitbyFragmentation(reference.c_str(),big_objects.c_str(),frag_objects.c_str(),save_objects.c_str(),coefficient);

}
int __splitbyFragmentation(const char* reference, const char* big_objects, const char* frag_objects, const char* save_objects,double coefficient)
{

	 vloP *_Bobjvec = pool->getlObj(big_objects);
	 vloP *_Fobjvec = pool->getlObj(frag_objects);	 

	 Mat *ref=pool->getImage(reference);
	 Mat refF,refB;
	 refF = Mat::zeros(ref->rows,ref->cols,CV_32FC1);
	 refB = Mat::zeros(ref->rows,ref->cols,CV_32FC1);
	 vector<std::set<int>> Obslist;
	 vloP total;
	 Point ps;
	 int count = 1.0;
	 for(vloP::iterator it = _Fobjvec->begin(); it!=_Fobjvec->end(); ++it)
	{
		for(loP::iterator itn = (*it).begin(); itn!=(*it).end(); ++itn)
		{
			ps = (*itn);
			refF.at<float>(ps) = count;
		}
		count++;
	 }
	int val;
	count = 0;
	for(vloP::iterator it2 = _Bobjvec->begin(); it2!=_Bobjvec->end(); ++it2)
	{
		 std::set<int> myset;
		 Obslist.push_back(myset);
		for(loP::iterator itn2 = it2->begin(); itn2!=it2->end(); ++itn2)
		{
			ps = (*itn2);
			val = (int) refF.at<float>(ps);
			if(val!=0.0)
			{
			// I save in my list
			Obslist[count].insert(val);
			}
			
		}
		count++;
	 } 
	// Now classify and create new set:
	double pcoef;
	for(unsigned int i=0;i<Obslist.size();i++)
	{
		pcoef = (Obslist[i].size()*1.0)/((*_Bobjvec)[i].size()*1.0);
		// cout << pcoef <<endl;
		if( pcoef>coefficient)
		{
			for( auto it=Obslist[i].begin(); it!=Obslist[i].end(); ++it)
			{
				total.push_back((*_Fobjvec)[*it]);
			}
		
		}
		else
		{
			total.push_back((*_Bobjvec)[i]);
		}

	}
	pool->storelObj(total,save_objects);
	return 0;
}



/****************************************************************************
 * MERGE OBJECTS
 * ----------------------------
 *  
 *
 ***************************************************************************/

void _merge(std::vector<MType *> parValues, unsigned int pid)
{

	
	 string load_objects_1	= (dynamic_cast<MStringType*>(parValues[0]))->getValue();	// Name of vector of Objects
	 string load_objects_2	= (dynamic_cast<MStringType*>(parValues[1]))->getValue();	// Name of vector of Objects
	 string output	= (dynamic_cast<MStringType*>(parValues[2]))->getValue();	// Name of vector of Objects

	 this->combnames(load_objects_1	,pid,load_objects_1);
	 this->combnames(load_objects_2,pid,load_objects_2);
	 this->combnames(output,pid,output);

	 __merge(load_objects_1.c_str(),load_objects_2.c_str(),output.c_str());

}

int __merge(const char* load_objects_1,const char* load_objects_2, const char* output)
{
	vloP* lobj_1= pool->getlObj(load_objects_1);
	vloP* lobj_2 = pool->getlObj(load_objects_2);
	vloP  lobj_3;
	lobj_3.insert(lobj_3.begin(),lobj_1->begin(),lobj_1->end());
	lobj_3.insert(lobj_3.begin(),lobj_2->begin(),lobj_2->end());
	// Sorting by coordinates?
	// 
	 pool->storelObj(lobj_3,output);
	return 0;

}


/****************************************************************************
 * Extra functions and definitions
 * ----------------------------
 *  
 *
 ***************************************************************************/

/****************************************************************************
 * Pixel structure needed for propagation algorithm
 * ------------------------------------------------
 *  
 *
 ***************************************************************************/
struct Pixel {
  double distance;
  unsigned int i, j;
  int label;
  Pixel(): 
    distance(0), i(0), j(0), label(0) {};
  Pixel (double ds,unsigned int ini,unsigned int inj,int l): 
    distance(ds), i(ini), j(inj), label(l) {};
	bool operator>(const Pixel &rhs) const { return distance > rhs.distance; };
	bool operator<(const Pixel &rhs) const { return distance < rhs.distance; };
};

// For using standard library or boost, the next functor can be used instead
/*
struct Pixel_compare { 
 inline bool operator() (const Pixel& a, const Pixel& b) const 
 { return  a.distance > b.distance; }
} px;
*/
   typedef PriorityQueue<Pixel> PixelQueue;
// Boost and STL definitions for priority queue
// 
// typedef boost::heap::priority_queue<Pixel,boost::heap::compare<Pixel_compare>> PixelQueue;

private:
inline void push_neighbors_on_queue(PixelQueue &pq,
						double dist,
                        Mat &image,
                        unsigned int i,unsigned int j,
                        unsigned int m,unsigned int n,
                        double lambda, int label,
                        int *labels_out,
						Mat &mask);

static void contaminate(vloP &o_out,Mat &lab_copy,int i, int j,unsigned int &last_tag);

};



#endif // _SEGMENTATION_PROCESSING_


