#ifndef _SEGMENTATION_PROCESSING_
#define _SEGMENTATION_PROCESSING_



#include <math.h>
#include <queue>
#include <set>
#include <vector>
#include <iostream>
#include "PModule.h"
#include <opencv2/opencv.hpp>
#include "MType.h"
#include "MAllTypes.h"
#include "WatershedSegmenter.h"
#include "utils.h"


using namespace cv;
using namespace std;


enum SFLAGS
{
	MERGE_FLAGS 
}; 
enum MERGE
{
	FAST,
	FUSION,
	FIRST_FOUND
};


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
		
		eMap["FAST"]=MERGE::FAST+SFLAGS::MERGE_FLAGS ;
		eMap["FUSION"]=MERGE::FUSION+SFLAGS::MERGE_FLAGS ;
		eMap["FIRST_FOUND"]=MERGE::FIRST_FOUND+SFLAGS::MERGE_FLAGS;

	};
	~SegmentationProcessing(void){};



	virtual void executeAction(string current_action,vector<MType *> params,unsigned int pid)
	{
	
		std::map<std::string, Function >::iterator x =  tFunc.find(current_action);
		if (x != tFunc.end())
		{
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
*  VALUES :Intensity or distance
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
 *	 (The minimum that we can tolerate). 
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
/*
Given two sets, one of big objects and other of the same objects fragmented.
The fragmented objects are imprinted to an image. Then, I create a list of sets.
Each set represents one big object. If for example, in the image, the big object 4
has 200 intersections, I save only the unique numbers from the fragmented objects, 
like 2,3,4,5,7.


*/
int __splitbyFragmentation(const char* reference, const char* big_objects, const char* frag_objects, const char* save_objects,double coefficient)
{

	ut::Trace tr = ut::Trace("Split by Fragmentation :",__FILE__); 

	 vloP *_Bobjvec = pool->getlObj(big_objects);
	 vloP *_Fobjvec = pool->getlObj(frag_objects);	 

	 Mat *ref=pool->getImage(reference);
	 Mat refF,refB;
	 refF = Mat::zeros(ref->rows,ref->cols,CV_32FC1);
	 vector<std::set<int>> Obslist;
	 vloP total;
	 Point ps;
	 bool good_behavior=true;
	 int count = 0;
	 // Imprint to an image fragmented objects
	 for(vloP::iterator it = _Fobjvec->begin(); it!=_Fobjvec->end(); ++it)
	{
		count++;
		for(loP::iterator itn = (*it).begin(); itn!=(*it).end(); ++itn)
		{
			ps = (*itn);
			refF.at<float>(ps) = count;
		}
		
	 }
	int val;
	// Create a set for each big object and save possible intersections
	set<int> myset;	
	bool intersect = false;
	for(vloP::iterator it2 = _Bobjvec->begin(); it2!=_Bobjvec->end(); ++it2)
	{
		intersect=false;	
		for(loP::iterator itn2 = it2->begin(); itn2!=it2->end(); ++itn2)
		{
			val = (int)(refF.at<float>(*itn2));
			if(val!=0.0)
			{
			 // I save in my list if I have found an intersection
			 myset.insert(val);
			 intersect=true;
			}
		}
		if(!intersect) total.push_back(*it2);
		Obslist.push_back(myset);
		myset.clear();
	 } 
	// Now classify and create new set:
	double pcoef;
	std::set<int>::iterator set_it;
	for(unsigned int i=0;i<Obslist.size();i++)
	{
		
		pcoef = ((*_Bobjvec)[i].size()*1.0)/coefficient;  // Example 200/40 = 5
		// cout << pcoef <<endl;
		if( Obslist[i].size()>(int)pcoef)  // I have 20 objects then 20 > 5, high fragmentation.
		{
			for( auto it=Obslist[i].begin(); it!=Obslist[i].end(); ++it)
			{
			  total.push_back((*_Fobjvec)[(*it)-1]); // I save all fragmented objects
			}
		}
		else
		{
			total.push_back((*_Bobjvec)[i]);
		}

	} 
	// Create mask for Big objects
	 refB = Mat::zeros(ref->rows,ref->cols,CV_8UC1);
	 for(vloP::iterator it = _Bobjvec->begin(); it!=_Bobjvec->end(); ++it)
	 {
		for(loP::iterator itn = (*it).begin(); itn!=(*it).end(); ++itn)
		{	
			ps = (*itn);
			refB.at<uchar>(ps) = 255;
		}	
	 }
	
	 for(vloP::iterator it =total.begin(); it!=total.end(); ++it)
	 {
		loP::iterator itn = it->begin();
		while(itn!=it->end())
		{
			ps = (*itn);
			if(refB.at<uchar>(ps)!=0)  
			{
			  refB.at<uchar>(ps)=0;
			  ++itn;
			}
			else  // then I have a point out of place or repeated--> I remove it
			{
			if(good_behavior)
				{
				tr.message("WARNING:Point destroyed! Small objects intersecting two or more big objects!!");
				good_behavior=false;
				}
			//	tr.message("x:",ps.x);
			//	tr.message("y:",ps.y);
				itn=it->erase(itn);
			}
		}	
	 }
	 



	tr.message("Total objects from BIG:",_Bobjvec->size());
	tr.message("Total objects from FRAGMENTED:",_Fobjvec->size());
	tr.message("After splitting:",total.size());
	if(good_behavior) tr.message("Perfect fitting between Big and Small!!! :) ");
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
	 string mask = (dynamic_cast<MStringType*>(parValues[2]))->getValue();
	 string soperation = (dynamic_cast<MStringType*>(parValues[3]))->getValue(); // operation type
	 string output	= (dynamic_cast<MStringType*>(parValues[4]))->getValue();	// Name of vector of Objects
	 string reference = (dynamic_cast<MStringType*>(parValues[5]))->getValue();

	int operation = eMap[soperation];

	 this->combnames(mask,pid,mask);
	 this->combnames(reference,pid,reference);
	 this->combnames(load_objects_1	,pid,load_objects_1);
	 this->combnames(load_objects_2,pid,load_objects_2);
	 this->combnames(output,pid,output);

	 __merge(load_objects_1.c_str(),load_objects_2.c_str(),output.c_str(),reference.c_str(),operation, mask.c_str());

}

int __merge(const char* load_objects_1,const char* load_objects_2, const char* output,const char *reference,int operation, const char *mask)
{

	ut::Trace tr = ut::Trace("Merge :",__FILE__); 	

	vloP* lobj_1= pool->getlObj(load_objects_1);
	vloP* lobj_2 = pool->getlObj(load_objects_2);
	Mat *ref=pool->getImage(reference);
	string _m(mask);
	
	tr.checkObject(load_objects_1,*lobj_1,*ref);
	tr.checkObject(load_objects_2,*lobj_2,*ref);

	Mat _mask(ref->rows,ref->cols, CV_8UC1, Scalar(0));
	if(_m.substr(_m.size()-2).compare("NO")!=0)
	{
		Mat *tmask = pool->getImage(mask);
		tmask->copyTo(_mask);
		tr.printMatrixInfo("MASK:",_mask);
	}
	vloP  lobj_3;
	operation = operation-SFLAGS::MERGE_FLAGS;
	// Merging two objects can be done in three different ways.
	// First is a fast merging, only recommended if you know that the objects don´t have overlapping regions
	tr.message("Total objects from vloP 1 : ", lobj_1->size());
	tr.message("Total objects from vloP 2 : ", lobj_2->size());
	
	if(operation==MERGE::FAST){

		lobj_3.insert(lobj_3.begin(),lobj_1->begin(),lobj_1->end());
		lobj_3.insert(lobj_3.begin(),lobj_2->begin(),lobj_2->end());
		pool->storelObj(lobj_3,output);
		tr.message("After merging vloP 3 : ", lobj_3.size());
		return 0;
	}
	// The other possibility is making a fussion of objects.
		// The best way of merge two different groups of objects is to write them to an image.
		// We first throw the object to an image

	if(lobj_1->size()==0)
	{
		pool->storelObj(*lobj_2,output);
		tr.message("After merging vloP 3 : ", lobj_2->size());
		return 0;
	}
	if(lobj_2->size()==0)
	{
		pool->storelObj(*lobj_1,output);
		tr.message("After merging vloP 3 : ", lobj_1->size());
		return 0;
	}
	// Initialize blank image to 8 bits for binarization
		Mat test(ref->rows,ref->cols, CV_32FC1, Scalar(0));
		int count = 0.0;
		for(auto myIterator =lobj_1->begin();  myIterator != lobj_1->end(); myIterator++)
		{
			count++;
			for(auto myIt2 = myIterator->begin(); myIt2!= myIterator->end();myIt2++)
			{
				test.at<float>(*myIt2)= count;
			}
			
		}
		

	if(operation==MERGE::FUSION)
	{
	// When the second object will be writing, if it is found to intersect with one of the previous 
	// objects, then all pixels are renumbered
	bool intersect =false;	
	std::set<float> mysetint;
	float oldcount=0.0;
	int value=0;
	loP tempLop;
	
		for(auto myIterator =lobj_2->begin();  myIterator != lobj_2->end(); myIterator++)
		{
			count++;
			for(auto myIt2 = myIterator->begin(); myIt2!= myIterator->end();myIt2++)
			{

				oldcount=test.at<float>(*myIt2);
				if(test.at<float>(*myIt2)!=0.0)  // intersection found!!!
				{			
				  mysetint.insert(oldcount);
				//  cout << oldcount << "," << endl;
				  intersect = true;
				}
				else
				{
				test.at<float>(*myIt2)=count;
				}
			}
		
// INTERSECTION:
			if(intersect) // then I need to rewrite all my touched objects.
			{
			 // Then find the minimum value of all the intersected
             set<float>::iterator it;
			 it = mysetint.begin();
			 int mymin=floor(*it);
			
			 // now assign the value to my object again, with the value of the intersection
			for(auto myIt2 = myIterator->begin(); myIt2!= myIterator->end();myIt2++)
			{
				test.at<float>(*myIt2)=mymin;
			} 
			count--;
			if(mymin > lobj_1->size())
			{
				 tr.message("WARNING: The second set of your objects intersect with itself (repeated pixels for the same object)!!!!");
				 tr.message("Object intersected:",mymin);
				 intersect=false;
				 continue;
			}
			// also assign everything connected
			it++;
			for(  ; it!= mysetint.end();it++)
			{
					value= floor((*it));
					tempLop= (*lobj_1)[value-1];
					for(auto myIt2 = tempLop.begin(); myIt2!= tempLop.end();myIt2++)
					{
					test.at<float>(*myIt2)=mymin;
					}  // and update my counter
			}
			mysetint.clear();
			}
			intersect = false; // reset the intersection possibility
		}
	}


	if(operation==MERGE::FIRST_FOUND)  // In this case, when intersection is found, the object in the second part is removed.
	{
		bool intersect =false;

		for(auto myIterator =lobj_2->begin();  myIterator != lobj_2->end(); myIterator++)
		{
			for(auto myIt2 = myIterator->begin(); myIt2!= myIterator->end();myIt2++)
			{
				if(test.at<float>(*myIt2)!=0)  // intersection found!!!
				{			
					intersect = true;
				}
			}
			if(!intersect) // then I am  assigning
			{
			    count++;
				for(auto myIt2 = myIterator->begin(); myIt2!= myIterator->end();myIt2++)
				{	
					 test.at<float>(*myIt2)=count;
				}			
			}
			
			intersect = false; // reset the intersection possibility
		}
	}
// And last but not least, we need to move points from the image to the new container

		int temp;

		for(unsigned int i = 0; i < count; i++) lobj_3.push_back(loP());

		for(unsigned int i = 0; i < test.rows; i++)
		{
			for(unsigned int j = 0; j < test.cols; j++){
			 temp = floor(test.at<float>(i,j));
			 if(_mask.at<uchar>(i,j)==0)
			 {
			    if(temp!=0)
				{
					lobj_3[temp-1].push_back(Point(j,i));
				}
			 }
		}
		}
		 auto iter = lobj_3.begin();
		 while (iter != lobj_3.end())
		 {
			 if (iter->size()==0)
			 {
              iter =lobj_3.erase(iter);
			 }
			else
			{
			 ++iter;
			 }
		}


		tr.message("After merging vloP 3 : ", lobj_3.size());
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

struct Pixel_compare { 
 inline bool operator() (const Pixel& a, const Pixel& b) const 
 { return  a.distance > b.distance; }
} px;

//  typedef PriorityQueue<Pixel> PixelQueue;
// Boost and STL definitions for priority queue
// 
typedef  std::priority_queue<Pixel, std::vector<Pixel>, Pixel_compare> PixelQueue;

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
static void contaminate(vloP &o_out,Mat &lab_copy,int i, int j,unsigned int &last_tag,int limit);
};



#endif // _SEGMENTATION_PROCESSING_


