#include "SegmentationProcessing.h"
#include <float.h>
#include <stack>
#include <queue>
#include <vector>
#include <math.h>
#include "proctools.h"

using namespace std;
#define IJ(i,j) ((i)*m+(j))



std::map<std::string, SegmentationProcessing::Function > SegmentationProcessing::tFunc;
std::map<std::string, int> SegmentationProcessing::eMap;


/* -------------------------------------------------------------------------
Implementation of the Voronoi-based segmentation on image manifolds [1]

The code below is based on the 'IdentifySecPropagateSubfunction.cpp'
module (revision 4730) of CellProfiler [2]. CellProfiler is released
under the terms of GPL, however the LGPL license was granted by T. Jones
on Feb 7, 07 to use the code in the above file for this project.

[1] T. Jones, A. Carpenter and P. Golland,
    "Voronoi-Based Segmentation of Cells on Image Manifolds"
    CVBIA05 (535-543), 2005

[2] CellProfiler: http://www.cellprofiler.org

Copyright (C) of the original CellProfiler code:
 - Anne Carpenter <carpenter@wi.mit.edu>
 - Thouis Jones <thouis@csail.mit.edu>
 - In Han Kang <inthek@mit.edu>

Copyright (C) of partial implementation below:
 - Oleg Sklyar <osklyar@ebi.ac.uk>
 - Wolfgang Huber <huber@ebi.ac.uk>

See: ../LICENSE for license, LGPL.

------------------------------------------------------------------------- */


static double
clamped_fetch(Mat &image, 
              unsigned int i, unsigned int j,
              unsigned int m, unsigned int n)
{
  if (i < 0) i = 0;
  if (i >= n) i = n-1;
  if (j < 0) j = 0;
  if (j >= m) j = m-1;

  if(image.type()==CV_32F) return (image.at<float>(i,j));
  if(image.type()==CV_16U) return (image.at<unsigned short>(i,j));
  if(image.type()==CV_8U) return (image.at<uchar>(i,j));
  return 0;
}




inline static double Difference(Mat &image,
           unsigned int i1,  unsigned int j1,
           unsigned int i2,  unsigned int j2,
           unsigned int m,   unsigned int n,
           double lambda)
{
  int delta_i, delta_j;
  double pixel_diff = 0.0;

  /* At some point, the width over which differences are calculated should probably be user controlled. */
  for (delta_j = -1; delta_j <= 1; delta_j++) {
    for (delta_i = -1; delta_i <= 1; delta_i++) {
      pixel_diff += fabs(clamped_fetch(image, i1 + delta_i, j1 + delta_j, m, n) - 
                         clamped_fetch(image, i2 + delta_i, j2 + delta_j, m, n));
    }
  } 
  double dEucl = (double(i1)-i2)*(double(i1)-i2) + (double(j1)-j2)*(double(j1)-j2);
  return  sqrt((pixel_diff*pixel_diff + lambda*dEucl)/(1.0 + lambda));
 // return (sqrt(pixel_diff*pixel_diff + (fabs((double) i1 - i2) + fabs((double) j1 - j2)) * lambda * lambda));
}


inline void SegmentationProcessing::push_neighbors_on_queue(PixelQueue &pq,
						double dist,
                        Mat &image,
                        unsigned int i,unsigned int j,
                        unsigned int m,unsigned int n,
                        double lambda, int label,
                        int *labels_out,
						Mat &mask)
{
  /* TODO: Check if the neighbor is already labelled. If so, skip pushing. 
   */    
    int val;
  /* 4-connected */
  if (i > 0) {
	val  = labels_out[IJ(i-1,j)];
	if (val==0 && (mask.at<uchar>(i-1, j))!=0)
	{ // if the neighbor was not labeled, do pushing
	  Pixel pix(dist + Difference(image, i, j, i-1, j, m, n, lambda), i-1, j, label);
	  pq.push(pix);
	}
  }                                                                   
  if (j > 0) {  
	val = labels_out[IJ(i,j-1)];
	if (val==0  && (mask.at<uchar>(i, j-1))!=0)
	{
	  Pixel pix(dist + Difference(image, i, j, i, j-1, m, n, lambda), i, j-1, label);
	  pq.push(pix);
	}
  }                                                                   
  if (i < (n-1)) {
	 val =  labels_out[IJ(i+1,j)];
	 if (val==0 && (mask.at<uchar>(i+1, j))!=0)
	 { 
	   Pixel pix(dist + Difference(image, i, j, i+1, j, m, n, lambda), i+1, j, label);
	   pq.push(pix);
	 }
  }                                                                   
  if (j < (m-1)) { 
	  val = labels_out[IJ(i,j+1)];
	  if (val==0 && (mask.at<uchar>(i, j+1))!=0)
	  {
      Pixel pix(dist + Difference(image, i, j, i, j+1, m, n, lambda), i, j+1, label);
      pq.push(pix);

	  }
  } 
  /* 8-connected */
   if ((i > 0) && (j > 0)) {
	val = labels_out[IJ(i-1,j-1)];
	if (val==0 && (mask.at<uchar>(i-1, j-1))!=0)
	{ 
		Pixel pix(dist + Difference(image, i, j, i-1, j-1, m, n, lambda), i-1, j-1, label);
		pq.push(pix);
	}
  }                                                                       
  if ((i < (n-1)) && (j > 0)) {    
	val=labels_out[IJ(i+1,j-1)];
	if (val==0 && (mask.at<uchar>(i+1, j-1))!=0) 
	{ 
		Pixel pix(dist + Difference(image, i, j, i+1, j-1, m, n, lambda), i+1, j-1, label);
		pq.push(pix);
	}
  }                                                                       
  if ((i > 0) && (j < (m-1))) {     
	  val =labels_out[IJ(i-1,j+1)];
	  if (val==0 && (mask.at<uchar>(i-1, j+1))!=0 )
	  {
		  Pixel pix(dist + Difference(image, i, j, i-1, j+1, m, n, lambda), i-1, j+1, label);
		  pq.push(pix);
	  }
  }                                                                       
  if ((i < (n-1)) && (j < (m-1))) {
	  val=labels_out[IJ(i+1,j+1)];
	  if (val==0 && (mask.at<uchar>(i+1, j+1))!=0)
	  { 
		  Pixel pix(dist + Difference(image, i, j, i+1, j+1, m, n, lambda), i+1, j+1, label);
		  pq.push(pix);
	  }
  }

}


int  SegmentationProcessing::__propagate(const char* seeds, const char* input,const char*output, const char* mask, double lambda)
{
  
	ut::Trace tr = ut::Trace("__propagate",__FILE__);

	vloP* labels_in = pool->getlObj(seeds);
	Mat*  image = pool->getImage(input);
	Mat*  _mask = pool->getImage(mask);
	tr.printMatrixInfo("IMAGE",*image);
	tr.printMatrixInfo("mask",*_mask);


	if(image->channels()>1) //normalize
	{
		cvtColor(*image,*image, CV_RGB2GRAY);
		double minVal, maxVal;
		minMaxLoc(*image, &minVal, &maxVal); 	
		if(image->type()== CV_16U) 
				image->convertTo(*image,CV_16UC1,65535/(maxVal - minVal), -minVal * 65535/(maxVal - minVal));
		else 
				image->convertTo(*image,CV_8UC1,255/(maxVal - minVal), -minVal * 255/(maxVal - minVal));
	
	}
	 /* initialize dist to Inf */
	unsigned int n = image->rows;
	unsigned int m = image->cols;
	double  *dists =  new double[n*m]; // (double *)calloc(n*m,sizeof(double)); 
	int *labels_out =  new int[n*m];//(int *)calloc(n*m,sizeof(int));

	vloP objects_out;

	unsigned int i, j;
	PixelQueue pq;

	for (i=0; i<m*n; i++)
	{
			labels_out[i]=0;
			dists[i]=DBL_MAX;
	}
	// Populate matrix with labels, each object gets one label
    int count=1;
	for (vloP::iterator it = labels_in->begin(); it!=labels_in->end(); ++it,++count)
	{
				objects_out.push_back(loP());
				for (loP::iterator obj = it->begin(); obj!=it->end(); ++obj)
				{
					j = obj->x;
				    i = obj->y;
					labels_out[IJ(i,j)]= count;
					dists[IJ(i,j)]=0.0;
				} 
   }
	
  /* if the pixel is already labeled (i.e, labeled in labels_in) and within a mask, 
   * then set dist to 0 and push its neighbors for propagation */
  // double t1 = tr.GetTimeMs64();
  for (vloP::iterator it = labels_in->begin(); it!=labels_in->end(); ++it)
  {
				for (loP::iterator obj = it->begin(); obj!=it->end(); ++obj)
				{			    
				   j = obj->x;
				   i = obj->y;
				 if((_mask->at<uchar>(i,j))!=0) 
				 {	
				 push_neighbors_on_queue(pq,0.0, *image, i, j, m, n, lambda,labels_out[IJ(i,j)], labels_out,*_mask);
				 }
				}
  }
 //  double t2 = tr.GetTimeMs64();
 //  tr.printTime(t1,t2); 
 //  t1 = tr.GetTimeMs64();
  while (! pq.empty())
  {
		Pixel p = pq.top();
		pq.pop();  // for priority_queues from standard or boost
		
		 if (dists[IJ(p.i, p.j)] > p.distance)
		 {
			dists[IJ(p.i, p.j)] = p.distance;
			labels_out[IJ(p.i,p.j)] = p.label;
			push_neighbors_on_queue(pq, p.distance, *image, p.i, p.j, m, n, lambda, p.label, labels_out, *_mask);
		}
	//	delete &p;
  }
 //  t2 = tr.GetTimeMs64();
 //  tr.printTime(t1,t2);
	for(unsigned int i = 0; i < n; i++)
	{
		 for(unsigned int j = 0; j < m; j++){
			 if(labels_out[IJ(i,j)]!=0)
			 {
				objects_out[labels_out[IJ(i,j)]-1].push_back(Point(j,i));
			 }
		}
	}

	
  //save image mask
	

    pool->storelObj(objects_out,output); 
	delete [] dists;
	delete [] labels_out; 
	return 0;
}
/*****************************************************************************************************
*  CONTAMINATION ALG
*  Simple stack which propagates a tag in four directions.
*  The diagonals are not considered "touching" neighbors. 
*******************************************************************************************************/
void SegmentationProcessing::contaminate(vloP &o_out,Mat &lab_copy,int i, int j,unsigned int &last_tag){

	stack <Point> checked;
	int _i, _j;
	Point n;
	checked.push(Point(j,i));
	o_out.push_back(loP());
	
	o_out[last_tag].push_back(Point(j,i));
	lab_copy.at<uchar>(i,j)=0;

	while(!checked.empty())
	{
		n = checked.top();
		checked.pop();	
		_i = n.y;
		_j = n.x;
		
		
		if ( _i+1 < lab_copy.rows) 
			if(lab_copy.at<uchar>(_i+1,_j)!=0)
			{ 		
				lab_copy.at<uchar>(_i+1,_j)=0;
				checked.push(Point(_j,_i+1));	
				o_out[last_tag].push_back(Point(_j,_i+1));
			}
		if ( _j+1 < lab_copy.cols) if(lab_copy.at<uchar>(_i,_j+1)!=0)
			{ 
				lab_copy.at<uchar>(_i,_j+1)=0;
				checked.push(Point(_j+1,_i));	
				o_out[last_tag].push_back(Point(_j+1,_i));
		    }
		if ( _i-1 > 0)
			if(lab_copy.at<uchar>(_i-1,_j)!=0)
		    {
				lab_copy.at<uchar>(_i-1,_j)=0;
				checked.push(Point(_j,_i-1));	
				o_out[last_tag].push_back(Point(_j,_i-1));
			}
		if ( _j-1 > 0)
			if(lab_copy.at<uchar>(_i,_j-1)!=0)
			{
			  lab_copy.at<uchar>(_i,_j-1)=0;
			  checked.push(Point(_j-1,_i));	
			  o_out[last_tag].push_back(Point(_j-1,_i));
			}	
	}
	last_tag++;	
return;
}


/*****************************************************************************************************
*  CONTAMINATION ALG
*  Simple stack which propagates a tag in four directions.
*  The diagonals are not considered "touching" neighbors. 
*******************************************************************************************************/
void SegmentationProcessing::contaminate(vloP &o_out,Mat &lab_copy,int i, int j,unsigned int &last_tag,int limit){

	stack <Point> checked;
	int _i, _j;
	Point n;
	checked.push(Point(j,i));
	o_out.push_back(loP());
	
	o_out[last_tag].push_back(Point(j,i));
	lab_copy.at<float>(i,j)=0;

	while(!checked.empty())
	{
		n = checked.top();
		checked.pop();	
		_i = n.y;
		_j = n.x;
		
		
		if ( _i+1 < lab_copy.rows) 
			if(lab_copy.at<float>(_i+1,_j)==limit)
			{ 		
				lab_copy.at<float>(_i+1,_j)=0.0;
				checked.push(Point(_j,_i+1));	
				o_out[last_tag].push_back(Point(_j,_i+1));
			}
		if ( _j+1 < lab_copy.cols) if(lab_copy.at<float>(_i,_j+1)==limit)
			{ 
				lab_copy.at<float>(_i,_j+1)=0.0;
				checked.push(Point(_j+1,_i));	
				o_out[last_tag].push_back(Point(_j+1,_i));
		    }
		if ( _i-1 > 0)
			if(lab_copy.at<float>(_i-1,_j)==limit)
		    {
				lab_copy.at<float>(_i-1,_j)=0.0;
				checked.push(Point(_j,_i-1));	
				o_out[last_tag].push_back(Point(_j,_i-1));
			}
		if ( _j-1 > 0)
			if(lab_copy.at<float>(_i,_j-1)==limit)
			{
			  lab_copy.at<float>(_i,_j-1)=0.0;
			  checked.push(Point(_j-1,_i));	
			  o_out[last_tag].push_back(Point(_j-1,_i));
			}	
	}
	last_tag++;	
return;
}



/*****************************************************************************************************
*  LABEL
*  Given a binary image (8U) creates a vector of objects by connected components.
*******************************************************************************************************/

int SegmentationProcessing::__label(const char* input, const char*output){
	 
	 ut::Trace tr = ut::Trace("label",__FILE__); 
	
	 vloP objects_out;
	 Mat*  bin_im = pool->getImage(input);
	 Mat lab_copy;
	 bin_im->copyTo(lab_copy);
	 unsigned int total_objs = 0;

	 tr.message("\n Labeling image:");
	 tr.printMatrixInfo("binary Im",*bin_im);
	 if(bin_im->depth()!=CV_8U) 
	 {
	    cout<<"Only images of 8 bit (binary) can be labelled!" <<endl;
		exit(-1984);
	 }

	for(int i = 0; i < bin_im->rows; i++)
	{
		const uchar* Mi = lab_copy.ptr<uchar>(i);
		for(int j = 0; j < bin_im->cols; j++)
		{
			if(Mi[j]!=0)
			{
			contaminate(objects_out,lab_copy,i,j,total_objs);
			}
		}
	 }

	 pool->storelObj(objects_out,output);


	 return 0;
}


int SegmentationProcessing::__label(Mat &im,vloP &objects_out){
	 
	 ut::Trace tr = ut::Trace("label",__FILE__); 
	
	 Mat lab_copy;
	 im.copyTo(lab_copy);
	 unsigned int total_objs = 0;

	 tr.message("\n Labeling image:");
	 tr.printMatrixInfo("binary Im",im);

	for(int i = 0; i < im.rows; i++)
	{
		const uchar* Mi = lab_copy.ptr<uchar>(i);
		for(int j = 0; j < im.cols; j++)
		{
			if(Mi[j]!=0)
			{
			contaminate(objects_out,lab_copy,i,j,total_objs);
			}
		}
	 }
	 return 0;
}



/*****************************************************************************************************
*  Split By Otsu
* 
* 
*******************************************************************************************************/

int SegmentationProcessing::__splitbyOtsu(const char* reference, const char* load_objects, const char* save_objects)
{

	vloP* lobj_in = pool->getlObj(load_objects);
	Mat*  ref_im = pool->getImage(reference);
	
	// Initialize blank image to 8 bits for binarization
	Mat ref2(ref_im->rows,ref_im->cols, CV_32FC1, Scalar(0));



	double t = 0.0;
/*********************************/
// For each loP (object = list of Points)
	// get Otsu�s threshold
	// 
	loP *vm;
	int i,j;
	double count=0;
	for (vloP::iterator it = lobj_in->begin(); it!=lobj_in->end(); ++it)
	{	
				vm = &(*it);
				t = proctools::otsu_thresh(*ref_im,*vm);
				count++;
				for (loP::iterator obj = it->begin(); obj!=it->end(); ++obj)
				{			    
				   j = obj->x;
				   i = obj->y;
				   // Each point under the threshold is removed.
				   if((ref_im->at<float>(i,j))<t) 
					{	
				       ref2.at<float>(i,j)=0.0;
				    }
				   else
				   {
				      ref2.at<float>(i,j)=count;
				   }
				}
  }

	 vloP objects_out;
	 Mat lab_copy;
	 ref2.copyTo(lab_copy);
	 unsigned int total_objs = 0;

	// display(ref2,"Works!!");
/******Now the image is relabeled*************************/
   
	for(int i = 0; i < ref2.rows; i++)
	{
		const float* Mi = lab_copy.ptr<float>(i);
		for(int j = 0;  j < ref2.cols; j++)
		{
			if(Mi[j]>0.001)
			{
			SegmentationProcessing::contaminate(objects_out,lab_copy,i,j,total_objs,Mi[j]);
			}
		}
	 }

  pool->storelObj(objects_out,save_objects);
  return 0;
}
