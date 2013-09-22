#ifndef _ACTION_
#define _ACTION_

#include <iostream>
#include <vector>
#include <map>
#include <memory>
#include <string>
#include "MType.h"
#include "MAllTypes.h"
#include "PModule.h"

using namespace std;

class Action
{
 protected:
	string _current_action;
	vector<MType *> _realParams;
	vector<string>  _nameParams;
//	PModule *_myModule;
	shared_ptr<PModule> _myModule;

 public:

	typedef map<string,TypeT> TypesMap;
	static TypesMap _typesmap;

	Action(shared_ptr<PModule> mod)
	{
		_myModule=mod;
		_typesmap=initialize();
	
	};
	~Action(void){
	    cout<<"ACTION destroyed "<<_current_action<<endl;
		while(!this->_realParams.empty()) delete this->_realParams.back(), this->_realParams.pop_back();
	};

	static TypesMap initialize();
	
	inline int execute(unsigned int pid)
	{
	 _myModule->executeAction(this->_current_action,this->_realParams,pid);
	 return 0;
	}



std::vector<MType *> parseParams(const char **nparams,const char **params,int countP)
		{
	
		vector<MType*> myConvertedParams;
		int nelem = countP;
		int i;
		string s1,s2;
		for (i =0; i<nelem; i++)
		{
		      s1=nparams[i];
			  s2=params[i];
			  myConvertedParams.push_back(GetType::fromType(_typesmap[s1],s2,s1));
			  _nameParams.push_back(s1);
		}
		return myConvertedParams;
	}
	
// Parameter are sorted for accessing the parameter functions in the right order
void sortParams()
{
	sort(_nameParams.begin(),_nameParams.end());
	sort(_realParams.begin(), _realParams.end(), [](MType *a, MType  *b) -> bool { return a->getTag()<b->getTag(); });
}


int setAction(const char *FNAME,const char **nparams,const char **params,int countP)
{
		 _current_action = FNAME;
	// Doing the params interpretation and casting
		 _realParams = parseParams(nparams,params,countP);
		 sortParams();
		return 0;
}



// Adds a new parameter
void addParameter(string paramName,const char*paramValue){

	MType *mp = new MStringType(paramValue,paramName);
	this->_nameParams.push_back(paramName);
	this->_realParams.push_back(mp);
	sortParams();
}

// Add new value to a existent parameter
inline	void modifyParameterValue(string paramName,MType *newValue)
{
	  vector<MType *>::iterator myit;
	  for (vector<MType *>::iterator myit = this->_realParams.begin();
		   myit !=this->_realParams.end(); myit++ )
	  {
			if((*myit)->getTag().compare(paramName)==0)
			{
			 (*myit)=newValue; 
			 break;
			}
							
	  }
	
}

/*** Getters and setters ****/
	string getCurrentAction(){			return _current_action;}
	vector<string>& getParameterNames(){	return _nameParams;}
	vector<MType *>& getParameters(){	return _realParams;}

	void setParameterNames(vector<string> nameParams){	_nameParams=nameParams;}
	void setParameters(vector<MType *> realParams){ _realParams=realParams;}


	/*************************************************************/
};

#endif //ACTION
