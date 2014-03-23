#ifndef _CHECKER_
#define _CHECKER_
#include <cctype>
#include <vector>
#include <sstream>
#include <regex>
#include <cctype>
#include <limits>
#include <stdlib.h>
#include <stdio.h>
#include <boost/regex.hpp>
// #include <boost/pattern_except.hpp>

#include "Action.h"
#include "FlowMap.h"
#include "FileProcessing.h"
#include "ImageProcessing.h"
#include "SegmentationProcessing.h"
#include "ComputeFeatures.h"
#include "utils.h"
#include "PoolManager.h"



#define TOTAL_MODULES 8
#define TOTAL_FUNCTIONS_FP 10
#define TOTAL_FUNCTIONS_IP 21
#define TOTAL_FUNCTIONS_SP 8
#define TOTAL_FUNCTIONS_CF 6

#define MAX_PARAMS 15

using namespace ut;


class Checker
{
public:
	typedef  bool(Checker::*SFunction)(Action *action,string &error); // function pointer type
	static map<string, SFunction > cFunc;
	FlowMap myMap;
	shared_ptr<PoolManager> pool;
	static const char* Modules[ TOTAL_MODULES ];
	static const char*  ParamsFP[TOTAL_FUNCTIONS_FP][MAX_PARAMS];
	static const char*  ParamsIP[TOTAL_FUNCTIONS_IP][MAX_PARAMS];
	static const char*  ParamsSP[TOTAL_FUNCTIONS_SP][MAX_PARAMS];
	static const char*  ParamsCF[TOTAL_FUNCTIONS_CF][MAX_PARAMS];
	static const char*  ParamsGN[1][MAX_PARAMS];
	string lastload;
	int imwidth;  // Last width and height from image
	int imheight;
	
	Checker(void)
	{
		 lastload = "";
		 imwidth=0;
		 imheight=0;
	 pool = PoolManager::getInstance();
	 
	  cFunc["CLEAN"] = &Checker::_good;
	 
	 cFunc["LOAD FILE"] = &Checker::_checkLoadFile;
	 cFunc["NORMALIZE"] = &Checker::_checkNormalize;
	 cFunc["SHOW"] = &Checker::_checkShow;
	 cFunc["CROP IMAGE"] = &Checker::_checkCropImage;
	 cFunc["WRITE FILE"] = &Checker::_checkWriteFile;
	 cFunc["PAINT OBJECTS"] = &Checker::_checkPaintObjects;
	 cFunc["BLEND"] = &Checker::_checkBlend;
	 cFunc["CONVERT"] =  &Checker::_checkConvert;

	 cFunc["THRESHOLD"] = &Checker::_checkThreshold;
	 cFunc["MAKE BRUSH"] = &Checker::_checkMakeBrush;
	 cFunc["ERODE"] = &Checker::_checkMOP;
	 cFunc["DILATE"] = &Checker::_checkMOP;
	 cFunc["MORPHOLOGICAL OPERATION"] = &Checker::_checkMOP2;
	 cFunc["FIND CONTOURS"] = &Checker::_checkFindContours;
	 cFunc["FILLHULL"] = &Checker::_checkFillHull;
	 cFunc["CANNY"] = &Checker::_checkCanny;
	 cFunc["BOUNDARY EXTRACTION"]= &Checker::_checkBoundaryExtraction;
	 cFunc["GAUSSIAN BLUR"] = &Checker::_checkGaussianBlur;
	 cFunc["SOBEL"] = &Checker::_checkSobel;
	 cFunc["BLUR"] =  &Checker::_checkBlur;
	 cFunc["LAPLACIAN"] =  &Checker::_checkLaplacian;
	 cFunc["ADAPTIVE THRESHOLD"] =  &Checker::_checkAdaptiveThreshold;
	 cFunc["FILTER2D"] =  &Checker::_checkFilter2D;
	 cFunc["CREATE KERNEL"] =  &Checker::_checkCreateKernel;
	 cFunc["RGB COMPOSITE"] =  &Checker::_checkRGBComposite;
	 cFunc["FLOODFILL"] =  &Checker::_checkFloodfill;
	 cFunc["SCALAR OPERATION"] =  &Checker::_checkSop;
	 cFunc["MATRIX OPERATION"] =  &Checker::_checkMatOp;
	 cFunc["MATRIX TO SCALAR OPERATION"] =  &Checker::_checkMtsop;
	 cFunc["GBLOB"] =  &Checker::_checkGBlob;
	 
	 cFunc["WATERSHED"] = &Checker::_checkWatershed;
	 cFunc["LABEL"] =  &Checker::_checkLabel;
	 cFunc["PROPAGATE"] =  &Checker::_checkPropagate;
	 cFunc["SPLIT BY WATERSHED"] =  &Checker::_checkSplitWatershed;
	 cFunc["SPLIT BY OTSU"] =  &Checker::_checkSplitByOtsu;
	 cFunc["SPLIT BY FRAGMENTATION"] =  &Checker::_checkSplitByFragmentation;
	 cFunc["MERGE OBJECTS"] =  &Checker::_checkMerge;
	 cFunc["HOUGH TRANSFORM"] =  &Checker::_checkHough;

	 cFunc["FILTER OBJECTS"] =  &Checker::_checkFilterObjects;
	 cFunc["BASIC"] =  &Checker::_checkBasic;
	 cFunc["SHAPE"] =  &Checker::_checkShape;
	 cFunc["MOMENT"] =  &Checker::_checkMoment;
	 cFunc["ALL FEATURES"] =  &Checker::_checkAllFeatures;
	 cFunc["HARALICK"] =  &Checker::_checkHaralick;

	 utils::initialise();
	}
	~Checker(void);

/*************************************************************
*  checkModule()
*  Checks if a Module exists
* \param string with the module name.
* \return true if module name exists
***********************************************************	*/
	bool checkModule(string moduleName)
	{
	
	// Check that  module exists
		for (int i=0; i<TOTAL_MODULES; i++)
		{
			if(moduleName.compare(Checker::Modules[i])==0) return true;	
		}
        return false;
	}

/*************************************************************
*  checkFunction()
*  Checks if a function exists
* \param string with the function name.
* \return true if function name exists
***********************************************************	*/
	bool checkFunction(string moduleName, string functionName)
	{
	    int i,top;
		for (i=0; i<TOTAL_MODULES; i++)
		{
			if(moduleName.compare(Checker::Modules[i])==0) break;	
		}

		const char** func;
		switch(i){
		case(0): func= Checker::ParamsFP[0]; top=TOTAL_FUNCTIONS_FP; break;
		case(1): func= Checker::ParamsIP[0]; top=TOTAL_FUNCTIONS_IP; break;
		case(2): func= Checker::ParamsSP[0]; top=TOTAL_FUNCTIONS_SP; break;
		case(3): func= Checker::ParamsCF[0]; top=TOTAL_FUNCTIONS_CF; break;
		case(4): func= Checker::ParamsGN[0]; top=1; break;
		default: break;
		} 

	// Check that function exists
		for (int j=0; j<MAX_PARAMS*top; j+=MAX_PARAMS)
		{
			if(functionName.compare(func[j])==0) return true;	
		}

		return false;	
	}

	
/*************************************************************
*  printModuleFunctions(string moduleName)
*   Prints all the functions from a module
***********************************************************	*/

	void printModuleFunctions(string moduleName)
	{
	    int i;
		int top=0;
		for (i=0; i<TOTAL_MODULES; i++)
		{
			if(moduleName.compare(Checker::Modules[i])==0) break;	
		}
		
		const char** func;
		switch(i){
		case(0): func= Checker::ParamsFP[0]; top=TOTAL_FUNCTIONS_FP; break;
		case(1): func= Checker::ParamsIP[0]; top=TOTAL_FUNCTIONS_IP; break;
		case(2): func= Checker::ParamsSP[0]; top=TOTAL_FUNCTIONS_SP; break;
		case(3): func= Checker::ParamsCF[0]; top=TOTAL_FUNCTIONS_CF; break;
		default: break;
		} 

	// Print existent functions
	    for (int j=0; j<MAX_PARAMS*top; j+=MAX_PARAMS)
		{
			cout<<(func[j])<<endl;
		}
        return;	
	}	

	/******checkActionParameters********************
	*
	*  Checks semantically, for each function, if parameters are well created.
	*  It doesn�t check if parameters are string, doubles or integers.
	*  It assumes that the XML generator checks the type somewhere else.
	*  What it does is;
	*        Assess that is the right number of parameters
	*        Assess that any parameter tag is mispelled or missing
	*        Assess that parameters are not repeated or is any parameter missing
	* 
	********************************************/
	bool checkParameters(string MNAME, string FNAME, const char** nameParams,const char** params, int countP, string &error)
	{
	// Check Module and Function
	    int i;
		int top=0;
		for (i=0; i<TOTAL_MODULES; i++)
		{
			if(MNAME.compare(Checker::Modules[i])==0) break;	
		}
		// cout << "Found:"<< MNAME << endl;
		// cout << Checker::Modules[i]<< endl;
		// Assign corresponding set of functions with its parameters
		const char** func;
		switch(i){
		case(0): func= Checker::ParamsFP[0]; top=TOTAL_FUNCTIONS_FP; break;
		case(1): func= Checker::ParamsIP[0]; top=TOTAL_FUNCTIONS_IP; break;
		case(2): func= Checker::ParamsSP[0]; top=TOTAL_FUNCTIONS_SP; break;
		case(3): func= Checker::ParamsCF[0]; top=TOTAL_FUNCTIONS_CF; break;
		default: break;
		} 

		// cout << "Module "<<i<<" selected" << endl;
		// Search specific function
		int j;
	    for (j=0; j<MAX_PARAMS*top; j+=MAX_PARAMS)
		{
			if(FNAME.compare(func[j])==0) break;	
		}
		if(j>=top*MAX_PARAMS){
		  string error("-- Function ");
		  error+=FNAME;
		  error+=" not found. \n";
		  return false;
		}

		// now I locate one by one the parameters.
		int k=j+1;
		string stop("-");
		string nameparam,param;
	
		int totalP = 0;
		int k1=0;
		while(!stop.compare(func[k])==0 && k1<MAX_PARAMS)
		{		
			
			// search param in my list of parameters from the file
			nameparam = func[k];
			// cout << "Checking " << nameparam << endl;
			k1=0;
			while(k1<countP)
			{ 
			//	cout << "Par n "<< k1 << " of "<< params.size() << endl;
			//	cout << "Comparing against " << nameParams[k1] << endl;
				if(nameparam.compare(nameParams[k1])==0) break;
				k1++;
			}
			// If I pass the line, means I couldn�t find my parameter, is missing!
			if(k1==countP)
			{
				error= " -- Parameter  ";
				error+=nameparam;
				error+=" missing !!! \n";
				return false;
			}
			totalP++;
			k++;
		}

		// Check for extra parameters that can interfere with functionality
		if(totalP!=countP)  // If numbers doesn�t fit
		{
		   // check paramaters and shown the ones that doesn�t fit.
			for (int k2=0; k2<countP; k2++)
			{				
				int k3;
				for (k3=0; k3<totalP; k3++)
				{			
					nameparam = func[k3+j+1];
					if(nameparam.compare(nameParams[k2])==0) break;
				}
				if(k3==totalP)
				{
				error="Parameter  ";
				error+=nameParams[k2];
				error+=" does NOT exist. Remove it!!! \n";
				return false;
				}
			}
				// Check that parameters are not repeated  
				error="Repeated parameter. Remove it!!! \n";
				return false;
		}

       	return true;
	}	


	/***************************************************************************
	*  We need to check a list of input and outputs to check that
	*  there is coherence between the flow of input/output
	******************************************************************************/
	bool checkAction(Action &action,string &error)
	{
		// First check parameters
		error="";
		vector<string> params;
		for(size_t i=0;i<action.getParameters().size();i++)
		{
			params.push_back(action.getParameters()[i]->getTag());
		}

			// Search the right function for checking (must be implemented individually)
			std::map<std::string, SFunction >::iterator f=cFunc.find(action.getCurrentAction());
			if (f != cFunc.end())
			{
				return (this->*(f->second))(&action,error);
			}
			else
			{
			error="Checker function not recognised for ";
			error+= action.getCurrentAction();
			error+=" \n";
			return false;
			}
		
	}

	/*************************************************************
	*  checkRegExp()
	*  Checks possible errors with the regular expression according to the regexp library.
	*  http://www.cplusplus.com/reference/regex/regex_error/
	*  http://www.regular-expressions.info/posixbrackets.html
	*
	*****************************************************************/
	bool checkRegExp(const char* regexp,string &error)
	{
		try {
			 boost::regex myregex(regexp);
			} 
		catch (boost::regex_error& e)
			{
				error+="ERROR in REGEXP \n";
			    error+= "Invalid regular exception.\n";
				return false;
			}
	    return true;
	}

	bool checkType(std::map<string,int> parmap,string value,string &error)
	{
		if(parmap.find(value)==parmap.end())
		{
			error+="Type :";
			error+=value;
			error+=" does not exist.Check the manual!!!";
			return false;
		}
		else return true;
	}

	/***********************************************
	  Given the appropiate type, checks for a range.
	************************************************/
    bool checkRangeifType(std::map<string,int> parmap,string type,int expected_type, double param,double max, double min,string &error,bool strict=false)
	{
		
		std::map<string,int>::iterator it= (parmap.find(type));
		std::stringstream s;
		// cout<<it->second<< " "<<expected_type <<endl;
		if(it->second == expected_type){
		
		if(strict){
			if(param>max){
				   s<< "Parameter with value "<<param<<" passes the maximum allowed value : "<< max << " if  type  "<< type << " is selected." <<endl;
				   error+=s.str();
				   return false;
				  }
		 if(param<=min){
				   s<< "Parameter with value "<<param<<" is under the minimum allowed value :"<< min << " if  type  "<< type << " is selected." <<endl;
				   error+=s.str();
				   return false;
				  }
		}
		else
		{
		if(param>max){
				   s<< "Parameter with value "<<param<<" passes the maximum allowed value : "<< max << "if  type  "<< type << "is selected." <<endl;
				   error+=s.str();
				   return false;
				  }
		 if(param<min){
				   s<< "Parameter with value "<<param<<" is under the minimum allowed value :"<< min << "if  type  "<< type << "is selected." <<endl;
				   error+=s.str();
				   return false;
				  }
		
		}
		} 
	 return true;
	
	}
	

	bool checkRange(double param,double max, double min,string &error)
	{
	 if(param>max){
				   std::stringstream s;
				   s<< "Parameter with value "<<param<<" passes the maximum allowed value : "<< max <<endl;
				   error+=s.str();
				   return false;
				  }
	 if(param<min){
				   std::ostringstream s;
				   s<< "Parameter with value "<<param<<" is under the minimum allowed value :"<< min <<endl;
				   error+=s.str();
				   return false;
				  }
	 return true;
	
	}

	bool checkRange(int param,int max, int min,string &error)
	{
	 if(param>max){
				   std::stringstream s;
				   s<< "Parameter with value "<<param<<" passes the maximum allowed value : "<< max <<endl;
				   error+=s.str();
				   return false;
				  }
	 if(param<min){
				   std::ostringstream s;
				   s<< "Parameter with value "<<param<<" is under the minimum allowed value :"<< min <<endl;
				   error+=s.str();
				   return false;
				  }
	 return true;
	
	}

	bool checkColour(string colour,string &error)
	{

		// Check if HEX
		if(colour[0]=='#'){
			if(colour.substr(1,colour.length()-1).length()!=6)
				{
				  error += "Bad input in Hexadecimal color. Must be # followed by 6 hexadecimal digits.";
				  return false; 
				}

			 size_t pos;
			 pos=colour.find_first_not_of("#0123456789ABCDEFabcdef");
			 if(pos==std::string::npos) return true;
			 else
			 {
				  std::string buffer = std::to_string(5);
				  error += "Bad input in Hexadecimal color. Must be # followed by 6 hexadecimal digits. \n";
				  error +="Character in position ";
				  error += buffer;
				  error +=" is not \"0123456789ABCDEF\".";
				  return false; 
			 }
		}
	
	     // if RGB(0,0,0)
		// Check if RGB
		if(colour.substr(0,4).compare("RGB(")==0){
			if(colour[colour.length()-1]!=')')
			{
			 error+=" Bad colour expression: RGB expression non valid. Use RGB(x,x,x) where x is a number between 0 and 255.";
			 return false;
			}
			const char * pch;
			char tmp[3];
			pch = colour.c_str();
			int i,j;
			int c[3];

			j=0;
			size_t pos;
			pos= colour.find_first_not_of("RGB(1234567890,)");
			if(pos!=std::string::npos) 
			{
			  error+="COLOUR: Do not write letters inside an RGB format, please. How old are you? 5?\n";
			  error+="Use RGB(x,x,x) where x is a number between 0 and 255.";
			  return false;
			}
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
					if(c[j]>255 || c[j]<0)
					{
						error +="COLOUR: Bad colour value, ranges allowed: 0-255 for RGB";
						return false;
					}
					j++; 
					}
				 }

			return true;
		}

		// Check name
		for(size_t i=0;i<TOTAL_COLOR_NAMES;i++)
		{
			if(colour.compare(utils::colourNames[i])==0) return true;
		}
		error+= "Wrong colour name. You are only allowed to use: \n";
		for(size_t i=0;i<TOTAL_COLOR_NAMES;i++)
		{
			error+="  ";
		    error+=utils::colourNames[i];
		}
		error+="\n";
		return false;



	}

	 bool checkMatrix(map<string,int> param,string matrix,int kernelsize,string &error)
	 {
	    // I know is not a type, need to check if it is a number
		if(checkType(param,matrix,error)) return true;
		error="";
	    // if is not, then we check if it is a manual matrix	
		if(matrix.find_first_not_of(" \r\t\n0123456789.,;-")!=string::npos)
		{
			error+="MATRIX:Characters are not allowed in numerical matrices";
		    return false;
		}
		return true;

	 }
	 
	 bool checkArray(string marray,string &error)
	 {
	    // if is not, then we check if it is a manual matrix	
		if(marray.find_first_not_of(" \r\t\n0123456789,")!=string::npos)
		{
			error+="ARRAY:Characters are not allowed in numerical matrices";
		    return false;
		}
		return true;

	 }

	/****************************************************************************
	*    Check Kernel
	*
	*****************************************************************************/

	bool checkKernel(int ksize,string &error)
	{
		Mat *img = pool->getImage(lastload);
		this->imheight = img->rows;
		this->imwidth = img->cols;
		int max = (this->imwidth>this->imheight) ? imwidth:imheight;
		
		if(!checkRange(ksize,max,1,error="Parameter Kernel Size. \n")) return false;
		if(ksize%2==0) 
			{
				error+="Kernel size must be odd: 1,3,7,11,13,15...";
				return false;
			}
		return true;
	}
	bool checkEmpty(string param,string &error)
	{
	  size_t found;
	  found = param.find_last_not_of(" \n\r\t");
	  if (found != string::npos)
    		param.erase(found+1);
	  else
    	     param.clear();            
	  
	  if (param.empty()){ error+=" Empty value in  parameter\n"; return false; }
	  else return true;
	
	}
	/*************************************
	*		Checker Functions
	*
	*
	*
	 *************************************/
	bool _checkLoadFile(Action *action, string &error)
	{
		vector<MType *> params = action->getParameters();	
		long id=this->myMap.subscribeFunction(action->getCurrentAction());
	   // Remember: 0 is FILENAME
	
		this->lastload = (dynamic_cast<MIdentifierType*>(params[1]))->getValue(0);

		if (!this->myMap.addImageOutput(id,params[1]->getValue(),error)) return false;
		if (!checkRegExp(params[2]->getValue().c_str(),error="In REGEXP, input regular expression.")) return false;
		return true;
	}

	bool _checkNormalize(Action *action, string &error)
	{
		
		vector<MType *> params = action->getParameters();	
		long id=this->myMap.subscribeFunction(action->getCurrentAction());
		
		if(!myMap.addImageInput(id,params[1],error)) return false;
	    if(!myMap.addImageOutput(id,params[4],error)) return false;
		
		if(!checkType(FileProcessing::eMap,params[5]->getValue(),error)) return false;
		if(FileProcessing::eMap[params[5]->getValue()]==FileProcessing::CLAHE+FileProcessing::NORMTYPES)
		{
		if(!checkRange((dynamic_cast<MDoubleType*>(params[2]))->getValue(0),
							FLT_MAX,0.0,error)) return false;
		if(!checkRange((dynamic_cast<MDoubleType*>(params[3]))->getValue(0),
						   FLT_MAX,0.0,error)) return false;
		return true;
		}
		
		
		Mat *img = pool->getImage((dynamic_cast<MIdentifierType*>(params[1]))->getValue(0));
		
		double max = 0.0;
		switch(img->depth())
		{
		case(CV_8U): max=255; break;
		case(CV_16U): max=65535; break; 
		case(CV_32F): max=1.0; break;
		default: max=1.0; break;
		}
		
		        // First check we are between 1 and 0 for each parameter
				// then check that max is not smaller than min
				// then check that min is not bigger than max
			return(checkRange((dynamic_cast<MDoubleType*>(params[2]))->getValue(0),
							max,0.0,error)&
				checkRange((dynamic_cast<MDoubleType*>(params[3]))->getValue(0),
						   max,0.0,error) &
				checkRange((dynamic_cast<MDoubleType*>(params[2]))->getValue(0),
						   1.0,(dynamic_cast<MDoubleType*>(params[3]))->getValue(0),error) &		   
				checkRange((dynamic_cast<MDoubleType*>(params[3]))->getValue(0),
						   (dynamic_cast<MDoubleType*>(params[2]))->getValue(0),0.0,error));	
	}
	bool _checkShow(Action *action, string &error)
	{
		
		vector<MType *> params = action->getParameters();	
		long id=this->myMap.subscribeFunction(action->getCurrentAction());

		// Get maximum available resolution 
		int width =  utils::getMaxScreenWidth();
		int height = utils::getMaxScreenHeight();

	    return( myMap.addImageInput(id,params[1],error) &
			    checkRange((dynamic_cast<MIntType*>(params[3]))->getValue(0),
							width,0,error)&
			    checkRange((dynamic_cast<MIntType*>(params[0]))->getValue(0),
							height,0,error)
			);
				
	};

	bool _checkCropImage(Action *action, string &error)
	{
		
		vector<MType *> params = action->getParameters();	
		long id=this->myMap.subscribeFunction(action->getCurrentAction());
		return( myMap.addImageInput(id,params[3],error) &
				myMap.addImageOutput(id,params[4],error));

	};

	  bool _checkWriteFile(Action *action, string &error)
	{
		
		vector<MType *> params = action->getParameters();	
		long id=this->myMap.subscribeFunction(action->getCurrentAction());
			// save objects 
		if(!myMap.addImageInput(id,params[5],error="Parameter Input \n")) return false;
		if(!checkEmpty(params[4]->getValue(),error="Parameter FOUTPUT \n")) return false;
		if(!checkType(FileProcessing::eMap,params[2]->getValue(),error="Parameter FILE EXTENSION not allowed. Use PNG,JPG,JP2,PPM,PBM,PGM or TIFF.\n")) return false;// Valid extension
		if(!checkRangeifType(FileProcessing::eMap,params[2]->getValue(), FileProcessing::WRITEFORMATS::PNG,(dynamic_cast<MIntType*>(params[6]))->getValue(0),
							    9,0,error = "Parameter Compression Level not suitable for PNG. Must be between 1 and 9. \n",true)) return false;
		if(!checkRangeifType(FileProcessing::eMap,params[2]->getValue(), FileProcessing::WRITEFORMATS::JPG,
							   (dynamic_cast<MIntType*>(params[6]))->getValue(0),
							    100.0,0.0,error = "Parameter Compression Level not suitable for JPG. Must be between 1 and 100. \n",true))// Compressionlevel others */
									 return false;
		if(!checkRangeifType(FileProcessing::eMap,params[2]->getValue(), FileProcessing::WRITEFORMATS::JP2,
							   (dynamic_cast<MIntType*>(params[6]))->getValue(0),
							    100.0,0.0,error = "Parameter Compression Level not suitable for JPG2000. Must be between 1 and 100 \n",true))// Compressionlevel others */
									 return false; 
		return true;
	};



	// IMAGE PROCESSING
	bool _checkThreshold(Action *action, string &error)
	{
		
		vector<MType *> params = action->getParameters();	
		long id=this->myMap.subscribeFunction(action->getCurrentAction());
		return( myMap.addImageInput(id,params[1],error) &
				myMap.addImageOutput(id,params[3],error)&
				// Threshold can�t be bigger than max_threshold
				checkRange((dynamic_cast<MDoubleType*>(params[4]))->getValue(0),
							(dynamic_cast<MDoubleType*>(params[2]))->getValue(0),0.0,error)&
				checkType(ImageProcessing::eMap,params[5]->getValue(),error)						
				);

	};

	bool _checkMakeBrush(Action *action, string &error)
	{
		
		vector<MType *> params = action->getParameters();	
		long id=this->myMap.subscribeFunction(action->getCurrentAction());
		Mat *img = pool->getImage(lastload);
		this->imheight = img->rows;
		this->imwidth = img->cols;
		int min = (imwidth<imheight) ? imwidth:imheight;
		return( myMap.addImageOutput(id,params[4],error)&
				checkType(ImageProcessing::eMap,params[3]->getValue(),error)&
			    // Brush Size Range in X direction
				checkRange((dynamic_cast<MIntType*>(params[1]))->getValue(0),
							imwidth,1,error)&
				// Brush Size Range in Y direction
				checkRange((dynamic_cast<MIntType*>(params[2]))->getValue(0),
							imheight,1,error)&
				  // SIGMA if we use Gaussian brush, can�t be 0.
				  checkRangeifType(ImageProcessing::eMap,params[3]->getValue(),ImageProcessing::SHAPE::GAUSSIAN + ImageProcessing::BRUSHFLAGS,
								   (dynamic_cast<MDoubleType*>(params[5]))->getValue(0),
									 min,0.0,error="Parameter SIGMA for GAUSSIAN BRUSH\n",true)); 

	};

	
	bool _checkMOP(Action *action, string &error)
	{
		
		vector<MType *> params = action->getParameters();	
		long id=this->myMap.subscribeFunction(action->getCurrentAction());
		return( myMap.addImageOutput(id,params[4],error)&
			    myMap.addImageInput(id,params[1],error)&
				myMap.addImageInput(id,params[2],error)&
				checkRange((dynamic_cast<MIntType*>(params[3]))->getValue(0),
							32768,1,error)
				);

	};
	bool _checkMOP2(Action *action, string &error)
	{
		
		vector<MType *> params = action->getParameters();	
		long id=this->myMap.subscribeFunction(action->getCurrentAction());
		return( myMap.addImageOutput(id,params[5],error)&
			    myMap.addImageInput(id,params[1],error)&
				myMap.addImageInput(id,params[2],error)&
				checkType(ImageProcessing::eMap,params[4]->getValue(),error="Parameter OPERATION. Types available OPENING,CLOSING, GRADIENT,TOP HAT,BLACK HAT. \n" )&
				checkRange((dynamic_cast<MIntType*>(params[3]))->getValue(0),
							32768,1,error)
				);

	};

	bool _checkFindContours(Action *action, string &error)
	{
		
		vector<MType *> params = action->getParameters();	
		long id=this->myMap.subscribeFunction(action->getCurrentAction());
		
		// save objects 
		return( myMap.addImageInput(id,params[3],error)&
				myMap.addImageOutput(id,params[4],error)&
				myMap.addObjectOutput(id,params[6]->getValue(),error)&
				checkColour(params[2]->getValue(),error)& 
				checkType(ImageProcessing::eMap,params[5]->getValue(),error="Parameter Retrieval Mode")& // Valid retrieval mode
				checkType(ImageProcessing::eMap,params[0]->getValue(),error)& // Valid approximation value
				checkRange((dynamic_cast<MIntType*>(params[7]))->getValue(0),
							100,-1,error) // Thickness is a valid integer (limit 100)
				);

	};

  
	bool _checkFillHull(Action *action, string &error){
		
		vector<MType *> params = action->getParameters();	
		long id=this->myMap.subscribeFunction(action->getCurrentAction());
		return( myMap.addImageInput(id,params[1],error)&
				myMap.addImageOutput(id,params[2],error));

	};

	  
	bool _checkCanny(Action *action, string &error){
		
		vector<MType *> params = action->getParameters();	
		long id=this->myMap.subscribeFunction(action->getCurrentAction());

		Mat *img = pool->getImage((dynamic_cast<MIdentifierType*>(params[2]))->getValue(0));
		int type = img->depth();
		double _max;
		if(type==CV_8U) _max=255;
		if(type==CV_16U) _max=65535;
		if(type==CV_32F) _max = FLT_MAX;
		else _max = 100000;

		if(!myMap.addImageInput(id,params[2],error)) return false;
	    if(!myMap.addImageOutput(id,params[6],error)) return false;
		if(!checkRange((dynamic_cast<MDoubleType*>(params[1]))->getValue(0),
						_max,0.0,error="Parameter HIGH THRESHOLD. \n")) return false;
		if(!checkRange((dynamic_cast<MDoubleType*>(params[5]))->getValue(0),
					    _max,0.0,error="Parameter LOW THRESHOLD. \n")) return false; 
		if(!checkRange((dynamic_cast<MIntType*>(params[3]))->getValue(0),
					    7,1,error="Parameter KERNEL SIZE. \n")) return false; 
		if(!checkKernel((dynamic_cast<MIntType*>(params[3]))->getValue(0),error="Parameter KERNEL SIZE. \n")) return false;
	};

	bool _checkBoundaryExtraction(Action *action, string &error){
		
	vector<MType *> params = action->getParameters();	
	long id=this->myMap.subscribeFunction(action->getCurrentAction());
	return( myMap.addImageInput(id,params[2],error)&
			myMap.addImageOutput(id,params[3],error)&
			checkColour(params[1]->getValue(),error)& 
			checkRange((dynamic_cast<MIntType*>(params[4]))->getValue(0),
							1000,1,error) // Thickness is a valid integer (limit 100)
			
			);

	};


	bool _checkGaussianBlur(Action *action, string &error){
		
		vector<MType *> params = action->getParameters();	
		long id=this->myMap.subscribeFunction(action->getCurrentAction());

		if(!myMap.addImageInput(id,params[2],error)) return false;
	    if(!myMap.addImageOutput(id,params[3],error)) return false;
		if(!checkRange((dynamic_cast<MDoubleType*>(params[4]))->getValue(0),
						1000.0,0.0,error="Parameter SIGMA_X. \n")) return false;
		if(!checkRange((dynamic_cast<MDoubleType*>(params[5]))->getValue(0),
					    1000.0,0.0,error="Parameter SIGMA_Y. \n")) return false;	
		if(!checkKernel((dynamic_cast<MIntType*>(params[6]))->getValue(0),error="Parameter WIDTH. \n")) return false;
		if(!checkKernel((dynamic_cast<MIntType*>(params[1]))->getValue(0),error="Parameter HEIGHT. \n")) return false;

	
	};

	
	bool _checkSobel(Action *action, string &error){
		
		vector<MType *> params = action->getParameters();	
		long id=this->myMap.subscribeFunction(action->getCurrentAction());

		if(!myMap.addImageInput(id,params[1],error)) return false;
	    if(!myMap.addImageOutput(id,params[3],error)) return false;
		
		if(!checkType(ImageProcessing::eMap,params[2]->getValue(),error="Parameter OPERATION. Allowed types SOBEL and SCHARR. \n")) return false;
		if(!checkRange((dynamic_cast<MIntType*>(params[6]))->getValue(0),1000,0,error="Parameter X_ORDER. \n")) return false;
		if(!checkRange((dynamic_cast<MIntType*>(params[7]))->getValue(0),1000,0,error="Parameter Y_ORDER. \n")) return false;	
		if(((dynamic_cast<MIntType*>(params[6]))->getValue(0)+(dynamic_cast<MIntType*>(params[7]))->getValue(0))==0)
		{
		  error= " Both derivatives X_ORDER and Y_ORDER cannot be 0 at the same time";
		  return false;
		}
		// First check is a good kernel
		if(!checkKernel((dynamic_cast<MIntType*>(params[4]))->getValue(0),error="Parameter SIZE. \n")) return false;
		// Check is valid
		if(!checkRange((dynamic_cast<MIntType*>(params[4]))->getValue(0), 7,1,error="Parameter SIZE. Kernel allowed sizes 1,3,5,7.\n")) return false;	
		// Check if SIZE==1, dx and dy can�t be bigger than 2
		if((dynamic_cast<MIntType*>(params[4]))->getValue(0)==1)
		{
			if((((dynamic_cast<MIntType*>(params[6]))->getValue(0))>2)||((dynamic_cast<MIntType*>(params[7]))->getValue(0)>2))
			{
			 error="Only first and second derivatives allowed. \n";
			 return false;
			}
		}
		return true;
	};

     bool _checkBlur(Action *action, string &error){
		
		vector<MType *> params = action->getParameters();	
		long id=this->myMap.subscribeFunction(action->getCurrentAction());

		if(!myMap.addImageInput(id,params[2],error)) return false;
	    if(!myMap.addImageOutput(id,params[4],error)) return false;
		if(!checkType(ImageProcessing::eMap,params[3]->getValue(),error="Parameter OPERATION. Allowed types BOX, MEDIAN or DESPECKLE. \n")) return false;
		if(params[3]->getValue().compare("MEDIAN")==0)
		{
		if(!checkKernel((dynamic_cast<MIntType*>(params[5]))->getValue(0),error="Parameter WIDTH. \n")) return false;
		if(!checkKernel((dynamic_cast<MIntType*>(params[1]))->getValue(0),error="Parameter HEIGHT. \n")) return false;
		if((dynamic_cast<MIntType*>(params[5]))->getValue(0)!=(dynamic_cast<MIntType*>(params[1]))->getValue(0))
		{ error = "For MEDIAN type, WIDTH and HEIGHT must have same size. \n"; return false;}
		}
		else
		{
		if(!checkRange((dynamic_cast<MIntType*>(params[5]))->getValue(0),1000,0,error="Parameter WIDTH. \n")) return false;
		}
	};

	 	bool _checkLaplacian(Action *action, string &error){
		
		vector<MType *> params = action->getParameters();	
		long id=this->myMap.subscribeFunction(action->getCurrentAction());

		if(!myMap.addImageInput(id,params[1],error)) return false;
	    if(!myMap.addImageOutput(id,params[3],error)) return false;
		// First check is a good kernel
		if(!checkRange((dynamic_cast<MIntType*>(params[2]))->getValue(0),31,1,error="Parameter KERNEL SIZE. \n")) return false;
		if(!checkKernel((dynamic_cast<MIntType*>(params[2]))->getValue(0),error="Parameter KERNEL SIZE. \n")) return false;
		return true;
	};

	 bool _checkAdaptiveThreshold(Action *action, string &error){
		
		vector<MType *> params = action->getParameters();	
		long id=this->myMap.subscribeFunction(action->getCurrentAction());

		if(!myMap.addImageInput(id,params[3],error)) return false;
	    if(!myMap.addImageOutput(id,params[5],error)) return false;
		
		Mat *img = pool->getImage((dynamic_cast<MIdentifierType*>(params[3]))->getValue(0));
		
		double max = 0.0;
		switch(img->depth())
		{
		case(CV_8U): max=255; break;
		case(CV_16U): max=65535; break; 
		case(CV_32F): max=1.0; break;
		default: max=1.0; break;
		}
		if(!checkRange((dynamic_cast<MDoubleType*>(params[2]))->getValue(0),max,0.0,error="Parameter CONSTANT not allowed. \n")) return false;	
		if(!checkType(ImageProcessing::eMap,params[6]->getValue(),error="Parameter THRESHOLD TYPE. Allowed types BINARY BINARY_INV and THRESH_16 or BINARY_16. \n")) return false;	
		if(!checkType(ImageProcessing::eMap,params[4]->getValue(),error="Parameter METHOD. Allowed types GAUSSIAN or MEAN. \n")) return false;	
		if(!checkKernel((dynamic_cast<MIntType*>(params[1]))->getValue(0),error="Parameter BLOCK SIZE. \n")) return false;
		return true;
	};

	/********CREATE KERNEL ***********************/
	 bool _checkCreateKernel(Action *action, string &error){
		
		vector<MType *> params = action->getParameters();	
		long id=this->myMap.subscribeFunction(action->getCurrentAction());

	    if(!myMap.addImageOutput(id,params[4],error)) return false;
		if(!checkKernel((dynamic_cast<MIntType*>(params[2]))->getValue(0),error="Parameter BLOCK SIZE. \n")) return false;
		if(!checkMatrix(ImageProcessing::eMap,params[3]->getValue(),(dynamic_cast<MIntType*>(params[2]))->getValue(0),error="Parameter MATRIX. Allowed types EYE ONES ZEROS or number. \n")) return false;	
		if(!myMap.addFactorInput(id,params[1]->getValue(),error="FACTOR inexistent. Use DFACTOR or or rename your parameter.\n")) return false;
		return true;
	};

	

	 bool _checkFilter2D(Action *action, string &error){
		
		vector<MType *> params = action->getParameters();	
		long id=this->myMap.subscribeFunction(action->getCurrentAction());

		if(!myMap.addImageInput(id,params[1],error)) return false;
		if(!myMap.addImageInput(id,params[2],error)) return false;
	    if(!myMap.addImageOutput(id,params[3],error)) return false;
		
		return true;
	};

	 bool _checkRGBComposite(Action *action, string &error){
		
		vector<MType *> params = action->getParameters();	
		long id=this->myMap.subscribeFunction(action->getCurrentAction());
		if(params[1]->getValue().compare("NULL")==0 && params[2]->getValue().compare("NULL")==0 && params[4]->getValue().compare("NULL")==0)
		{
		  error+="RED, GREEN and BLUE cannot be null at the same time.";
		  return false;
		}

		if(!params[1]->getValue().compare("NULL")==0)
			if(!myMap.addImageInput(id,params[1],error="In parameter BLUE CHANNEL. \n")) return false;
		if(!params[2]->getValue().compare("NULL")==0)
			if(!myMap.addImageInput(id,params[2],error="In parameter GREEN CHANNEL. \n")) return false;
		if(!params[4]->getValue().compare("NULL")==0)
			if(!myMap.addImageInput(id,params[4],error="In parameter RED CHANNEL. \n")) return false;
	    if(!myMap.addImageOutput(id,params[3],error)) return false;
		
		return true;
	};

	 	

	 bool _checkWatershed(Action *action, string &error){
		
		vector<MType *> params = action->getParameters();	
		long id=this->myMap.subscribeFunction(action->getCurrentAction());

		if(!myMap.addImageInput(id,params[2],error="In parameter FOREGROUND. \n")) return false;
		if(!myMap.addImageInput(id,params[1],error="In parameter BACKGROUND. \n")) return false;
	    if(!myMap.addImageInput(id,params[3],error="In parameter INPUT. \n")) return false;
		if(!myMap.addImageOutput(id,params[4],error)) return false;
		if(!myMap.addObjectOutput(id,params[5]->getValue(),error)) return false;
		return true;
	};


	 
	 bool _checkFloodfill(Action *action, string &error){
		
		vector<MType *> params = action->getParameters();	
		long id=this->myMap.subscribeFunction(action->getCurrentAction());

		if(!myMap.addObjectInput(id,params[5]->getValue(),error="Parameter SEEDS : \n")) return false;
		if(!myMap.addImageInput(id,params[2],error)) return false;
	    if(!myMap.addImageOutput(id,params[4],error)) return false;
		if(!checkType(ImageProcessing::eMap,params[1]->getValue(),error="Parameter COLOUR. \n"))	
		   if(!checkColour(params[1]->getValue(),error)) return false;
	    if(!checkRange((dynamic_cast<MDoubleType*>(params[3]))->getValue(0),FLT_MAX,0.0,error="Parameter LOW_DIFF. \n")) return false;	
	    if(!checkRange((dynamic_cast<MDoubleType*>(params[6]))->getValue(0),FLT_MAX,0.0,error="Parameter UP_DIFF. \n")) return false;	
	
		return true;
	};


	 bool _checkBlend(Action *action, string &error){
		
		vector<MType *> params = action->getParameters();	
		long id=this->myMap.subscribeFunction(action->getCurrentAction());

		if(!myMap.addImageInput(id,params[3],error)) return false;
		if(!myMap.addImageInput(id,params[4],error)) return false;
	    if(!checkRange((dynamic_cast<MDoubleType*>(params[0]))->getValue(0),FLT_MAX,0.0,error="Parameter ALPHA_1. \n")) return false;	
		if(!checkRange((dynamic_cast<MDoubleType*>(params[1]))->getValue(0),FLT_MAX,0.0,error="Parameter ALPHA_2. \n")) return false;	
		 if(!myMap.addImageOutput(id,params[5],error)) return false;
		
		return true;
	};


    bool _checkPaintObjects(Action *action, string &error){
		
		vector<MType *> params = action->getParameters();	
		long id=this->myMap.subscribeFunction(action->getCurrentAction());
		
		// save objects 
		if ((params[3]->getValue().compare("BLANK_BINARY")!=0)&&(params[3]->getValue().compare("BLANK_COLOUR")!=0))
														   if (!myMap.addImageInput(id,params[3],error)) return false;
		if (!myMap.addObjectInput(id,params[4]->getValue(),error)) return false;
	    if (!myMap.addImageOutput(id,params[5],error)) return false;
		

		if(!checkRange((dynamic_cast<MIntType*>(params[8]))->getValue(0),
							INT_MAX,0,error="Parameter WIDTH.\n")) return false;
	    if(!checkRange((dynamic_cast<MIntType*>(params[2]))->getValue(0),
							INT_MAX,0,error="Parameter HEIGHT.\n")) return false;
		if(!checkColour(params[1]->getValue(),error="Parameter COLOUR.\n")) return false; 
		if(!checkRange((dynamic_cast<MIntType*>(params[7]))->getValue(0),
							1000,-1,error)) return false;  // Thickness is a valid integer (limit 1000)
		return true;
	};

	
	 bool _checkLabel(Action *action, string &error){
		
		vector<MType *> params = action->getParameters();	
		long id=this->myMap.subscribeFunction(action->getCurrentAction());

		if(!myMap.addImageInput(id,params[0],error)) return false;
		if(!myMap.addObjectOutput(id,params[1]->getValue(),error)) return false;
		return true;
	};

	 bool _checkPropagate(Action *action, string &error){
		
		vector<MType *> params = action->getParameters();	
		long id=this->myMap.subscribeFunction(action->getCurrentAction());

		if(!myMap.addObjectInput(id,params[4]->getValue(),error="SEEDS \n")) return false;
	    if(!myMap.addObjectOutput(id,params[3]->getValue(),error="SAVE_OBJ. \n")) return false;
		if(!myMap.addImageInput(id,params[0]->getValue(),error="INPUT. \n")) return false;
		if(!myMap.addImageInput(id,params[2],error="MASK. \n")) return false;
	    if(!checkRange((dynamic_cast<MDoubleType*>(params[1]))->getValue(0),FLT_MAX,0.0,error="Parameter LAMBDA. \n")) return false;	
	
		
		return true;
	};





	 bool _checkMoment(Action *action, string &error){
		
		vector<MType *> params = action->getParameters();	
		long id=this->myMap.subscribeFunction(action->getCurrentAction());

		if(!myMap.addObjectInput(id,params[6]->getValue(),error)) return false;
		if(params[7]->getValue().compare("NO_REFERENCE")!=0) 
			if(!myMap.addImageInput(id,params[7]->getValue(),error)) return false; //Reference
		if(!checkEmpty(params[5]->getValue(),error="Parameter FOUTPUT \n")) return false;
		return true;
	};

    	 bool _checkShape(Action *action, string &error){
		
		vector<MType *> params = action->getParameters();	
		long id=this->myMap.subscribeFunction(action->getCurrentAction());

		if(!myMap.addObjectInput(id,params[4]->getValue(),error)) return false;
		if(!myMap.addImageInput(id,params[7]->getValue(),error)) return false; //Reference
		if(!checkEmpty(params[3]->getValue(),error="Parameter FOUTPUT \n")) return false;
		return true;
	};


	 bool _checkBasic(Action *action, string &error){
		
		vector<MType *> params = action->getParameters();	
		long id=this->myMap.subscribeFunction(action->getCurrentAction());

		if(!myMap.addObjectInput(id,params[3]->getValue(),error)) return false;
		if(!myMap.addImageInput(id,params[7]->getValue(),error)) return false; //Reference
		if(!checkEmpty(params[2]->getValue(),error="Parameter FOUTPUT \n")) return false;
		return true;
	};


      bool _checkHaralick(Action *action, string &error){
		
		vector<MType *> params = action->getParameters();	
		long id=this->myMap.subscribeFunction(action->getCurrentAction());

		if(!myMap.addObjectInput(id,params[4]->getValue(),error)) return false;
		if(!myMap.addImageInput(id,params[5],error)) return false; //Reference
		if(!checkEmpty(params[3]->getValue(),error="Parameter FOUTPUT \n")) return false;
		if(!checkRange((dynamic_cast<MIntType*>(params[2]))->getValue(0),
							65536,2,error= "Parameter BINS. \n")) return false;  
		if(!checkArray((dynamic_cast<MStringType*>(params[6]))->getValue(),error= "Parameter SCALES. \n")) return false;  
		return true;
	};


	  bool _checkAllFeatures(Action *action, string &error){
		
		vector<MType *> params = action->getParameters();	
		long id=this->myMap.subscribeFunction(action->getCurrentAction());

		if(!myMap.addObjectInput(id,params[5]->getValue(),error)) return false;
		if(!myMap.addImageInput(id,params[7]->getValue(),error)) return false; //Reference
		if(!checkEmpty(params[3]->getValue(),error="Parameter FOUTPUT \n")) return false;
		return true;
	};


	  bool _checkGBlob(Action *action, string &error){
		
		vector<MType *> params = action->getParameters();	
		long id=this->myMap.subscribeFunction(action->getCurrentAction());

		if(!myMap.addImageInput(id,params[3],error="Parameter INPUT. \n")) return false;
	    if(!myMap.addImageOutput(id,params[4],error="Parameter OUTPUT. \n")) return false;
		if(!checkRange((dynamic_cast<MIntType*>(params[5]))->getValue(0),
							1000,1,error= "Parameter SEQUENCE. \n")) return false;  
		if(!checkKernel((dynamic_cast<MIntType*>(params[6]))->getValue(0),error="Parameter KERNEL SIZE. \n")) return false;
		return true;
	};



      bool _checkSplitWatershed(Action *action, string &error){
		
		vector<MType *> params = action->getParameters();	
		long id=this->myMap.subscribeFunction(action->getCurrentAction());

		if(!myMap.addImageInput(id,params[1],error="In parameter INPUT. \n")) return false;
		if(!myMap.addImageInput(id,params[3],error="In parameter REFERENCE. \n")) return false;
		if(!myMap.addObjectOutput(id,params[4]->getValue(),error="In parameter SAVE_OBJ. \n")) return false;
		if(!checkRange((dynamic_cast<MIntType*>(params[0]))->getValue(0),
							1000,0,error= "Parameter EXT. \n")) return false;  
		if(!checkRange((dynamic_cast<MDoubleType*>(params[5]))->getValue(0),
							1000.0,0.0,error= "Parameter TOL. \n")) return false;  
		if(!checkType(SegmentationProcessing::eMap,params[2]->getValue(),error="Parameter METHOD. \n"))	return false;
	
		return true;
	};


       bool _checkMtsop(Action *action, string &error)
	   {
		vector<MType *> params = action->getParameters();	
		long id=this->myMap.subscribeFunction(action->getCurrentAction());

		if(!myMap.addImageInput(id,params[0]->getValue(),error="In parameter INPUT. \n")) return false;
		if(!myMap.addFactorOutput(id,params[2]->getValue(),error="In parameter OUTPUT FACTOR. \n")) return false;
	    if(!checkType(ImageProcessing::eMap,params[1]->getValue(),error="Parameter OPERATION. \n"))	return false;
	    return true;
	   };

	    bool _checkSop(Action *action, string &error)
		{
		vector<MType *> params = action->getParameters();	
		long id=this->myMap.subscribeFunction(action->getCurrentAction());

		if(!myMap.addImageInput(id,params[2],error="In parameter INPUT. \n")) return false;
		if(!myMap.addImageOutput(id,params[5],error="In parameter OUTPUT. \n")) return false;
		if(!myMap.addFactorOutput(id,params[1]->getValue(),error="In parameter OUTPUT FACTOR. \n")) return false;
	    if(!checkType(ImageProcessing::eMap,params[4]->getValue(),error="Parameter OPERATION. \n"))	return false;
	    if(!checkRange((dynamic_cast<MIntType*>(params[3]))->getValue(0),
							INT_MAX,1,error= "Parameter ITERATIONS. \n")) return false;  
		return true;

	   };


      	bool _checkMatOp(Action *action, string &error){
		
		vector<MType *> params = action->getParameters();	
		long id=this->myMap.subscribeFunction(action->getCurrentAction());

		if(!myMap.addImageInput(id,params[1],error="In parameter INPUT1. \n")) return false;
		if(!myMap.addImageInput(id,params[2],error="In parameter INPUT2. \n")) return false;
		if(!myMap.addImageOutput(id,params[5],error="In parameter OUTPUT. \n")) return false;
	    if(!checkType(ImageProcessing::eMap,params[4]->getValue(),error="Parameter OPERATION. \n"))	return false;
	    if(!checkRange((dynamic_cast<MIntType*>(params[3]))->getValue(0),
							INT_MAX,1,error= "Parameter ITERATIONS. \n")) return false;  
		return true;

	   };

      	bool _checkSplitByOtsu(Action *action, string &error){
		
		vector<MType *> params = action->getParameters();	
		long id=this->myMap.subscribeFunction(action->getCurrentAction());

		if(!myMap.addImageInput(id,params[1],error="In parameter REFERENCE. \n")) return false;
		if(!myMap.addObjectInput(id,params[0]->getValue(),error="In parameter LOAD OBJECTS. \n")) return false;
		if(!myMap.addObjectOutput(id,params[2]->getValue(),error="In parameter SAVE OBJECTS. \n")) return false;
		return true;

	   };

		bool _checkSplitByFragmentation(Action *action, string &error){
		
		vector<MType *> params = action->getParameters();	
		long id=this->myMap.subscribeFunction(action->getCurrentAction());

		if(!myMap.addImageInput(id,params[3],error="In parameter REFERENCE. \n")) return false;
		if(!myMap.addObjectInput(id,params[0]->getValue(),error="In parameter BIG OBJECTS. \n")) return false;
		if(!myMap.addObjectInput(id,params[2]->getValue(),error="In parameter FRAG OBJECTS. \n")) return false;
		if(!myMap.addObjectOutput(id,params[4]->getValue(),error="In parameter SAVE OBJECTS. \n")) return false;
		 if(!checkRange((dynamic_cast<MDoubleType*>(params[1]))->getValue(0),
							DBL_MAX,0.0,error= "Parameter COEFFICIENT. \n")) return false;  
		return true;

	   };

		bool _checkMerge(Action *action, string &error)
		{
		
		vector<MType *> params = action->getParameters();	
		long id=this->myMap.subscribeFunction(action->getCurrentAction());

		if(!myMap.addImageInput(id,params[5],error="In parameter REFERENCE. \n")) return false;
		if(params[2]->getValue().compare("NULL")!=0)
				if(!myMap.addImageInput(id,params[2],error="In parameter MASK. \n")) return false;
		if(!myMap.addObjectInput(id,params[0]->getValue(),error="In parameter LOAD OBJECTS 1. \n")) return false;
		if(!myMap.addObjectInput(id,params[1]->getValue(),error="In parameter LOAD OBJECTS 2. \n")) return false;
		if(!myMap.addObjectOutput(id,params[4]->getValue(),error="In parameter OUTPUT. \n")) return false;
		if(!checkType(SegmentationProcessing::eMap,params[3]->getValue(),error="Parameter OPERATION. \n"))	return false;
		return true;

	   };


	 bool _checkFilterObjects(Action *action, string &error){
		
		vector<MType *> params = action->getParameters();	
		long id=this->myMap.subscribeFunction(action->getCurrentAction());
		
	    if(!myMap.addObjectInput(id,params[1]->getValue(),error="In parameter LOAD OBJECTS. \n")) return false;
		if(!myMap.addObjectOutput(id,params[4]->getValue(),error="In parameter SAVE OBJECTS. \n")) return false;
	    
		if(!checkType(ComputeFeatures::eMap,params[0]->getValue(),error="Parameter BY. \n"))	return false;
	    
		if((dynamic_cast<MIntType*>(params[2]))->getValue(0)!=0)
			{
             if(!checkRange((dynamic_cast<MIntType*>(params[3]))->getValue(0),
							(dynamic_cast<MIntType*>(params[2]))->getValue(0),0,error= "Parameter MINIMUM. \n")) return false;  
			 
			 if(!checkRange((dynamic_cast<MIntType*>(params[2]))->getValue(0),
				INT_MAX,(dynamic_cast<MIntType*>(params[3]))->getValue(0),error= "Parameter MAXIMUM. \n")) return false;
			}
		else{
			 if(!checkRange((dynamic_cast<MIntType*>(params[3]))->getValue(0),
							INT_MAX,0,error= "Parameter MINIMUM. \n")) return false;  
			}
	
		return true;
	};


	  bool _checkHough(Action *action, string &error){
		  vector<MType *> params = action->getParameters();	
		  long id=this->myMap.subscribeFunction(action->getCurrentAction());
		  if(!myMap.addImageInput(id,params[2],error="In parameter INPUT. \n")) return false;
	      if(!myMap.addObjectOutput(id,params[3]->getValue(),error="In parameter LINES. \n")) return false;
	      
		   // RHO
		    if(!checkRange((dynamic_cast<MDoubleType*>(params[7]))->getValue(0),800.0,1.0,error= "Parameter RHO. \n")) return false;
		   // THETA
			if(!checkRange((dynamic_cast<MDoubleType*>(params[8]))->getValue(0),360.0,1.0,error= "Parameter THETA. \n")) return false;
		  // THRESHOLD_INTERSECTIONS	
			if(!checkRange((dynamic_cast<MIntType*>(params[9]))->getValue(0),10000,1,error= "Parameter THRESHOLD_INTERSECTIONS. \n")) return false;
		  // PROBABILISTIC	
		  // MIN_LIN_LENGTH
			if(!checkRange((dynamic_cast<MDoubleType*>(params[5]))->getValue(0),5000.0,0.0,error= "Parameter MIN_LINE_LENGTH. \n")) return false;
		  // MAX_LINE_GAP
			if(!checkRange((dynamic_cast<MDoubleType*>(params[4]))->getValue(0),5000.0,0.0,error= "Parameter MAX_LINE_GAP. \n")) return false;
		 
			if(!checkColour(params[1]->getValue(),error="Parameter COLOUR.\n")) return false; 
			
	  }

	  bool _checkConvert(Action *action, string &error){
		  vector<MType *> params = action->getParameters();	
		  long id=this->myMap.subscribeFunction(action->getCurrentAction());
		  if(!myMap.addImageInput(id,params[1],error="In parameter REFERENCE. \n")) return false;
		  if(!myMap.addImageOutput(id,params[3],error="In parameter OUTPUT. \n")) return false;
	      if(!checkType(FileProcessing::eMap,params[4]->getValue(),error="Parameter TYPE. NONE, GRAY or RGB. \n"))	return false;
		  
		  int val = (dynamic_cast<MIntType*>(params[0]))->getValue(0);
		  if(val!=8 && val!=16 && val!=32 && val!=64) 
		   { 
			   error="Parameter DEPTH. Depth allowed only 8, 16, 32 or 64. \n"; 
			   return false;
		   }
	  }

	  bool _good(Action *action, string &error){ return true; }
};

#endif //checker