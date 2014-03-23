#ifndef _ACTION_
#define _ACTION_

#include <iostream>
#include <vector>
#include <map>
#include <memory>
#include <string>
#include "MType.h"
#include "PModule.h"
#include "utils.h"

using namespace std;


#ifdef __GNUC__
	#if GCC_VERSION < 40500
	struct less_than_tag
	{
		inline bool operator() ( MType *a, MType *b)
		{
        return (a->getTag() < b->getTag());
		}
	};
	#endif
#endif

class Action
{
 protected:
	string _current_action;
	vector<MType *> _realParams;
	shared_ptr<PModule> _myModule;

 public:

	typedef map<string,TypeT> TypesMap;
	static TypesMap _typesmap;

	Action(Action &other)
	{
		other._myModule = other.getModule();
		_current_action = other.getCurrentAction();
		copy(other.seeParameters().begin(),other.seeParameters().end(),_realParams.end());
	}

	 Action(Action *const other)
	{
		
		_myModule = other->getModule();
		_current_action = other->getCurrentAction();
		copy(other->seeParameters().begin(),other->seeParameters().end(),_realParams.end());
	}


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
		}
		return myConvertedParams;
	}
	
// Parameter are sorted for accessing the parameter functions in the right order
void _sortParams()
{

#ifdef __GNUC__
	#if GCC_VERSION > 40500  // for lambda expression
	sort(_realParams.begin(), _realParams.end(), [](MType *a, MType  *b) -> bool { return a->getTag()<b->getTag(); });
	#else
	sort(_realParams.begin(), _realParams.end(),less_than_tag());
	#endif
#else
	sort(_realParams.begin(), _realParams.end(), [](MType *a, MType  *b) -> bool { return a->getTag()<b->getTag(); });
#endif	
	
}


int setAction(const char *FNAME,const char **nparams,const char **params,int countP)
{
		 _current_action = FNAME;
	// Doing the params interpretation and casting
		 _realParams = parseParams(nparams,params,countP);
		 _sortParams();
		return 0;
}



// Adds a new parameter
void addParameter(string paramName,const char*paramValue){

	MType *mp = new MStringType(paramValue,paramName);
	this->_realParams.push_back(mp);
	_sortParams();
}

// Add new value to a existent parameter
inline	void modifyParameterValue(string paramName,MType *newValue)
{
	  vector<MType *>::iterator myit;
	  MType *old;
	  for (vector<MType *>::iterator myit = this->_realParams.begin();
		   myit !=this->_realParams.end(); myit++ )
	  {
			if((*myit)->getTag().compare(paramName)==0)
			{
			 old =  (*myit);
			 (*myit)=newValue;
			 delete old;
			 break;
			}
							
	  }
	
}

/*** Getters and setters ****/
	string getCurrentAction(){			return _current_action;}
	vector<MType *>& getParameters(){	return _realParams;}
	vector<MType *> seeParameters(){	return _realParams;}

	shared_ptr<PModule> getModule(){ return this->_myModule; }
	void setParameters(vector<MType *> realParams){ _realParams=realParams;}


	/*************************************************************/
};

#endif //ACTION
