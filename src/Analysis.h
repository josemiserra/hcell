/* IN PREPARATION */


#ifndef _ANALYSIS_
#define _ANALYSIS_



#include <map>
#include "PModule.h"
#include "utils.h"


using namespace cv;
using namespace std;
using namespace ut;


class Analysis :
	public PModule
{
public:
	typedef  void( Analysis::*Function)(vector<MType *>,unsigned int pid); // function pointer type
	static map<string, Function > tFunc;
	static map<string, int> eMap;
	 


	 Analysis(void){};
	~Analysis(void){};


	virtual void executeAction(string current_action,vector<MType *> params,unsigned int pid)
	{	
		std::map<std::string, Function >::iterator x =  tFunc.find(current_action);
		if (x != tFunc.end())
		{
			(this->*(x->second))(params,pid);
		}
		return;
	}
};
#endif // _Analysis_
