
#ifndef _MALLTYPES_
#define _MALLTYPES_

#include "MType.h"
#include <iostream>
#include "utils.h"

using namespace std;

class MStringType : public MType {
	public:
		MStringType(string obj,string tag):MType(obj,tag){};
		MStringType(const char *obj,const char* tag):MType(obj,tag){};

		MStringType(string obj):MType(obj){};
		MStringType(const char *obj):MType(obj){};

		~ MStringType(){};


	void transformType(){};

        const char* getValue() { return _myNameType.c_str(); };
		
};

class MIntType : public MType {
	private:
		int _myObjectType;

	public:
		MIntType(string obj):MType(obj)
		{
			_myObjectType=0;
			this->transformType();

		};
		MIntType(string obj,string tag):MType(obj,tag)
		{
			_myObjectType=0;
			this->transformType();

		};
		~ MIntType(){};
		void transformType(){
		    _myObjectType = atoi(_myNameType.c_str());;		
	
	}

	    int getValue() { return _myObjectType; };


	};
/********************DOUBLE TYPE*********************/
class MDoubleType : public MType {
	private:
		double _myObjectType;

	public:
		MDoubleType(string obj):MType(obj){
		
			_myObjectType = 0.0;
			this->transformType();

		};
		MDoubleType(string obj,string tag):MType(obj,tag){
			_myObjectType = 0.0;
			this->transformType();

		};
		~ MDoubleType(){};
		void transformType(){
		    _myObjectType = atof(_myNameType.c_str());;		
	
	}

	    double getValue() { return _myObjectType; };


	};





class MBoolType : public MType {
	private:
		bool _myObjectType;

	public:
		MBoolType(string obj):MType(obj)
		{
			_myObjectType = false;
			this->transformType();

		};
		MBoolType(string obj,string tag):MType(obj,tag)
		{
			_myObjectType = false;
			this->transformType();

		};
		~ MBoolType(){};
		void transformType()
		{	
			_myObjectType = (this->_myNameType[0]=='T');		
		}

	    bool getValue() { return _myObjectType; };


	};

struct GetType{

static MType* fromType(TypeT t,string param,string tag){
			ut::Trace tr = ut::Trace("GetType::fromType",__FILE__);
			switch(t){
				case(stringT):
					return new MStringType(param,tag);

				case(intT):
					return new MIntType(param,tag);

				case(boolT):
					return new MBoolType(param,tag);

				case(doubleT):
					return new MDoubleType(param,tag);

				default:	
					tr.message("Non existent type taken as string:", tag.c_str());
					return new MStringType(param,tag);

			};
			return nullptr;
		};
};


#endif
