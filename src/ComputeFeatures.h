#ifndef _COMPUTE_FEATURES_
#define _COMPUTE_FEATURES_

#include <iostream>
#include <map>
#include <bitset>
#include <opencv2/opencv.hpp>
#include "PModule.h"
#include "MType.h"
#include "Action.h"
#include "utils.h"
#include "FeatureCalculator.h"
#include "Writer.h"
#include <math.h>

using namespace cv;
using namespace std;



class ComputeFeatures:
	public PModule
{
	protected:

		typedef  void(ComputeFeatures::*Function)(vector<MType *>,unsigned int pid); // function pointer type
		static map<string, Function > tFunc;
	    static FeatureCalculator *featcalc;
		
		

	public:
		static Writer *featureWriter;
		static map<string, int> eMap;
		enum FILTER_OPTS{ SIZE, DIAMETER};
	ComputeFeatures(void)
	{
	 tFunc["BASIC"] = &ComputeFeatures::_basicfeatures;
	 tFunc["SHAPE"] = &ComputeFeatures::_shapefeatures;
	 tFunc["FILTER OBJECTS"] = &ComputeFeatures::_filterobjects; 
	 tFunc["MOMENT"] = &ComputeFeatures::_momentfeatures;
	 tFunc["HARALICK"] = &ComputeFeatures::_haralickfeatures; 
	 tFunc["ALL FEATURES"] = &ComputeFeatures::_allfeatures; 
	// tFunc["NUMBER OF NEIGHBORS"] = &ComputeFeatures::_nneigh;
	 featcalc =  FeatureCalculator::getInstance();
	 
	 eMap["SIZE"] = SIZE;
	 eMap["DIAMETER"] = DIAMETER;
	 

	}
	~ComputeFeatures(void)
	 {

		// cout<< "COMPUTEFEATURES finished" <<endl;
		delete featureWriter;
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

/****************************************************************************
 * BASIC FEATURES
 * ----------------------------
 *  b_mean: mean intensity
 *  b_sd: standard deviation intensity
 *  b_mad: mad intensity
 *  b_median : median intensity
 *  b_quant: quantile intensity
 *
 ***************************************************************************/
	void _basicfeatures(std::vector<MType *> parValues, unsigned int pid)
	{
     
	 // append date 0
	 // append name 1
	 // foutput name 2
     string foutput	= (dynamic_cast<MStringType*>(parValues[2]))->getValue(); 
	 string objects	= (dynamic_cast<MIdentifierType*>(parValues[3]))->getValue(pid);  // imageName (output>
    //  bool mad 4 // feature
    //  bool mean 5 // feature
	//  bool quantiles 6 // feature
	 string ref	= (dynamic_cast<MIdentifierType*>(parValues[7]))->getValue(pid);  // imageName (output>
	 bool save_ind = (dynamic_cast<MBoolType*>(parValues[8]))->getValue(); // feature
	//  bool sd 9 // feature	

	 string absref = (dynamic_cast<MIdentifierType*>(parValues[7]))->getValueAsString();
	 __basicfeatures(objects.c_str(),ref.c_str(),foutput.c_str(),save_ind);

	}


void __basicfeatures(const char* objects, const char* ref,const char* absref, bool save_ind)
{
		vloP *_objvec = pool->getlObj(objects);
		Mat *_ref = pool->getImage(ref);
		vector<double> gvals;
		vector<vector<double>> indvals;
		bitset<4> options(string("0000"));
		FeaturesPipe *fp;
		// The absolute reference of the image is used for indexing the file to which write the features
		fp = this->featureWriter->getFeaturePipe(absref);

		for (size_t i = 0; i < 4; ++ i) {
				 if(fp->_options.test(i)) options |= options.set(i);
			}
		
		this->featcalc->basic(*_objvec, *_ref, options, gvals, indvals);
		this->featureWriter->write(gvals,indvals,absref,save_ind);
		
	}

/****************************************************************************
 * SHAPE FEATURES
 * ----------------------------
 *  area : in px
 *  perimeter : in px
 *  radius: mad in px
 *  radius.min : in px
 *  radius.max: in px
 *
 ***************************************************************************/
	void _shapefeatures(std::vector<MType *> parValues, unsigned int pid)
	{
     // append date 0
	 // append name 1
	 //  bool area 2 // feature
	 // foutput name 3
		 string foutput	= (dynamic_cast<MStringType*>(parValues[3]))->getValue(); 
		string objects	= (dynamic_cast<MIdentifierType*>(parValues[4]))->getValue(pid);  // imageName (output>
    //  bool perimeter 5// feature
	//  bool radius (6) 
		string reference_image	= (dynamic_cast<MIdentifierType*>(parValues[7]))->getValue(pid);  // imageName (output>
	 //  bool roundness (8) 
	 bool save_ind = (dynamic_cast<MBoolType*>(parValues[9]))->getValue(); // feature
	
	 __shapefeatures(objects.c_str(),reference_image.c_str(),foutput.c_str(),save_ind);

	}


void __shapefeatures(const char* objects, const char* ref,const char *absref, bool save_ind)
{
		vloP *_objvec = pool->getlObj(objects);
		Mat *_ref = pool->getImage(ref);
		vector<double> gvals;
		vector<vector<double>> indvals;
		bitset<4> options(string("0000"));
		FeaturesPipe *fp;
		fp = this->featureWriter->getFeaturePipe(absref);

		for (size_t i = 4; i < 8; ++ i) {
				 if(fp->_options.test(i)) options |= options.set(i-4);
			}
		
		vloP contours;
		getContours(*_objvec,contours,_ref->rows,_ref->cols);
		this->featcalc->shape(*_objvec,contours, *_ref, options, gvals, indvals);
		this->featureWriter->write(gvals,indvals,absref,save_ind);
		
	}


	/****************************************************************************
 * MOMENT FEATURES
 * ----------------------------
 * cx: center of mass x (in pixels)
 * cy: center of mass y (in pixels)
 * major axis: elliptical fit major axis (in pixels)
 * minor axis: elliptical fit minor axis (in pixels)
 * eccentricity: elliptical eccentricity defined by sqrt(1-majoraxis^2/minoraxis^2). Circle eccentricity is 0 and straight line eccentricity is 1.
 * theta: object angle (in radians)
 *
 ***************************************************************************/
	void _momentfeatures(std::vector<MType *> parValues, unsigned int pid)
	{
     // append date 0
	 // append name 1
	//  bool axis 2 // feature
	//  bool centroid 3 //feature
	//  bool eccentricity 4 // feature
	//  foutput 5
	 string foutput	= (dynamic_cast<MStringType*>(parValues[5]))->getValue(); 
	 string objects	= (dynamic_cast<MIdentifierType*>(parValues[6]))->getValue(pid);  // imageName (output>
	 string reference_image	= (dynamic_cast<MIdentifierType*>(parValues[7]))->getValue(pid);  // imageName (output>
	 bool save_ind = (dynamic_cast<MBoolType*>(parValues[8]))->getValue(); // feature
	 
	 bool woref = false;
	 if(!reference_image.compare("NO_REFERENCE")!=0) 
	 {
			  woref=true;
	 }

	 __momentfeatures(objects.c_str(),reference_image,foutput.c_str(),save_ind, woref);

	}


void __momentfeatures(const char* objects, string &ref, const char* absref,bool save_ind,bool woref)
{
		vloP *_objvec = pool->getlObj(objects);
		Mat *_ref;
		if(!woref) 
		{
			_ref = pool->getImage(ref.c_str());
		}
		
		vector<double> gvals;
		vector<vector<double>> indvals;
		bitset<4> options(string("0000"));
		FeaturesPipe *fp;
		fp = this->featureWriter->getFeaturePipe(absref);

		for (size_t i = 8; i < 12; ++ i) {
				 if(fp->_options.test(i)) options |= options.set(i-8);
			}
		
		this->featcalc->moment(*_objvec, *_ref, options, gvals, indvals, woref);
		this->featureWriter->write(gvals,indvals,absref,save_ind);
		
}



/****************************************************************************
 * HARALICK TEXTURE FEATURES
 * ----------------------------
 *  Haralick features are not treated like the previous features.
 *  Since they are texture features, there is no need to store the mean and general values.
 *  Otherwise, the user can import the data in R or Matlab and do it by himself.
 *  Secondly, the file is stored independently because the concept of bins and scales.
 *  Different bins can give different results, and also, the more scales we use, 
 *  the more data we have.
 *  Adding this to the previous features file gives us an enormous file, so at the end
 *  is more comfortable having the Haralick features in an independent file.
 *  It also gives a framework (using the featureWriter->writeNew method) for a new 
 *  features added in the feature.
 * angular second moment (ASM), 
 * inverse diff mom      (IDM),
 * difference Variance  (DVA)
 * difference entropy   (DEN)
 * entropy               (ENT)
 * correlation           (COR) 
 * contrast              (CON) 
 * variance             (VAR)
 * sum average          (SAV)
 * sum entropy          (SEN)
 * sum variance         (SVA) 
  "h.asm", "h.con", "h.cor", "h.var", "h.idm", "h.sav", "h.sva", 
  "h.sen", "h.ent", "h.dva", "h.den", "h.f12", "h.f13"
 ***************************************************************************/
	void _haralickfeatures(std::vector<MType *> parValues, unsigned int pid)
	{
     // append date 0
	 // append name 1
	 int bins = (dynamic_cast<MIntType*>(parValues[2]))->getValue(pid);
	 string fname = (dynamic_cast<MStringType*>(parValues[3]))->getValue();
	 string objects	=  (dynamic_cast<MIdentifierType*>(parValues[4]))->getValue(pid); //
	 string reference_image	= (dynamic_cast<MIdentifierType*>(parValues[5]))->getValue(pid);
	 string scales = (dynamic_cast<MStringType*>(parValues[6]))->getValue();
	 
	 // Obtain number of scales
	 vector<int> _scales;
	 char * pch;
	 pch = strtok((char*)scales.c_str(),",");
	 while (pch != NULL)
	{
		_scales.push_back(atoi(pch));
		// cout<<pch<<endl;
		pch = strtok (NULL, ",");
	}
	
	 __haralickfeatures(objects.c_str(),reference_image.c_str(),_scales,bins,fname);

	}


void __haralickfeatures(const char* objects, const char* ref,vector<int> scales,int bins,string fname)
{

		vloP *_objvec = pool->getlObj(objects);
		Mat *_ref = pool->getImage(ref);
		
		vector<vector<double>> indvals;
		vector<double> cm;

		 const char* args[] = {"h.asm", "h.con", "h.cor", "h.var", "h.idm", "h.sav", "h.sva",
				               "h.sen", "h.ent", "h.dva", "h.den", "h.f12", "h.f13"};
		 vector<string> header(args, args + 13);

		// Since my file is independent, I don´t need to count generally.
		const char* filename;
		// get filename

		// save them
		string header_f;

		int *data;
	    int nx = _ref->cols;
		int ny = _ref->rows;
		// for each scale


		// Generate matrix with objects (integers)

		data = new int[nx * ny];
		for(int  i = 0; i < nx*ny; i++ ) data[i]=0;
		Point pn;
		int count = 0;
		// Now create a matrix with the objects, in this case of integers.
		for(vloP::iterator itO = _objvec->begin(); itO!=_objvec->end(); ++itO)
		{
			 count++;
			 for(loP::iterator itnO = (*itO).begin(); itnO!=(*itO).end(); ++itnO)
			 {
					pn = (*itnO);
					data[((pn.y)*nx+(pn.x))]= count;
			  }
		}
		for(vector<int>::iterator it = scales.begin(); it!=scales.end(); ++it)
		{
				// Generate header
						//  h.asm_s1 for scale 1, h.asm_s2 for scale 2,
						// filename_s = filename + "_s1"
						header_f="N_Obj \t";
						std::string suff;
						suff.append("_s");
						int val=*it;
						stringstream ss;
						ss << val;
						string s = ss.str();
						suff.append(s);
					    for(unsigned int i=0;i<header.size();i++)
					    {
						header_f.append(header[i]);
						header_f.append(suff);
						header_f.append("\t");
					    }

				// Calculate subset of scale indexes
				if((*it)>1)
				{

					// calculate sequence of indexes
					//  and remove the ones not belonging in the objects and the reference matrix
					//  That will give us the new scaled matrix (1 pixel of each "scale")
					vector<int> indexes_x;
					vector<int> indexes_y;
					for(int i=0;i<_ref->cols;i+=(*it)) indexes_x.push_back(i);
					for(int i=0;i<_ref->rows;i+=(*it)) indexes_y.push_back(i);

					int nx2 = indexes_x.size();
					int ny2 = indexes_y.size();
					Mat refcp(ny2,nx2,CV_64FC1);
					Mat temp;
					_ref->convertTo(temp,CV_64FC1);

					int *data2;
					data2 = new int[nx2 * ny2];
					int i,j;
					int x2,y2;
					i=0;
					j=0;
					for(auto ity = indexes_y.begin(); ity!=indexes_y.end(); ++ity,++i)
					{
					    j=0;
						for(auto itx = indexes_x.begin(); itx!=indexes_x.end(); ++itx,++j)
						{
							y2 =*ity;
							x2 =*itx;
							refcp.at<double>(i,j) = temp.at<double>(y2,x2);
							data2[((i)*nx2+j)]=data[((y2)*nx+x2)];
						}
					}

					//
					cm.clear();

					this->featcalc->haralickMatrix(data2,_objvec->size(), refcp, bins, cm);
					featureWriter->dontCountFile(fname.c_str());
					delete[] data2;
			}
			else
			{
			// create haralick co-occurrence matrix
			this->featcalc->haralickMatrix(data,_objvec->size(), *_ref, bins, cm);
			
			}
			// now calculate features
			indvals.clear();
			this->featcalc->haralickFeatures(cm, indvals,bins,_objvec->size());
			string endf;
			endf.append(suff);
			endf.append(".dat");
			this->featureWriter->writeNew(header_f.c_str(),fname,endf.c_str(),indvals);

		}
		// end loop

		delete[] data;

	}



/*****************************************************************************************************
*  FILTER OBJECTS
*  Objects can filtered by area (size)
*  or by diameter. The diameter is based in the radius.
* 
*******************************************************************************************************/
void _filterobjects(std::vector<MType *> parValues,unsigned int pid)
{
	const char* opt =  (dynamic_cast<MStringType*>(parValues[0]))->getValue(); // by
	string load_objects = (dynamic_cast<MIdentifierType*>(parValues[1]))->getValue(pid);
	int max = (dynamic_cast<MIntType*>(parValues[2]))->getValue(pid); 
	int min = (dynamic_cast<MIntType*>(parValues[3]))->getValue(pid); 
	MIdentifierType* out = dynamic_cast<MIdentifierType*>(parValues[4]);
	string save_objects = out->getValue(pid);
	
	int fopt;
	if(strcmp(opt,"SIZE")==0) fopt = FILTER_OPTS::SIZE;
	else fopt= FILTER_OPTS::DIAMETER;

	__filterobjects(load_objects.c_str(),save_objects.c_str(),min,max,fopt);

	out->refresh(pid);
}

void __filterobjects(const char *load_objects, const char *save_objects, int &min,int &max,int &opt)
{
	 ut::Trace tr=ut::Trace("FILTER OBJECTS",__FILE__);
	vloP *obj1 = pool->getlObj(load_objects);
	vloP obj2;
	if(opt == FILTER_OPTS::SIZE)
	{
		    if(max==0) max = INT_MAX;		

			for (vloP::iterator it = obj1->begin(); it!=obj1->end(); ++it)
					{
						if(((*it).size()>min) && ((*it).size()<=max)) obj2.push_back(*it); 
					}
						
	}
	else
	{
		if(opt == FILTER_OPTS::DIAMETER) // heuristic approach
		{
			int area_min = ceil(min*min*FeatureCalculator::PI);
			int area_max = ceil(max*max*FeatureCalculator::PI);
			if(max==0) area_max = INT_MAX;	

			for (vloP::iterator it = obj1->begin(); it!=obj1->end(); ++it)
					{
						if(((*it).size()>area_min) && ((*it).size()<area_max)) obj2.push_back(*it); 
					}
		}
	}
	tr.message("Num. of objects initially: ",obj1->size());
	tr.message("Num. of objects after filter: ",obj2.size());
	tr.message("Total eliminated: ",(obj1->size()-obj2.size()));
	pool->storelObj(obj2,save_objects);
}

/*****************************************

COMPUTE ALL FEATURES

********************************************/
	void _allfeatures(std::vector<MType *> parValues, unsigned int pid)
	{

	 // append date 0
	 // append name 1
	 bool basic = (dynamic_cast<MBoolType*>(parValues[2]))->getValue(); // feature
	 string foutput	= (dynamic_cast<MStringType*>(parValues[3]))->getValue(); 
	 bool haralick = (dynamic_cast<MBoolType*>(parValues[4]))->getValue();
	 string objects	= (dynamic_cast<MIdentifierType*>(parValues[5]))->getValue(pid);  // loadObjects
	 bool moment = (dynamic_cast<MBoolType*>(parValues[6]))->getValue();
	 string reference_image	= (dynamic_cast<MIdentifierType*>(parValues[7]))->getValue(pid);  // imageName (output>
	 bool save_ind = (dynamic_cast<MBoolType*>(parValues[8]))->getValue(); // feature
	 bool shape = (dynamic_cast<MBoolType*>(parValues[9]))->getValue();

	 __allfeatures(objects.c_str(),reference_image,foutput.c_str(),save_ind,basic,shape,moment,haralick);

	}


void __allfeatures(const char* objects, string &ref, const char* foutput,bool save_ind, bool basic, bool shape, bool moment, bool haralick)
{
	if(basic)
	{
		__basicfeatures(objects,ref.c_str(),foutput,save_ind);
	}
	if(shape)
	{
	__shapefeatures(objects,ref.c_str(),foutput,save_ind);
	}
	if(moment)
	{
	__momentfeatures(objects,ref,foutput,save_ind,false);
	__momentfeatures(objects,ref,foutput,save_ind,true);
	
	}
	if(haralick)
	{
		 vector<int> _scales;
		 _scales.push_back(1);
		 _scales.push_back(2);
		 string n_name("HAR_");
		 n_name.append(foutput);
		 __haralickfeatures(objects,ref.c_str(),_scales,8,n_name.c_str());
	}

	return;
}

/* CHAIN CODE Algorithm
*   used direction-to-code convention is:       3  2  1
%                                                \ | /
%                                             4 -- P -- 0
%                                                / | \
%                                               5  6  7
%   
%   and in terms of deltax,deltay if next pixel compared to the current:
%   --------------------------
%   | deltax | deltay | code |
%   |------------------------|
%   |    0   |   +1   |   2  |
%   |    0   |   -1   |   6  |
%   |   -1   |   +1   |   3  |
%   |   -1   |   -1   |   5  |
%   |   +1   |   +1   |   1  |
%   |   +1   |   -1   |   7  |
%   |   -1   |    0   |   4  |
%   |   +1   |    0   |   0  |
%   --------------------------
*/
/*
enum DIR{ RIGHT,DIAGUPR, UP, DIAGUPL, LEFT, DIAGDOWNL,DOWN, DIAGDOWNR };

typedef struct chainO { 
   Point inipos; 
   vector<int> chain;
   chainO(Point p):inipos(p){} // Constructor
} Chain;



struct comp {
	bool operator() (Point p1,Point p2) { return (p1.x < p2.x);}
			} compareJ;

typedef vector<Chain> vChains;

 void getContours(vloP &objects_in,unsigned int rows, unsigned int cols)
{
	
	// set init point 
	vChains vC;
	for (vloP::iterator it = objects_in.begin(); it!=objects_in.end(); ++it)
	{
		sort((*it).begin(),(*it).end(),compareJ);
		Chain *_chain = new Chain((*it)[0]);
		_chain->chain.push_back(DIR::RIGHT);

		for(int i=1;i<(*it).size();i++)
		{
			Point p=(*it)[i];
			if(p.x-1 ==  
		}
	}

} */

};

#endif // _FEATURE_SELECTION_
