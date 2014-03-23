
#include "PModule.h"
#include <string.h>
#include <map>

class General :
	public PModule
{
protected:
	 typedef  void(General::*Function)(vector<MType *>,unsigned int pid); // function pointer type
	 static map<string, Function > tFunc;

public:
	General(void){
			tFunc["CLEAN"] = &General::clean;
	
	}
 ~General(void)
 {
	// std::cout<<"CLEAN destroyed"<<std::endl;
 };


virtual void executeAction(string current_action,vector<MType *> params,unsigned int pid)
	{
	
		std::map<std::string, Function >::iterator x =  tFunc.find(current_action);
	if (x != tFunc.end()) {
		(*this.*(x->second))(params,pid);
    }
	return;
	
	}

	void clean(std::vector<MType *> parValues,unsigned int pid)
	{
		pool->cleanPools();
	}


};

