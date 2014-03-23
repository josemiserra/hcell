#include <map>
#include "ActionLoader.h"

using namespace std;

map<string,MODULE_LIST> ActionLoader::tMap;


 void ActionLoader::initialize(void){
		ActionLoader::tMap["FILE PROCESSING"]=FILE_PROC;
		ActionLoader::tMap["IMAGE PROCESSING"]=IM_PROC;
		ActionLoader::tMap["COMPUTE FEATURES"]=COMPUTE_FEATURES;
		ActionLoader::tMap["SEGMENTATION PROCESSING"]=SEGMENTATION_PROC;
		ActionLoader::tMap["MEASUREMENTS"]=MEASUREMENTS;

		return;
	}



Action* ActionLoader::getAction(const char *MNAME,const char *FNAME,const char **nameparams,const char **params,int countP){
          
	    ut::Trace tr = ut::Trace("ActionLoader::getAction",__FILE__);
	
	    Action *result;
		MODULE_LIST myModule;
		myModule=ActionLoader::tMap[MNAME];  

		switch(myModule){
			case FILE_PROC:
				result =  this->mselector.fileProc(FNAME,nameparams,params,countP);
				   break;
			case IM_PROC:
				result = this->mselector.imageProc(FNAME,nameparams,params,countP);
				   break;
			case SEGMENTATION_PROC:
				result = this->mselector.segmentationProc(FNAME,nameparams,params,countP);
				   break;
			case COMPUTE_FEATURES:
				result = this->mselector.computeFeatures(FNAME,nameparams,params,countP);
				   break;
			default:
				   cout<<("NON EXISTENT module")<<endl;
				   break;
		
		}
    
       
        return result;
    }


