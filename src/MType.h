
#ifndef _MTYPE_
#define _MTYPE_

#include <map>
using namespace std;


enum TypeT {stringT, intT, doubleT, boolT, unknownT};


class MType 
{
	
	protected:
	string _myNameType;
	string _tag;

	public:
		MType(string obj)
		{
			_myNameType=obj;
		};
		MType(string obj,string tag)
		{
			_myNameType=obj;
			this->_tag=tag;
		};
		// virtual ~ MType();

		virtual void transformType()=0;
		string getTag(){ return _tag;};

		//user defined compare function that compares string values
		bool Compare(MType  &s1, MType  & s2) 
		{
			return (s1.getTag().compare(s2.getTag())>1);
		}
		string getValue(){ return _myNameType;}

};




#endif
