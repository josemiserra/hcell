
#ifndef _FEATURE_CALCULATOR_
#define _FEATURE_CALCULATOR_

#include <iostream>
#include <array> 
#include <sstream>
#include <fstream>
#include <string>
#include <map>
#include <bitset>
#include <opencv2/opencv.hpp>
#include "PModule.h"
#include "MType.h"
#include "MAllTypes.h"
#include "Action.h"
#include "utils.h"

using namespace cv;
using namespace std;

/*
Accumulator extracted from 
http://roth.cs.kuleuven.be/w-ess/index.php/Accurate_variance_and_mean_calculations_in_C%2B%2B11

*/



/* -------------------------------------------------------------------------- */
# define  SMALL 1e-7 //stops log2() breaking if the data is 0
# define  IND(I,X,Y) (X) + (Y)*(nc) + (I)*(nc)*(nc)
#define	  IJ(i,j) ((i)*nx+(j))


template <typename T, typename T2=T>
struct accumulator
{
 
	T2 sum; // we could plug in a more accurate type for the sum
    T S;
    T M;
    size_t N;
 
    // default constructor initializes all values
    accumulator() : sum(0), S(0), M(0), N(0) { }
 
	void clear()
	{ 
	 sum = 0.0;
	 S=0.0;
	 M=0.0;
	 N=0.0;
	}
    // add another number
    T2 operator()(const T& x)
    {
        ++N;
        sum += x;
        T Mprev = M;
        M += (x - Mprev) / N;		 	
        S += (x - Mprev) * (x - M);  // TODO: Review here...
        if(S!=S) S=0.0;
		return sum;
    }
 
    T mean() const
    {
        return M;
    }
 
    T variance() const
    {
        if(N==1) return 0;
		return S / (N - 1);
    }


	struct myclass {
		bool operator() (T i,T j) { return (i<j);}
			} comparator;

	/*
	Quantiles are calculated using the Type 7 method in R
	*/

	vector<T> quantiles(vector<T> &a) const
	{ 
		double tot,h,_h_;
	    vector<T> quants;
		int n= a.size();
		
		if(a.size()==0)
		{
		    for(int i=0;i<5;i++) quants.push_back(0.0);
			return quants;
		}
		if(a.size()==1)
		{
			for(int i=0;i<5;i++) quants.push_back(a[0]);
			return quants;
		} 
		sort(a.begin(),a.end(),comparator);
	  // basic.quantiles=(0.01, 0.05, 0.5, 0.95, 0.99)
		
		h = (n-1)*0.01+1;
		_h_ = floor(h);
		tot = a[_h_-1]+(h-_h_)*(a[_h_]-a[_h_-1]);
		quants.push_back(tot);
		
		h = (n-1)*0.05+1;
		_h_ = floor(h);
		tot = a[_h_-1]+(h-_h_)*(a[_h_]-a[_h_-1]);
		quants.push_back(tot); 

		h = (n-1)*0.5+1;
		_h_ = floor(h);
		tot = a[_h_-1]+(h-_h_)*(a[_h_]-a[_h_-1]);
		quants.push_back(tot); 

		h = (n-1)*0.95+1;
		_h_ = floor(h);
		tot = a[_h_-1]+(h-_h_)*(a[_h_]-a[_h_-1]);
		quants.push_back(tot); 
		
		h = (n-1)*0.99+1;
		_h_ = floor(h);
		tot = a[_h_-1]+(h-_h_)*(a[_h_]-a[_h_-1]);
		quants.push_back(tot); 

	/*	Type SAS-5/ R-2  implementation
	********************************
	    fl = floor(n*0.01);
		if((n*0.01-fl)>0.001)
		{
			val1 = a[fl];
		    val2 = a[fl+1];
		    tot = (val1+val2)/2;
			quants.push_back(tot);
		}
		else
		{
			quants.push_back(a[fl+1]);
		}
		fl = floor(n*0.05);
		if((n*0.05-fl)>0.001)
		{
			val1 = a[fl];
		    val2 = a[fl+1];
		    tot = (val1+val2)/2;
			quants.push_back(tot);
		}
		else
		{
			quants.push_back(a[fl+1]);
		}

		n= n-1;
		fl = floor(n*0.5);
		if((n*0.5-fl)>0.01)
		{
			val1 = a[fl];
		    val2 = a[fl+1];
		    tot = (val1+val2)/2;
			quants.push_back(tot);
		}
		else
		{
			quants.push_back(a[fl+1]);
		}
		fl = floor(n*0.95);
		if((n*0.95-fl)<0.01)
		{
			val1 = a[fl];
		    val2 = a[fl+1];
		    tot = (val1+val2)/2;
			quants.push_back(tot);
		}
		else
		{
			quants.push_back(a[fl+1]);
		}
		fl = floor(n*0.99);
		if((n*0.99-fl)<0.01)
		{
			val1 = a[fl];
		    val2 = a[fl+1];
		    tot = (val1+val2)/2;
			quants.push_back(tot);
		}
		else
		{
			quants.push_back(a[fl+1]);
		}

		*/

		return quants;

	}

	T mad(vector<T> &a) {
		T _mad;
		T center;
		vector<T> absolute;
		int n = a.size();
		int n2 = (int)floor((double)n/2.);
	//	qsort(&a, n, sizeof(a[0]), compare);
		sort(a.begin(),a.end(),comparator);
		center = a[n2];
		for(auto it = a.begin(); it!=a.end();it++)
		{
			absolute.push_back(abs((*it) - center));
		}
	    sort(absolute.begin(),absolute.end(), comparator);
		_mad = 1.4826*absolute[n2];
		return _mad;
    }
	 
};

/*
template <typename T>
T two_sum_replace(T& a, const T& b)
{
    volatile T x = a + b;
    T z = x - a;
    T y = (a - (x - z)) + (b - z);
    a = x;
    return y;
}




template <unsigned short K, typename T>
struct sumK
{
    std::array<T, K-1> qs; // length K-1 array of type T
    T s; // the final bits we could not capture with ''K-fold precision''
 
    // default constructor
    // the first element of qs should be the initialization value
    // for the sum, being x0, other values are initialized to zero
    sumK(const T& x0=0) : qs(), s(0) { qs[0] = x0; }
 
    // add another value using ''K-fold precision''
    // this corresponds to the first two for loops in the paper
    sumK& operator+=(const T& x)
    {
        auto alpha = x;
      //  for(auto& q : qs) // note the &, we want to update the q...
		for(int i=0;i< qs.size();i++)
		{
			alpha = two_sum_replace(qs[i], alpha);
		}
		s += alpha;
        return *this;
    }
 
    // if we ask the numerical value (as type T) of this object, then
    // we need to add up all our correction terms
    operator T() const
    {
        // need to take copies, we don't want to change our object
        std::array<T, K-1> qs(this->qs);
        T s = this->s;
        // this corresponds to the last for loop in the paper
        for(int j = 0; j < K-2; ++j) {
            auto alpha = qs[j];
            for(int k = j+1; k < K-1; ++k) {
                alpha = two_sum_replace(qs[k], alpha);
            }
            s += alpha;
        }
        return qs.back() + s;
    }

}; */

 class FeatureCalculator
{

private:
	static FeatureCalculator* featcalc;


	FeatureCalculator()
	{
		
	}
	
	
public:
	 static const double PI;


	 static FeatureCalculator* getInstance()
      {
          if(featcalc==nullptr)
		  {
			featcalc= new FeatureCalculator();
		  }
			
          return featcalc;
      }

	 ~FeatureCalculator(void)
	 {
	    delete featcalc;
	 };


	void static basic(vloP &objects,Mat &reference,bitset<4> &options,vector<double> &gen_values,vector<vector<double>> &ind_values)  // based on intensity
	{
		
		
		ut::Trace tr = ut::Trace("FeatureCalculator::basic",__FILE__);
		tr.message("Computing Features: basic");
		// in this case we have the reference matrix with the intensities 
		// and second the list of points for each object (vloP)
		vector<double> myValues;
		vector<double>	tmp2;
		Mat ref2;

        // construct a new accumulator which will
        // be passed around the whole for loop 
        // and shows up as the return value in the end
	//	accumulator<double,sumK<3,double>> a; // 3-fold precision of data
		accumulator<double,double> a; // 3-fold precision of data
		Point p;

		// Next step is create a double�s vector with the intensity values of each list of Points
		// int type = reference.type();
		// Data must be normalized between 0 and 1 to a 32F matrix
		double min;
		double max;
		minMaxIdx(reference, &min, &max);
		
		if(reference.type() == CV_8U || reference.type() == CV_16U || max >1.0 ) 
		{	
			reference.convertTo(ref2, CV_64F);
			

			for( int y = 0; y < ref2.rows; y++ )
			 for( int x = 0; x < ref2.cols; x++ )
				{
					ref2.at<double>(y,x)=((ref2.at<double>(y,x)-min)/(max-min));
				}
		}
		else
		{
			reference.convertTo(ref2, CV_64F);
		}
		if(objects.size()==0)
		{
			tr.message("WARNING: Features cannot be calculated if objects container empty!");
			vector<double> temp;
			if(options[0]) 			temp.push_back(0);
			if(options[1])			temp.push_back(0);	
			if(options[2])			temp.push_back(0);	
			if(options[3]) // QUANTILES
			{
			 for(int i=0; i<5; i++)			  temp.push_back(0);	
			}
		    ind_values.push_back(temp);
			for(unsigned int i=0;i<ind_values[0].size();i++)
			{
				gen_values.push_back(0);
				gen_values.push_back(0);
			}
			return;
		}


		for(vloP::iterator it = objects.begin(); it!=objects.end(); ++it)
		{
			for(loP::iterator obj = (*it).begin(); obj!=(*it).end(); ++obj)
			{
			 p = *obj;
			 myValues.push_back(ref2.at<double>(p.y,p.x));
			 a((const double)myValues.back());
			}
		    vector<double> temp;

		
		   if(options[0]) // Mean
			{			
			 temp.push_back(a.mean());
			}
			if(options[1]) // SD
			{
			 temp.push_back(sqrt(a.variance()));	
			}	
			if(options[2]) // MAD
			{
			 temp.push_back(a.mad(myValues));	
			}
			if(options[3]) // QUANTILES
			{
			tmp2 = a.quantiles(myValues);
			 for(auto itv = tmp2.begin(); itv != tmp2.end(); itv++)
			 {	
			  temp.push_back(*itv);	
			  }
			 tmp2.clear();
			} 
		    ind_values.push_back(temp);
			a.clear();
			myValues.clear();
		} 
		// so far we computed for each object	
		// now we are going to compute for general.  
		 accumulator<double,double> ga;
		for(unsigned int i=0;i<ind_values[0].size();i++)
		{
			
			for(auto itg = ind_values.begin(); itg!= ind_values.end();++itg)
			{
				ga((*itg)[i]);
			}
			gen_values.push_back(ga.mean());
			gen_values.push_back(sqrt(ga.variance()));
		    ga.clear();
		}
 // Search the reference value

		  
	}

void static shape(vloP &objects,vloP &contours, Mat &ref,bitset<4> &options,vector<double> &gen_values,vector<vector<double>> &ind_values)  // based on intensity
	{
		ut::Trace tr = ut::Trace("FeatureCalculator::shape",__FILE__);
		tr.message("Computing Features: shape");
		{
		 vector<double> tot;
		 
		 double t1xmean;
		 double t1ymean;
		 double  temp1x,temp1y;
		 accumulator<double> t1xa;
		 accumulator<double> t1ya;

		if(objects.size()==0)
		{
			tr.message("WARNING: Features cannot be calculated if objects container empty!");
			vector<double> temp;
			if(options[0]) 			temp.push_back(0);
			if(options[1])			temp.push_back(0);	
			if(options[2]) // RADIUS
			{
			 for(int i=0; i<3; i++)			  temp.push_back(0);	
			}
			if(options[3])			temp.push_back(0);	
		    ind_values.push_back(temp);
			for(unsigned int i=0;i<ind_values[0].size();i++)
			{
				gen_values.push_back(0);
				gen_values.push_back(0);
			}
			return;
		}


		vloP::iterator itc = contours.begin();
		for(vloP::iterator it = objects.begin(); it!=objects.end(); ++it,++itc)
		{

		  vector<double> temp;
		 if(options[0]) // Area
			{			
				temp.push_back((*it).size());
			}
			if(options[1]) // Perimeter
			{
				temp.push_back((*itc).size());
			}	
			if(options[2]) // Radius.mean, radius.max, radius.min
			{
				
				for(loP::iterator itn = (*itc).begin(); itn!=(*itc).end(); ++itn)
				{
					t1xa((*itn).x);
					t1ya((*itn).y);
				}

				 t1xmean = t1xa.mean();
				 t1xa.clear();
				 t1ymean = t1ya.mean();
				 t1ya.clear();

				 for(loP::iterator itn = (*itc).begin(); itn!=(*itc).end(); ++itn)
				 {
					temp1x = ((double)(*itn).x)-t1xmean;
					temp1y= ((double)(*itn).y)-t1ymean;
					tot.push_back(sqrt(temp1x*temp1x + temp1y*temp1y));;
				 }
				 auto a = std::for_each(  tot.begin(), tot.end(), accumulator<double>());
				 temp.push_back(a.mean());	
				 a.clear();
				 sort(tot.begin(),tot.end());

				 if(tot.size()==0)
				 { 
					 tr.message("ERROR: Wrong segmentation system or wrong contours. ########");
					 tr.message("Object with size 0 found!!! \n ########");
					 exit(-1962);
				 }
				 double min = tot[0];
				 double max = tot[tot.size()-1];
				 temp.push_back(max);
				 temp.push_back(min); 
				 tot.clear(); 
			}
			if(options[3]) // roundness = 1/compactness
			{
				double l= (*itc).size();
				double a = (*it).size();
				double compactness= (l*l)/(FeatureCalculator::PI*4*a);
				temp.push_back(1/compactness);
			}
			 ind_values.push_back(temp);
			
			}
		}
		// so far we computed for each object	
		// now we are going to compute for general.  
		accumulator<double,double> ga;
		for(unsigned int i=0;i<ind_values[0].size();i++)
		{
			
			for(auto itg = ind_values.begin(); itg!= ind_values.end();++itg)
			{
				ga((*itg)[i]);
			}
			gen_values.push_back(ga.mean());
			gen_values.push_back(sqrt(ga.variance()));
		    ga.clear();
		}

	}

void static moment(vloP &objects, Mat &reference,bitset<4> &options,vector<double> &gen_values,vector<vector<double>> &ind_values, bool woref)  // based on intensity
	{
		ut::Trace tr = ut::Trace("FeatureCalculator::moment",__FILE__);
		tr.message("Computing Features: moment");
		{
		 accumulator<double,double> t1x,t1y,t1xy,t2x,t2y,vala;
		 double m00,m01,m10,m11,m02,m20,val;
		 double cx,cy,l1,l2,mu20,mu02,mu11,det,eccentricity,theta;
		 Point p;
		 
		 Mat ref2;
		 double min,max;
		 
		 if(!woref)
		 { 
			 tr.message("Moment using REFERENCE.");
			 minMaxIdx(reference, &min, &max);
			 reference.convertTo(ref2, CV_64F);
		 }
		 else
		 {
		 tr.message("Moment without using REFERENCE.");

		 }
		 if(objects.size()==0)
		{
			tr.message("WARNING: Features cannot be calculated if objects container empty!");
			vector<double> temp;
			if(options[0]) 			
			{
				temp.push_back(0);
				temp.push_back(0);
			}
			if(options[1])
			{
				temp.push_back(0);
				temp.push_back(0);
			}
			if(options[2])			temp.push_back(0);	
			if(options[3])			temp.push_back(0);	
		    ind_values.push_back(temp);
			for(unsigned int i=0;i<ind_values[0].size();i++)
			{
				gen_values.push_back(0);
				gen_values.push_back(0);
			}
			return;
		}


		for(vloP::iterator it = objects.begin(); it!=objects.end(); ++it)
		{

		  vector<double> temp;
		  
		  for(loP::iterator itn = (*it).begin(); itn!=(*it).end(); ++itn)
		  {	    
		    
			p = *itn;			
			
			if(woref)
			{
				val=1.0;
			}
			else
			{
				if(max > 1.0) val=((ref2.at<double>(p.y,p.x)-min)/(max-min));
				else val = ref2.at<double>(p.y,p.x);
			 }
			vala(val);
			t1x(val*p.x);
			t1y(val*p.y);
			t1xy(val*p.x*p.y);
			t2x(val*p.x*p.x);
			t2y(val*p.y*p.y);

		  }
            
			m00 = vala.sum;
			m10 = t1x.sum;
			m01 = t1y.sum;
			m11 = t1xy.sum;
			m20 = t2x.sum;
			m02 = t2y.sum;

			cx = m10/m00;
		    cy = m01/m00;

			mu20 = m20/m00 - cx*cx;
            mu02 = m02/m00 - cy*cy;
             
			mu11 = m11/m00 - cx * cy;
		    
			det = sqrt(4 * mu11*mu11 + (mu20 - mu02)*(mu20 - mu02));
			theta = atan2(2 * mu11, (mu20 - mu02))/2;
			l1 = sqrt((mu20 + mu02 + det)/2) * 4;
            l2 = sqrt((mu20 + mu02 - det)/2) * 4;
            if(l1>0.0)
				eccentricity = sqrt(1 - (l2*l2)/(l1*l1));
			else
				eccentricity = 0.0;

			if(options[0]) // c.x and c.y
			{			
				 temp.push_back(cx);
				 temp.push_back(cy);
			}
			if(options[1]) // major and minor axis
			{
				
				temp.push_back(l1);
				temp.push_back(l2);
         
			}	
			if(options[2]) // eccentricity
			{
				temp.push_back(eccentricity);
			}
			if(options[3]) // theta
			{
				
				temp.push_back(theta);
			}
			
			ind_values.push_back(temp);
			t1x.clear();
			t1y.clear();
			t1xy.clear();
			t2x.clear();
			t2y.clear();
			vala.clear();
			}
		}
		// so far we computed for each object	
		// now we are going to compute for general.  
		accumulator<double,double> ga;
		for(unsigned int i=0;i<ind_values[0].size();i++)
		{
			
			for(auto itg = ind_values.begin(); itg!= ind_values.end();++itg)
			{
				ga((*itg)[i]);
			}
			gen_values.push_back(ga.mean());
			gen_values.push_back(sqrt(ga.variance()));
		    ga.clear();
		}

	}

/* -------------------------------------------------------------------------
Calculating Haralick image features
Copyright (c) 2007 Oleg Sklyar, Mike Smith
 LGPL

   Given an array consisting of layers of square co-occurrence matrices,
   one layer per object, this function returns a matrix of selected haralick
   features (rows - objects, columns - features)

------------------------------------------------------------------------- */

/* calculates haralick cooccurrence matrix
   http://murphylab.web.cmu.edu/publications/boland/boland_node26.html

   This matrix is square with dimension Ng, where Ng is the number of gray levels in the image.
   Element [i,j] of the matrix is generated by counting the number of times a pixel with value
   i is adjacent to a pixel with value j and then dividing the entire matrix by the total number
   of such comparisons made.
   Each entry is therefore considered to be the probability that a pixel with value i will be
   found adjacent to a pixel of value j.

   Since adjacency can be defined to occur in each of four directions in a 2D, square pixel image
   (horizontal, vertical, left and right diagonals), four such matrices can be calculated.
---------------------------------- */
void static haralickMatrix(int *data, int nobj,Mat &ref,int cgrades,vector<double> &c)
{
	ut::Trace tr = ut::Trace("FeatureCalculator::haralickMatrix",__FILE__);
	tr.message("Computing Matrix: haralick");

	int nx, ny, im, x, y, index, i,j, nc, colthis, colthat, no_objects, * ncomp;
	double  * refdata, *cmdata;

	Point p;

	nx = ref.cols;
	ny = ref.rows;
	nc = cgrades;
	if ( nc < 2 )
	{
		tr.message( "the number of color grades must be larger than 1" );
		exit(-1812);
	}

	Mat ref2;
	ref.convertTo(ref2, CV_64F);

	double min;
	double max;
    minMaxIdx(ref, &min, &max);

	refdata =new double[ nx * ny ];

	// We need to normalize for applying the bin ranges
	for( i = 0; i < ny; i++ )
	  for(  j = 0; j < nx; j++ )
          {
			refdata[IJ(i,j)]=((ref2.at<double>(i,j)-min)/(max-min));
          }

	/* get image data */
	// Transfer obj to a matrix numbered.

	/* create  co-occurrence matrix
	 * nc should be the number of gray values, but using, for example 256 gives us a matrix too sparse
	 * The best way then, is assign a number of bins, in which we split the space. Using normalized
	 * matrices will help us to fix the ranges.
	 * Ex. 0-1, and 8 bins, 0..7
	 * 0.1*7 = 0.7, floor = 0
	 * 0.5*7 = 3.5, floor = 3
	 * 0.95*7 = 6.65 , floor = 6
	 * 1 * 7 = 7, floor = 7
	*/
	cmdata = new double [nobj * nc * nc];  // e.g. 32*32*nobj   3 dimensions, 32x32x n_obj
	// initialize feature matrix with 0
	for ( index = 0; index < nobj * nc * nc; index++ ) cmdata [index] = 0.0;

	// number of comparisons for each object  = used later for normalization
	ncomp = new int[nobj];
	for ( index = 0; index < nobj; index++ )  ncomp[index] = 0;

// go through pixels and collect primary descriptors
// reason to skip lines: we compare from this to: horizontal, vertical, left diag, right-diag
 for ( x = 1; x < nx - 1; x++ )      // skip leftmost and rightmost cols
  for ( y = 0; y < ny - 1; y++ ) {  // skip bottom row
    index =  data[IJ(y,x)]; // index of the object
    if ( index < 1 ) continue;
    index--;
    colthis = floor(refdata[IJ(y,x)] * (nc - 1));
    // we compare from this to: horizontal, vertical, left diag, right-diag
    if ( data[IJ(y,x+1)] - 1 == index ) {     // 1. horizontal
      colthat = floor(refdata[IJ(y,x+1)] * (nc - 1));
      cmdata[IND(index,colthis,colthat)] += 1.0;
      cmdata[IND(index,colthat,colthis)] += 1.0;
      ncomp[index] += 2;
    }
    if ( data[IJ(y+1,x)] - 1 == index ) {   // 2. vertical
      colthat = floor(refdata[IJ(y+1,x)] * (nc - 1));
      cmdata[IND(index,colthis,colthat)] += 1.0;
      cmdata[IND(index,colthat,colthis)] += 1.0;
      ncomp[index] += 2;
    }
    if ( data[IJ(y+1,x+1)] - 1 == index ) { // 3. right-bottom
      colthat = floor(refdata[IJ(y+1,x+1)] * (nc - 1));
      cmdata[IND(index,colthis,colthat)] += 1.0;
      cmdata[IND(index,colthat,colthis)] += 1.0;
      ncomp[index] += 2;
    }
    if ( data[IJ(y+1,x-1)] - 1 == index ) { // 4. left-bottom
      colthat = floor(refdata[IJ(y+1,x-1)] * (nc - 1));
      cmdata[IND(index,colthis,colthat)] += 1.0;
      cmdata[IND(index,colthat,colthis)] += 1.0;
      ncomp[index] += 2;
    }
  }
for ( index = 0; index < nobj; index++ )
  for ( i = 0; i < nc * nc; i++ )
    if ( ncomp[index] > 0 ) cmdata[i + index * nc * nc] /= ncomp[index];

// convert result to output vector
for ( index = 0; index < nobj * nc * nc; index++ ) c.push_back(cmdata[index]);

delete[] refdata;
delete[] ncomp;
delete[] cmdata;
}


void static haralickFeatures(vector<double> cm,vector<vector<double>> &ind_values,int nc,int nobj)  // based on intensity
{

ut::Trace tr = ut::Trace("FeatureCalculator::haralickReatures",__FILE__);
tr.message("Computing Features: haralick");

double *res;
int  _nobj, index, i, j, n, nonZeros, no_objects, nf=13;
double mu, tmp;
double *p;   // p for co-occurrence matrix -- probability; f for features
double *px;      // partial probability density: rows summed, = py as matrix symmetrical
double *Pxpy, * Pxmy;
double HXY1, HXY2, entpx;  //used in calculating IMC1 and IMC2
// offsets in the res matrix for each feature, calculated from nobj
enum { ASM, CON, COR, VAR, IDM, SAV, SVA, SEN, ENT, DVA, DEN, IMC1, IMC2 };

// n = number of colors, nc, determined by the size of the haralick matrix

_nobj =nobj;


// temp vars
px   =  new double[nc];

Pxpy =  new double[2*nc +10]; // +10 is to be safe with limits
Pxmy =  new double[2*nc +10]; // +10 is to be safe with limits



// GO through objects and calculate features
for ( index = 0; index < nobj; index++ ) {
  vector<double> f;
  for ( i = 0; i <nf; i++ ) f.push_back(0.0);

  p = &( cm[index * nc * nc] );
// total number of non zeros to get mu,
	//   angular second moment (ASM),
	//   inverse diff mom      (IDM),
	//   entropy               (ENT)
// calculate Pxpy and Pxmy
  //  indexing (adding 1 to i and j) is based on Boland and netpbm,
  //  makes no sense to me
 nonZeros = 0;
for ( i = 0; i < 2 * nc + 10; i++ ) Pxpy[i] = Pxmy[i] = 0;
for ( i = 0; i < nc; i++ )
  for ( px[i]=0.0, j = 0; j < nc; j++ )
    if ( (tmp = p[i + j * nc]) > 0 ) {
      f[ASM] += tmp * tmp;
      f[IDM] += tmp / (double)(1 + (i-j)*(i-j));
      f[ENT] -= (tmp >= SMALL) ? (tmp * log10(tmp)) : (tmp * log10(SMALL));
      nonZeros++;
      Pxpy[i + j + 2] += tmp;
      Pxmy[abs(i - j)] += tmp;
      px[i] += tmp;
    }
// no sense to do anything if no non zero elements
if ( nonZeros < 1 ) continue;
// contrast              (CON)
for ( n = 1; n < nc; n++ ) { // was from 0, but n^2 at n=0 is 0, so nonsense
  tmp = 0.0;
  for ( i = 0; i < nc; i++ )
    for ( j = 0; j < nc; j++ )
      if ( abs(i - j) == n ) tmp += p[i + j * nc];
  f[CON] += n * n * tmp;
}
// correlation           (COR)
//  the calculation of mu and sd is based on Boland and netpbm,
//  but it does not make any sense to me
//  reset px and recalculate it
// the code assumes mux=muy, sdx=sdy
 for ( mu=0, tmp=0,  i = 0; i < nc; i++ ) {
  mu += i * px[i];     // why is it this way, no idea
  tmp += i * i * px[i]; // sum of squares, why it is i^2 -- no idea
}
if ( tmp - mu * mu > 0 ) { // tmp - mu * mu = sd^2
  for ( i = 0; i < nc; i++ )
    for ( j = 0; j < nc; j++ ) f[COR] += i * j * p[i + j * nc];
  f[COR] = ( f[COR] - mu * mu ) / ( tmp - mu * mu );
}
//  variance             (VAR)
//  the calculation of mu is based on Boland and netpbm,
//  but it and indexing do not make any sense to me
for ( mu=0,  i = 0; i < nc; i++ )
  for ( j = 0; j < nc; j++ ) mu += i * p[i + j * nc];
for ( i = 0; i < nc; i++ )
  for ( j = 0; j < nc; j++ )
    f[VAR] += (i + 1 - mu) * (i + 1 - mu) * p[i + j * nc];
//   sum average          (SAV)
//   sum entropy          (SEN)
for ( i = 2; i <= 2 * nc; i++ ) {
  f[SAV] += i * Pxpy[i];
  f[SEN] -= (Pxpy[i] >= SMALL) ? (Pxpy[i] * log10(Pxpy[i])) : (Pxpy[i] * log10(SMALL));
}
// sum variance         (SVA)
for ( i = 2; i <= 2 * nc; i++ )
  f[SVA] += (i - f[SEN]) * (i - f[SEN]) * Pxpy[i];
// difference Variance  (DVA)
//   difference entropy   (DEN)
for ( i = 0; i < nc - 1; i++ ) { // top: Nc - 1
  f[DVA] += i * i * Pxmy[i];
  f[DEN] -= (Pxmy[i] >= SMALL) ? (Pxmy[i] * log10(Pxmy[i])) : (Pxmy[i] * log10(SMALL));
}
// Info Measure of Correlation 1 and 2
for ( HXY1=0, HXY2=0, entpx=0,   i = 0; i < nc; i++ ) {
  entpx -= (px[i] >= SMALL) ? (px[i] * log10(px[i])) : (px[i] * log10(SMALL));
  for( j = 0; j < nc; j++ ) {
    tmp = px[i] * px[j];
    HXY1 -= (tmp >= SMALL) ? (p[i + j * nc] * log10(tmp)) : (p[i + j * nc] * log10(SMALL));
    HXY2 -= (tmp >= SMALL) ? (tmp * log10(tmp)) : (tmp * log10(SMALL));
  }
}
//    FIXME: why is the following negative (if fabs removed)?
f[IMC1] = (entpx != 0) ? ( fabs(f[ENT] - HXY1) / entpx) : 0.0;
tmp = 1.0 - exp( -2.0 * (HXY2 - f[ENT]));
f[IMC2] = ( tmp >= 0 ) ? sqrt(tmp) : 0.0;


ind_values.push_back(f);
}

// Store values

delete[] px;
delete[] Pxpy;
delete[] Pxmy;
return;
}

	
		 //**************************************************//

};

#endif
