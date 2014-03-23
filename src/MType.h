
#ifndef _MTYPE_
#define _MTYPE_

#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <map>
#include "STree.h"
#include "Interpreter.h"
#include "utils.h"

#define IS_TREE "XoF_TaEw"

using namespace std;


enum TypeT {stringT, intT, doubleT, boolT, idT};


class MType 
{	
	protected:
		string _name; // Refers to the value as a string e.g. "0.5".
		string _tag;        // Refers to the parameter name e.g. SIGMA.
		int    _compilerTag;  // Refers if type needs to be assigned in execution time or compilation time
		STree  _tree; // Syntactic Tree associated to the object to be evaluated in execution time
		/* There are two possible modalities 
		   = STAND in which remains to assigned type, 
		   = TREE in which the STree is evaluated,
		    */
		static long seed; 
	public:

		MType(const string& obj,const string& tag)
		{
			_name.assign(obj);
			_tag.assign(tag);
			_compilerTag==Interpreter::STAND;
		};
		// virtual ~ MType();
		virtual void transformType()=0;
		
		// Setters and getters
		string getTag(){ return _tag;};
	    string getValue(){ return _name;}
		string getValueAsString(){return _name;}
		int getCompilerTag(){ return _compilerTag; }
		STree getSTree(){ return _tree;}
		
		//user defined compare function that compares string values
		bool Compare(MType  &s1, MType  & s2) 
		{
			return (s1.getTag().compare(s2.getTag())>1);
		}

		// Hash key for pool
		int inthash(int key)
		{
			key += ~(key << 15);
			key ^=  (key >> 10);
			key +=  (key << 3);
			key ^=  (key >> 6);
			key += ~(key << 11);
			key ^=  (key >> 16);
			return key;
		} 

	    void assignName()
		{
		 string outname;
		 this->_tree.getEmptyName(outname);
		 if(outname.empty()) 
		 {
		 if(seed==(INT_MAX-1))  MType::seed = 0;
		 _name = IS_TREE;
		 string num = to_string((long long)seed);
		 _name.append(num);
		 _name.append("_");
		 MType::seed++;
		 return;
		 }
		 else
		 {
		  _name = outname;
		 
		 }

		}

		bool isTree(){ return _compilerTag==Interpreter::TREE;}

};
/***************************************************************************
*
*   STRING TYPE CHECKING
*
*   Note: A string Type IS NOT and identifier TYPE.
*   Identifiers must be checked by the interpreter!!
*   
*******************************************************************************/

class MStringType : public MType {
	public:
		MStringType(const string& obj,const string& tag):MType(obj,tag){ transformType(); };
		MStringType(const char *obj,const char* tag):MType(obj,tag){ transformType();};


		~ MStringType(){};

		void transformType()
		{
		     int pos;
			 this->_compilerTag = Interpreter::STAND;
			 pos= this->_name.find_first_not_of(" \r\t\n");
			 if(pos==std::string::npos || this->_name.length()==0)
			 {
			
				string error;
				error = "\n Bad Conversion from parameter to string:\n ";
				error += "Empty string!!";
				error+= " in parameter ";
				error+= this->getTag();
				error+=" : ";
				error+=_name.c_str();	
				throw  error;
			 }


		};

        const char* getValue() { return _name.c_str(); };
		
};

/***************************************************************************
*
*   Identifier TYPE CHECKING
*
*  
*   
*******************************************************************************/
class MIdentifierType : public MStringType {
	public:
		MIdentifierType(const string& obj,const string& tag):MStringType(obj,tag){ transformType(); };
		MIdentifierType(const char *obj,const char* tag):MStringType(obj,tag){ transformType();};


		void transformType() // override
		{

			 string error;
			 this->_compilerTag  = Interpreter::checkInputObject(_name,error); // Check if we need to parse or not
			 
			 switch(this->_compilerTag)
			 {			 
			 case(Interpreter::STAND):  // It is a simple identifier...
			 {
				// do nothing, we keep our name
				int pos;
				pos = this->_name.find_first_not_of(" \r\t\n");
				// We can�t start with numbers any name.
				// We can�t start with those characters
				if(pos==std::string::npos || this->_name.length()<3  || isdigit(_name[pos]))
				 {		
					string error;
					error = "\n Bad Conversion from parameter to string:\n ";
					error += "Length is empty, starts with forbiden character or number or is less than 3 characters.";
					error+= " in parameter ";
					error+= this->getTag();
					error+=" : ";
					error+=_name.c_str();	
					throw  error;
				 }
				 break;
			  }
			 case(Interpreter::TREE):  // It is a tree, we try to obtain our STree. Time to parse!!!
			 {
				 if(!Interpreter::getSyntacticTree(_name,_tree,error))
				 {
				   error+="Error parsing Syntactic Tree.\n";
				   throw error;
				 }
				 assignName(); // We have the tree, we don�t need the _name... so replace it for a URI
			     break;
			 }
			 case(Interpreter::CERROR): // It is an error 
			 {
				  error += "\n Bad Conversion from parameter to expression: ";
				  error += "Error converting ";
				  error	+=_name.c_str();	
				  error	+= " into  parameter ";
				  error	+= this->getTag();
				  throw  error;	  
			  }
			 default: break;
			 }
			  return;		     
		};

        string getValue(int pid)
		{ 
			string newname;
			if(this->_compilerTag==Interpreter::STAND)
			{
				ut::utils::combnames(_name,pid,newname);
				return newname;
			}
			else{ 
				return _tree.evalTreeAsString(pid,this->_name);
			}
		};
		
		void refresh(int pid)
		{
			if(_compilerTag==Interpreter::TREE)
			{
				this->_tree.evalTreeAsString(pid,_name);
				// 
			}
			return;
		}

};


/***************************************************************************
*
*   INTEGER TYPE CHECKING
*
*******************************************************************************/
class MIntType : public MType 
{
	private:
		int _value;

	public:
/*		MIntType(string obj):MType(obj)
		{
			_value=0;
			this->transformType();
		};
*/
		MIntType(const string& obj,const string& tag):MType(obj,tag)
		{
			_value=0;
			this->transformType();

		};
		MIntType(const string& obj,const string& tag, int value):MType(obj,tag)
		{
			_value=value;
		};
		~ MIntType(){};

		void transformType(){

			 string error;
			 this->_compilerTag  = Interpreter::checkInputObject(_name, error); // Check if we need to parse or not
			 
			 switch(this->_compilerTag)
			 {
			 
			 case(Interpreter::STAND):  // It is a double
			 {
					std::string::size_type sz;   // alias of size_t
					 _value = std::stoi(_name.c_str(),&sz);  // atoi can cause undefined behavior
			 	
					 if(_name.c_str()[sz]!='\0')
					 {
						 string error;
						 error = "\n Bad Conversion from parameter to integer:\n ";
						 error += "The string has numbers but also other characters, ";
						 error+= " in parameter ";
						 error+= this->getTag();
						 error+=" : ";
						 error+=_name.c_str();	
						 throw  error;
					 }
					break;
				}
				case(Interpreter::TREE):  // It is a tree
				 {
				 if(!Interpreter::getSyntacticTree(_name,_tree,error))
				 {
				   throw error;
				 }
			     break;
				}
				case(Interpreter::CERROR): // It is an error 
				 {
				  error += "\n Bad Conversion from parameter to double expression: ";
				  error += "Error converting ";
				  error	+=_name.c_str();	
				  error	+= " into  parameter ";
				  error	+= this->getTag();
				  throw  error;	  
				 }
				default: break;
			 }
			  return;
	}


	    int getValue(int pid) { 
			if(this->_compilerTag==Interpreter::STAND)
			{
			return _value;
			}
			else return _tree.evalTreeAsInt(pid);};

	};
/***************************************************************************
*
*   DOUBLE TYPE CHECKING
*
*******************************************************************************/
class MDoubleType : public MType {
	private:
		double _value;
	public:
	/*	MDoubleType(string obj):MType(obj){
			_value = 0.0;
			this->transformType();
		};
	*/
		MDoubleType(const string &obj,const string &tag):MType(obj,tag){
			_value = 0.0;
			this->transformType();
		};

		MDoubleType(const string &obj,const string& tag,double value):MType(obj,tag){
			_value = value;
		};

		~ MDoubleType(){};

		void transformType(){

			 string error;
			 char *pEnd;
			 this->_compilerTag  = Interpreter::checkInputObject(_name, error); // Check if we need to parse or not
			 
			 switch(this->_compilerTag)
			 {
			 
			 case(Interpreter::STAND):  // It is a double
			 {
				_value = strtod(_name.c_str(),&pEnd);
				//  _myObjectType = atof(_myNameType.c_str()); // undefined behaviour
				break;
			  }
			 case(Interpreter::TREE):  // It is a tree
			 {
				 if(!Interpreter::getSyntacticTree(_name,_tree,error))
				 {
				   throw error;
				 }
			     break;
			 }
			 case(Interpreter::CERROR): // It is an error 
			 {
				  error += "\n Bad Conversion from parameter to double expression: ";
				  error += "Error converting ";
				  error	+=_name.c_str();	
				  error	+= " into  parameter ";
				  error	+= this->getTag();
				  throw  error;	  
			  }
			 default: break;
			 }
			  return;
	}

	    double getValue(int pid)
		{ 
			if(this->_compilerTag==Interpreter::STAND)
			{
			return _value;
			}
			else return _tree.evalTreeAsDouble(pid);
		};

	};

/********************BOOL TYPE*********************/
class MBoolType : public MType {
	private:
		bool _value;
	public:
	/*	MBoolType(string obj):MType(obj)
		{
			_value = false;
			this->transformType();

		}; */
		MBoolType(const string& obj,const string& tag):MType(obj,tag)
		{
			_value = false;
			this->transformType();

		};
		~ MBoolType(){};
		
		void transformType()
		{	
			string error;
			this->_compilerTag =Interpreter::STAND;
			if(this->_name.compare("TRUE")==0||this->_name.compare("true")==0)
				_value = true;
			else
				if(this->_name.compare("FALSE")==0||this->_name.compare("false")==0)
					_value = false;
				else{
				  error = "\n -- Error converting string  ";
				  error+=_name;	
				  error+= " into boolean parameter ";
				  error+= _tag;
				  error+="\n Boolean allowed types: TRUE and FALSE";
				  throw  error;
				 }


		}

	    bool getValue() { return _value; };


	};

struct GetType{

static MType* fromType(TypeT t,const string& param,const string& tag){
			ut::Trace tr = ut::Trace("GetType::fromType",__FILE__);
			switch(t){
				case(stringT):
					return new MStringType(param,tag);
					break;
				case(idT):
					return new MIdentifierType(param,tag);
					break;
				case(intT):
					return new MIntType(param,tag);
					break;
				case(boolT):
					return new MBoolType(param,tag);
					break;
				case(doubleT):
					return new MDoubleType(param,tag);
					break;
				default:	
					tr.message("Non existent type taken as string:", tag.c_str());
					return new MStringType(param,tag);
					break;
			};
			return nullptr;
		};
};


#endif
