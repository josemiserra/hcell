#ifndef _FMAP_
#define _FMAP_

#include <string>
#include <iostream>
#include <vector>
#include "utils.h"
#include "MType.h"


using namespace std;
using namespace ut;

class FlowMap
{
	protected:
    static long idNum;
	vector<string> output_list; // output list for images and objects
	vector<string> output_list_obj; // output list for objects
	vector<string> output_factors; // output list for doubles

	typedef struct fNode
	{
	 long _id;
	 string function;
	vector<string>  images_in;
	vector<string>  images_out;
	vector<string>  objects_in;
	vector<string>  objects_out;
	 fNode(long id,string function)
	 {
	  _id=id;   
	 }
	};

	vector<shared_ptr<fNode>> fMap;


public:
	  FlowMap(void)
		{
	 
		};
	  ~FlowMap(void)
	    {
			fMap.clear();
			// delete fMaps
		};

/*****************************************************************
	NAME: subscribeFunction
    DESCRIPTION: given a function, it creates an entry into the map of functions

    @return long an unique identifier for the function, acts as a map.

******************************************************************/
 long subscribeFunction(string functionName)
	{
		  this->idNum++;
		  this->fMap.push_back(shared_ptr<fNode>(new FlowMap::fNode(this->idNum,functionName)));
		  return this->idNum;
	};

  /*****************************************************************
	NAME: addImageOutput
    DESCRIPTION: adds to the map an image declared as an output, checks
	first if is a syntactic tree.

    @return bool TRUE if function has succesfully added to the map.

******************************************************************/
  bool addImageOutput(long id,MType *type, string &error)
 {
	 if(type->getCompilerTag()==Interpreter::STAND)
	 {
		return addImageOutput(id,type->getValue(),error);
	 }
	 else
	 {
		 vector<string> Images, Factors;
		 type->getSTree().getListOfNames(Images,Factors);
		 for(auto it = Images.begin();it!= Images.end(); it++)
		 {
			 if(!utils::contains(output_list,*it)) // if does not contain my image, then is my output
			 {
			  if(!addImageOutput(id,*it,error)) return false;
			 }
			 else
			 {
			  if(!addImageInput(id,*it,error)) return false;
			 }
		 }
		  for(auto itf = Factors.begin();itf!= Factors.end(); itf++)
		  {		   
			 if(!addFactorInput(id,*itf,error)) return false;
		  }	  

	 }
     return true;
 }

 /*****************************************************************
	NAME: addImageOutput
    DESCRIPTION: adds to the map an image declared as an output

    @return bool TRUE if function has succesfully added to the map.

******************************************************************/
 bool addImageOutput(long id,string param,string &error)
	  {
          if(fMap.size()==0) return false; // Need to subscribe function first. Just a precaution.
		  size_t i;
		  for(i=0;i<fMap.size();i++)
		  {
			  if(fMap[i]->_id==id) break;
		  }
		  if(i==fMap.size())
		  {
		  return false; // Error, id not found
		  }
		   utils::trim(param);
		  if(param.compare("")==0)
		  {
			  error=" Empty parameter. Empty names are not allowed for output. \n";
			  return false;
		  }
		  size_t j;
		  for(j=0;j<fMap[i]->images_out.size();j++)
		  {
			  if(param.compare(fMap[i]->images_out[j])==0)
			  {
			   error="Two outputs with the same name in same function. Rename one of them.\n";
			   return false;
			  }
			  
		  }
		  fMap[i]->images_out.push_back(param);
		  // Adds to the general list of outputs
		  if(!utils::contains(output_list,param))  output_list.push_back(param);
		  return true;
	   };

  /*****************************************************************
	NAME: addImageInput
    DESCRIPTION: adds to the map an image declared as an input

    @return bool TRUE if function has succesfully added to the map.

******************************************************************/
 bool addImageInput(long id,MType *type, string &error)
 {
	 if(type->getCompilerTag()==Interpreter::STAND)
	 {
		return addImageInput(id,type->getValue(),error);
	 }
	 else
	 {
		 vector<string> Images, Factors;
		 type->getSTree().getListOfNames(Images,Factors);
		 for(auto it = Images.begin();it!= Images.end(); it++)
		 {
			if(!addImageInput(id,*it,error)) return false;
		 }
		 for(auto itf = Factors.begin();itf!= Factors.end(); itf++)
		 {  
			if(!addFactorInput(id,*itf,error)) return false;
		  }	  
	 }
    return true;
 }


 bool addImageInput(long id,string param,string &error){

		  if(fMap.size()==0) return false; // Need to subscribe function first. Just a precaution.
		  size_t i;
		  for(i=0;i<fMap.size();i++)
		  {
			  if(fMap[i]->_id==id) break;
		  }
		  if(i==fMap.size())
		  {
		  return false; // Error, id not found
		  }
		  utils::trim(param);
		  
		  if(param.compare("")==0)
		  {
			  error=" Empty parameter. Empty names are not allowed. \n";
			  return false;
		  }
		  
		  // If my input is not in the output list, means, there is something wrong!!!
		  // This is the most important check!!! 
		  if(output_list.size()==0)
		  {
				error="\t Parameter input ";
				error+=param;
				error+=" cannot be load.\n";
				error="\t You need to load images first with a LOAD FILE function!!! \n";
				return false;
		  }
		 // cout<<output_list[0]<<endl;
		 //  cout<<param<<endl;
		  if(!utils::contains(output_list,param)) 
		  {
			error="Input parameter image:";
			error+= param;
			error+= " cannot be withdrawn from memory. Rename it to an existent one, please!";
			return false;	  
		  }

		  fMap[i]->images_in.push_back(param);
		  return true;
	  };

   /*****************************************************************
	NAME: addObjectOutput
    DESCRIPTION: adds to the map an object declared as an output


******************************************************************/

  bool addObjectOutput(long id,string param,string &error)
	  {
          if(fMap.size()==0) return false; // Need to subscribe function first. Just a precaution.
		  size_t i;
		  for(i=0;i<fMap.size();i++)
		  {
			  if(fMap[i]->_id==id) break;
		  }
		  if(i==fMap.size())
		  {
		  return false; // Error, id not found
		  }
		   utils::trim(param);
		  if(param.compare("")==0)
		  {
			  error=" Empty parameter. Empty names are not allowed for output. Have you forgot it? \n";
			  return false;
		  }
		  size_t j;
		  for(j=0;j<fMap[i]->objects_out.size();j++)
		  {
			  if(param.compare(fMap[i]->objects_out[j])==0)
			  {
			   error="Two outputs with the same name in same function. Rename one of them.\n";
			   return false;
			  }
		  }

		  fMap[i]->objects_out.push_back(param);
		  if(!utils::contains(output_list_obj,param)) output_list_obj.push_back(param);
		  return true;
	   };

 bool addObjectInput(long id,string param,string &error){

		  if(fMap.size()==0) return false; // Need to subscribe function first. Just a precaution.
		  size_t i;
		  for(i=0;i<fMap.size();i++)
		  {
			  if(fMap[i]->_id==id) break;
		  }
		  if(i==fMap.size())
		  {
		  return false; // Error, id not found
		  }
		  utils::trim(param);
		  if(param.compare("")==0)
		  {
			  error=" Empty parameter. Empty names are not allowed for input. \n";
			  return false;
		  }
		  // If my input is not in the output list, means, there is something wrong!!!
		  // This is the most important check!!! 
		  if(output_list_obj.size()==0)
		  {
				error="\t Parameter input ";
				error+=param;
				error+=" cannot be load.\n";
				error="\t You need to create some objects first with a segmentation or contour function!!! \n";
				return false;
		  }
		 // cout<<output_list[0]<<endl;
		 //  cout<<param<<endl;
		  if(!utils::contains(output_list_obj,param)) 
		  {
			error="Input parameter :";
			error+= param;
			error+= " cannot be withdrawn from memory. Rename it to an existent one, please!";
			return false;	  
		  }
		  fMap[i]->objects_in.push_back(param);
		  return true;

	  };


  bool addFactorOutput(long id,string param,string &error)
	  {
          utils::trim(param);
		  if(param.compare("")==0)
		  {
			  error=" Empty parameter. Empty names are not allowed for output. \n";
			  return false;
		  }
	
		  if(!utils::contains(output_factors,param) && !isdigit(param[0]))
		  { 
			  output_factors.push_back(param);										             // cout<<"Out:"<< param <<endl;
		  }
		  return true;
	   };

  bool addFactorInput(long id,string param,string &error){

		  if(fMap.size()==0) return false; // Need to subscribe function first. Just a precaution.
		  utils::trim(param);
		  if(param.compare("")==0)
		  {
			  error=" Empty parameter. Empty names are not allowed. \n";
			  return false;
		  }
		  if(!utils::contains(output_list,param)&&(!isdigit(param[0])))
		  {
			error="Input factor:";
			error+= param;
			error+= " cannot be withdrawn from memory. Rename it to an existent one, please!";
			return false;	  
		  }
		  return true;

	  };



};
#endif  // _FLOWMAP