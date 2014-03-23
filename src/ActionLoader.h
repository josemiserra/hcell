#ifndef _ACTION_LOADER_
#define _ACTION_LOADER_

#include <map>
#include "PModule.h"
#include "ModuleSelector.h"
#include "Action.h"

enum MODULE_LIST
    {
        FILE_PROC,
        COMPUTE_FEATURES,
		IM_PROC,
		SEGMENTATION_PROC,
		MEASUREMENTS,
		OBJECT_PROC,
		TOOLS
    };

using namespace std;

class ActionLoader 
{
protected:
	ModuleSelector mselector;
	
public:
	
	static map<string,MODULE_LIST> tMap;

    ActionLoader()
    {   
		 ActionLoader::initialize();
	}
	~ActionLoader(void)
	{
		// cout<<"ACTIONLOADER destroyed"<<endl;
	}

	Action* getAction(const char *MNAME,const char *FNAME,const char **nameparams,const char **params,int countP);

private:
	 static void initialize(void);
};



#endif // ActionLoader
