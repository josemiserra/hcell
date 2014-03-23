#ifndef _SEGMENTATION_PROCESSING_
#define _SEGMENTATION_PROCESSING_

#pragma warning( disable : 4482 )
#pragma warning( disable : 4244 )

#include <math.h>
#include <queue>
#include <set>
#include <vector>
#include <iostream>
#include "PModule.h"
#include <opencv2/opencv.hpp>
#include "MType.h"
#include "WatershedSegmenter.h"
#include "ImageProcessing.h"
#include "utils.h"
#include "proctools.h"


using namespace cv;
using namespace std;


enum SFLAGS
{
	MERGE_FLAGS,
	WATERSHED_FLAGS = 50
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
		tFunc["HOUGH TRANSFORM"]=&SegmentationProcessing::_hough;
		tFunc["RADIAL SEGMENTATION"]=&SegmentationProcessing::_radial;


		eMap["FAST"]=MERGE::FAST+SFLAGS::MERGE_FLAGS ;
		eMap["FUSION"]=MERGE::FUSION+SFLAGS::MERGE_FLAGS ;
		eMap["FIRST_FOUND"]=MERGE::FIRST_FOUND+SFLAGS::MERGE_FLAGS;
		
		eMap["INTENSITY"] = WATERSHED_FLAGS;
		eMap["DISTANCE"] = WATERSHED_FLAGS;

		eMap["NO_BACKGROUND"] = WATERSHED_FLAGS;
		
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
	 string bginput	= (dynamic_cast<MIdentifierType*>(parValues[1]))->getValue(pid);  // B
	 string fginput	= (dynamic_cast<MIdentifierType*>(parValues[2]))->getValue(pid);  // G
	 string input	= (dynamic_cast<MIdentifierType*>(parValues[3]))->getValue(pid);  // R
	 MIdentifierType* out = dynamic_cast<MIdentifierType*>(parValues[4]);
	 string output	= out->getValue(pid);	//output	(OUTPUT)
	 string save_objects = (dynamic_cast<MIdentifierType*>(parValues[5]))->getValue(pid); // 
	 const char* wName	= (dynamic_cast<MStringType*>(parValues[6]))->getValue(); // windowName
			
	 __watershed(input,output,save_objects,fginput,bginput);

	 
  if(ashow){
		 this->display(output,wName);
	  }
     out->refresh(pid);
}

int __watershed(string input,string output,string save_objects,string fg_im,string bg_im){
	
	ut::Trace tr = ut::Trace("watershed",__FILE__);
	
	Mat *image = pool->getImage(input);
	Mat *fg = pool->getImage(fg_im);
	Mat *bg = pool->getImage(bg_im);

    Mat f1, b1, im1;
    
	double minVal, maxVal;
    minMaxLoc(*image, &minVal, &maxVal); 
	// Conversion to 8 bit possibly needed
    if(image->depth()!=CV_8U) image->convertTo(im1,CV_8UC1,255.0/(maxVal - minVal), -minVal * 255.0/(maxVal - minVal));
	else image->copyTo(im1);
	cvtColor(im1,im1,CV_GRAY2RGB);

   // display(im1,"original image");
	 if(fg->depth()!=CV_8U) fg->convertTo(f1,CV_8U,255, 0);
	 else fg->copyTo(f1);

	minMaxLoc(*bg, &minVal, &maxVal); 
	bg->convertTo(b1,CV_32F,1.0/(maxVal - minVal), -minVal * 1.0/(maxVal - minVal)); 
    // Background is considered one more object...
	b1 = 1-b1;
	// tr.printMatrixInfo("bg",b1);
	// display(b1,"bg1");
	// display(*bg,"bg");
    Mat markers;
	b1.copyTo(markers);	 
	 // create markers image with connected components
	 vloP in_objects;
	 this->__label(f1,in_objects);
	 // Now we need to stamp the objects to the image
	float count = 2.0;
	for(vloP::iterator it = in_objects.begin(); it!=in_objects.end(); ++it,++count)
	{	       
		        for (loP::iterator obj = it->begin(); obj!=it->end(); ++obj)
				{
				   markers.at<float>(*obj) = count;
				} 			  
   }

	// tr.printMatrixInfo("markers",markers); display(markers,"markers");
	 WatershedSegmenter segmenter;
	
    segmenter.process(im1,markers);
	// remove background object
	markers.convertTo(markers,CV_32FC1);
	for( int i = 0; i < markers.rows; i++ )
					for( int j = 0; j < markers.cols; j++ )
						{	
							if(markers.at<float>(i,j)==1.0) markers.at<float>(i,j)=0.0;
							else
							if(markers.at<float>(i,j)==-1.0) markers.at<float>(i,j)=0.0;
							else 
							if(markers.at<float>(i,j)>1.0)	markers.at<float>(i,j)=  markers.at<float>(i,j)-1.0;
						}
	

	tr.printMatrixInfo("markers",markers);
	in_objects.clear();
	proctools::convertImageToVlop<float>(markers,in_objects);
	markers.convertTo(markers,CV_8U);
	
	threshold(markers,markers,1,255,0);

//	tr.printMatrixInfo("res",result);
//    display(result,"r");	
	pool->storeImage(markers,output);
	pool->storelObj(in_objects,save_objects.c_str());
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
 
	 string input	= (dynamic_cast<MIdentifierType*>(parValues[0]))->getValue(pid);  // initial image to segment
	 double lambda	= (dynamic_cast<MDoubleType*>(parValues[1]))->getValue(pid);  // lamda value
	 string mask	= (dynamic_cast<MIdentifierType*>(parValues[2]))->getValue(pid);  // binary mask
	 MIdentifierType* out = dynamic_cast<MIdentifierType*>(parValues[3]);
	 string output	= out->getValue(pid);	//output	(objects)
	 string seeds	= (dynamic_cast<MIdentifierType*>(parValues[4]))->getValue(pid);  // seeds
	
	 __propagate(seeds.c_str(), input.c_str(), output.c_str(), mask.c_str(), lambda);

	 out->refresh(pid);
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

	 string input	= (dynamic_cast<MIdentifierType*>(parValues[0]))->getValue(pid);  // Image, binary or with seeds
	 MIdentifierType* out = dynamic_cast<MIdentifierType*>(parValues[1]);
	 string output	= out->getValue(pid);	// Name of vector of Objects

	 __label(input.c_str(), output.c_str());

	 out->refresh(pid);
}
int __label(const char* input, const char* output);
int static __label(Mat &im,vloP &objects_out);

/****************************************************************************
 *  SPLIT BY WATERSHED
 *  Special watershed application for splitting clumped nuclei
 * ----------------------------
 *  Given a binary or tagged image, stores each object as a vector of Points
 *
 ***************************************************************************/
void _splitbywatershed(std::vector<MType *> parValues, unsigned int pid)
{

	int ext	= (dynamic_cast<MIntType*>(parValues[0]))->getValue(pid);	 
	string input	= (dynamic_cast<MIdentifierType*>(parValues[1]))->getValue(pid);  // Image, binary
	const char* method = (dynamic_cast<MStringType*>(parValues[2]))->getValue();
	string reference	= (dynamic_cast<MIdentifierType*>(parValues[3]))->getValue(pid);	// Reference in intensity values.
	MIdentifierType* out = dynamic_cast<MIdentifierType*>(parValues[4]);
	string output	= out->getValue(pid);	// Save objects -  Name of vector of Objects
	double tol = (dynamic_cast<MDoubleType*>(parValues[5]))->getValue(pid);	

	  __splitbywatershed(input.c_str(),reference.c_str(), output.c_str(),ext,tol,method);

	  out->refresh(pid);
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

	
	 string load_objects	= (dynamic_cast<MIdentifierType*>(parValues[0]))->getValue(pid);	// Name of vector of Objects
	 string reference	= (dynamic_cast<MIdentifierType*>(parValues[1]))->getValue(pid);  // Image, binary or with seeds
	 MIdentifierType* out = dynamic_cast<MIdentifierType*>(parValues[2]);
	 string save_objects	= out->getValue(pid);	// Name of vector of Objects

	 __splitbyOtsu(reference.c_str(),load_objects.c_str(),save_objects.c_str());

	 out->refresh(pid);
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

	 string big_objects	= (dynamic_cast<MIdentifierType*>(parValues[0]))->getValue(pid);	// Name of vector of Objects
	 double coefficient = (dynamic_cast<MDoubleType*>(parValues[1]))->getValue(pid);
	 string frag_objects	= (dynamic_cast<MIdentifierType*>(parValues[2]))->getValue(pid);	// Name of vector of Objects
	 string reference = (dynamic_cast<MIdentifierType*>(parValues[3]))->getValue(pid);
	  MIdentifierType* out = dynamic_cast<MIdentifierType*>(parValues[4]);
	 string save_objects	= out->getValue(pid);	// Name of vector of Objects

	 __splitbyFragmentation(reference.c_str(),big_objects.c_str(),frag_objects.c_str(),save_objects.c_str(),coefficient);

	 out->refresh(pid);
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

	
	 string load_objects_1	= (dynamic_cast<MIdentifierType*>(parValues[0]))->getValue(pid);	// Name of vector of Objects
	 string load_objects_2	= (dynamic_cast<MIdentifierType*>(parValues[1]))->getValue(pid);	// Name of vector of Objects
	 string mask = (dynamic_cast<MIdentifierType*>(parValues[2]))->getValueAsString();
	 if(mask.compare("NULL")!=0) mask = (dynamic_cast<MIdentifierType*>(parValues[2]))->getValue(pid);
	 string soperation = (dynamic_cast<MStringType*>(parValues[3]))->getValue(); // operation type
	 MIdentifierType* out = dynamic_cast<MIdentifierType*>(parValues[4]);
	 string output	= out->getValue(pid);	// Name of vector of Objects
	 string reference = (dynamic_cast<MIdentifierType*>(parValues[5]))->getValue(pid);

	int operation = eMap[soperation];

	 __merge(load_objects_1.c_str(),load_objects_2.c_str(),output.c_str(),reference.c_str(),operation, mask);

	 out->refresh(pid);
}

int __merge(const char* load_objects_1,const char* load_objects_2, const char* output,const char *reference,int operation, string mask)
{

	ut::Trace tr = ut::Trace("Merge :",__FILE__); 	

	vloP* lobj_1= pool->getlObj(load_objects_1);
	vloP* lobj_2 = pool->getlObj(load_objects_2);
	Mat *ref=pool->getImage(reference);

	
	tr.checkObject(load_objects_1,*lobj_1,*ref);
	tr.checkObject(load_objects_2,*lobj_2,*ref);
	Mat _mask(ref->rows,ref->cols, CV_8UC1, Scalar(0));
	if(mask.compare("NULL")!=0)
	{
		Mat *tmask = pool->getImage(mask);
		tmask->copyTo(_mask);
		if(_mask.depth()!=CV_8U) _mask.convertTo(_mask,CV_8UC1);
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
		double newcount=0.0;
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
void _radial(std::vector<MType *> parValues, unsigned int pid)
{
}

void __radial(){};


void _hough(std::vector<MType *> parValues, unsigned int pid)
{
	bool ashow    = (dynamic_cast<MBoolType*>(parValues[0]))->getValue(); 
	string colour = (dynamic_cast<MStringType*>(parValues[1]))->getValue(); // colour	
    string input  =(dynamic_cast<MIdentifierType*>(parValues[2]))->getValue(pid); 
	string  lines  =(dynamic_cast<MIdentifierType*>(parValues[3]))->getValue(pid);
	int maxgap  = (dynamic_cast<MDoubleType*>(parValues[4]))->getValue(pid);
    int minlength = (dynamic_cast<MDoubleType*>(parValues[5]))->getValue(pid);
    bool probabilistic    = (dynamic_cast<MBoolType*>(parValues[6]))->getValue(); 
    int rho = (dynamic_cast<MDoubleType*>(parValues[7]))->getValue(pid);
	int theta = (dynamic_cast<MDoubleType*>(parValues[8]))->getValue(pid);
	int thresh_int = (dynamic_cast<MIntType*>(parValues[9]))->getValue(pid);
	const char* wName = (dynamic_cast<MStringType*>(parValues[10]))->getValue();


	if(probabilistic)
	{
		__houghprob(input,lines,rho,theta,thresh_int,minlength,maxgap);
	}
	else
	{
		__hough(input,lines,rho,theta,thresh_int);
	}

	  if(ashow)
	  {
		  Mat *img = pool->getImage(input);
		  Mat dst;
		  img->convertTo(dst,CV_8UC3);
		  cvtColor(*img,dst,CV_GRAY2RGB);
		  vloP *setoflines = pool->getlObj(lines.c_str());
		  Scalar *color = utils::toColor(colour); 
		  for(vloP::iterator it = setoflines->begin() ; it!=setoflines->end(); it++){
			  Point pt1 = (*it)[0];
			  Point pt2 = (*it)[1];
			  line( dst, pt1, pt2, *color, 1, CV_AA);
		  }
		  this->display(dst,wName);
		  delete color;
	  }

}

void __hough(string input, string lines_obj, double _rho,double _theta,int threshold)
{
	ut::Trace tr = ut::Trace("HOUGH TRANSFORM",__FILE__);

	Mat *inp = pool->getImage(input);
	Mat imp;
	if(inp->channels()>1)  cvtColor(*inp,imp,CV_RGB2GRAY);
	else inp->copyTo(imp);
	imp.convertTo(imp,CV_8U);

	double radians = _theta*CV_PI/180;
	vector<Vec2f> lines;
	HoughLines(imp, lines,_rho, radians, threshold);

	// Now we have the lines calculated. They are stored as objects with two points.
	vloP tlines;

	for( size_t i = 0; i < lines.size(); i++ )
	{
		float rho = lines[i][0], theta = lines[i][1];
		Point pt1, pt2;
	    double a = cos(theta), b = sin(theta);
		double x0 = a*rho, y0 = b*rho;
		pt1.x = cvRound(x0 + 1000*(-b));
		pt1.y = cvRound(y0 + 1000*(a));
		pt2.x = cvRound(x0 - 1000*(-b));
		pt2.y = cvRound(y0 - 1000*(a));
		loP cline;
		cline.push_back(pt1);
		cline.push_back(pt2);
		tlines.push_back(cline);
	}
	pool->storelObj(tlines,lines_obj.c_str());

};
// minLineLength – Minimum line length. Line segments shorter than that are rejected.
// maxLineGap – Maximum allowed gap between points on the same line to link them.
void __houghprob(string input, string lines_obj, double _rho,double _theta,int threshold,double minLineLength,double maxLineGap)
{
  ut::Trace tr = ut::Trace("PROBABILISTIC HOUGH TRANSFORM",__FILE__);

	Mat *inp = pool->getImage(input);
	Mat imp;
	if(inp->channels()>1)  cvtColor(*inp,imp,CV_RGB2GRAY);
	else inp->copyTo(imp);
	imp.convertTo(imp,CV_8UC1);

	double radians = _theta*CV_PI/180;
	vector<Vec4i> lines;
	HoughLinesP(imp, lines,_rho, radians, threshold,minLineLength,maxLineGap);

	// Now we have the lines calculated. They are stored as objects with two points.
	vloP tlines;

	for( size_t i = 0; i < lines.size(); i++ )
	{
		Point pt1, pt2;
		pt1.x = lines[i][0];
		pt1.y = lines[i][1];
		pt2.x = lines[i][2];
		pt2.y = lines[i][3];
		loP cline;
		cline.push_back(pt1);
		cline.push_back(pt2);
		tlines.push_back(cline);
	}
	pool->storelObj(tlines,lines_obj.c_str());

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


