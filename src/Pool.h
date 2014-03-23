#ifndef _POOL_
#define _POOL_

#include <vector>
#include <iostream>
#include <functional>
#include <memory>
#include <map>
#include <string>
#include "utils.h"
#include "Slot.h"

#include <limits>

using namespace cv;

/* Note, that this class is a singleton. */
class Pool
{
    private:
        
		enum STORED{ IMAGE,OBJECT};


		static vector<shared_ptr<Slot<Mat>>> imagesPool;
		static vector<shared_ptr<Slot<vloP>>> objectsPool;
		static map<string, double> factorsPool;

		static int numImages;
		static int maxImages; 
		static int numObjects;
		static int maxObjects;

        static shared_ptr<Pool> instance;
       

		Pool(int ipm, int op){

			createImagePool(ipm);
		    createObjectsPool(op);
		}



    public:
        /**
         * Static method for accessing class instance.
         * Part of Singleton design pattern.
         *
         * @return ObjectPool instance.
         */

	/*	static safeSingleton & getInstance( int id )
			{
				std::call_once( safeSingleton::only_one,
						[] (int idx)
						{
						   safeSingleton::instance_.reset( new safeSingleton(idx) );

						   std::cout << "safeSingleton::create_singleton_() | thread id " + idx << std::endl;
						 }
						, id );

				return *safeSingleton::instance_;
			}
		*/


        static shared_ptr<Pool> getInstance()
        {
            if (instance == 0)
            {
                instance = shared_ptr<Pool>(new Pool(100,80));
            }
            return instance;
        }


        static shared_ptr<Pool> getInstance(int imp,int op)
        {
            if (instance == 0)
            {
                instance = shared_ptr<Pool>(new Pool(imp,op));
            }
            return instance;
        }

		~Pool(){
			  //   std::cout<<"Pool Destroyed!"<<std::endl;
			}


		static void createImagePool(int pool_size)
		{
		 
			Pool::imagesPool.reserve(pool_size);
			Pool::maxImages = pool_size;
			Pool::numImages = 0;
		}

		static void createObjectsPool(int pool_size)
		{

			Pool::objectsPool.reserve(pool_size);
			Pool::maxObjects = pool_size;
			Pool::numObjects = 0;	 
		}

	/************************************************************************************************/
     /**
         * Returns instance of Resource.
         * 
         * New resource will be created if all the resources
         * were used at the time of the request.
         *
         * @return Resource instance.
         */
	inline	shared_ptr<Slot<Mat>> getResourceImage(const char* id)
    {
		ut::Trace tr = ut::Trace("getResource Image",__FILE__);
			
		if(imagesPool.empty())  // if empty allocate one place
		{
		   auto sp1 = make_shared<Slot<Mat>>(id);
		   imagesPool.push_back(sp1);
		   Pool::numImages++;
		   return sp1;
		}
		else
		{
			for(int i=0;i<Pool::numImages;i++)
			{
				auto tmp = imagesPool[i];
				if(strcmp(tmp->getId(),id)==0)  return tmp;  // it�s already in my pool
			}

			auto sp1 = make_shared<Slot<Mat>>(id);
			imagesPool.push_back(sp1); // if not, I must save...
			Pool::numImages++;
			return sp1;
		}
	//	return NULL;
    	}


	inline	shared_ptr<Slot<vloP>> getResourceObject(const char* id){

		if(objectsPool.empty())
		{
			auto sp1 = make_shared<Slot<vloP>>(id);
			Pool::numObjects++;
			objectsPool.push_back(sp1);
			return sp1;
		}
		else
		{
		   for( int i=0;i<Pool::numObjects;i++)
		   {
			 auto sp1 =  objectsPool[i];
			 if(strcmp(sp1->getId(),id)==0) return sp1;
		    }

		   auto sp1 = make_shared<Slot<vloP>>(id);
		   Pool::numObjects++;
		   objectsPool.push_back(sp1);
		   return sp1;
		}

		//	return NULL;
        }

	/************************************************************************************************/
  

/****************************************************************************
 * cleanPool
 * ----------------------------
 *  
 *
 ***************************************************************************/

		// Removes all objects from the pool
		void cleanPools(){
			if(Pool::numImages>Pool::maxImages || Pool::numObjects>Pool::maxObjects )
			{
			  
				if(!imagesPool.empty())
				{
					imagesPool.clear();
				}
			   while(!objectsPool.empty())
				{
					shared_ptr<Slot <vloP>> T =objectsPool.back();
					T->getValue()->clear();
					// while(!T->getValue()->empty()) T->getValue()->pop_back();
					objectsPool.pop_back();
				}
			  if(!objectsPool.empty())
			  {
				  objectsPool.clear();
			  }

			Pool::numImages = 0;
			Pool::numObjects =0;
			imagesPool.reserve(Pool::maxImages);
			objectsPool.reserve(Pool::maxObjects);
			}

			Pool::factorsPool.clear();
		}



/****************************************************************************
 * Image exists
 * ----------------------------
 ***************************************************************************/
		bool containsImage(const char*im)
		{
		 
			for(int i=0;i<Pool::numImages;i++)
			{
				auto tmp = imagesPool[i];
				if(strcmp(tmp->getId(),im)==0) return true;  // it�s already in my pool
			}
		    return false;
		}



/****************************************************************************
 * GET IMAGE
 * ----------------------------
 *  
 *  *
 * The resource must be initialized back to
 * the default settings before someone else
 * attempts to use it.
 ***************************************************************************/
	inline	Mat* getImage(const char *input){
			 auto newslot= getResourceImage(input);
		     return	newslot->getValue();
		}


/****************************************************************************
 * STORE IMAGE
 * ----------------------------
 *  
 *
 ***************************************************************************/
inline int storeImage(Mat &image,const char *output){

			 auto newslot= getResourceImage(output);  
			 newslot->setValue(image);
			 return 0;
		}


/****************************************************************************
 * Objects (LoP) exists
 * ----------------------------
 ***************************************************************************/
		bool containsObject(const char*ob)
		{
		 
			for(int i=0;i<Pool::numObjects;i++)
			{
				auto tmp = objectsPool[i];
				if(strcmp(tmp->getId(),ob)==0) return true;  // it�s already in my pool
			}
		    return false;
		}


/****************************************************************************
 * GET LIST OF POINTS
 * ----------------------------
 *  
 *
 ***************************************************************************/
		inline	vloP* getlObj(const char *input){
			auto newslot= getResourceObject(input);
			return	newslot->getValue();
		}


/****************************************************************************
 * STORE LIST of POINTS
 * ----------------------------
 *  
 *
 ***************************************************************************/
		inline int storelObj(vloP _vlop,const char *output){
			auto newslot=  getResourceObject(output);
			newslot->setValue(_vlop);
			return 0;
		}



/****************************************************************************
 *  Factor exists
 * ----------------------------
 ***************************************************************************/
		bool containsFactor(const char*ref)
		{
		 
		   std::map<string,double>::iterator it;
		   it=this->factorsPool.find(ref);
		   return (it!=factorsPool.end()); 
		}


/****************************************************************************
 * GET FACTOR
 * ----------------------------
 *  First method is deprecated
 *
 ***************************************************************************/
		inline double getFactor(const char *input,const char* ref)
		{
			std::map<string,double>::iterator it;
			double factor;
			it=this->factorsPool.find(ref);
			if(it!=factorsPool.end()) return (*it).second;
			else
			factor = atof(input);
			return factor;
		}
		inline double getFactor(const char* ref)
		{
			std::map<string,double>::iterator it;
			it=this->factorsPool.find(ref);
			if(it!=factorsPool.end()) return (*it).second;
			else
			return 0.0;
		}

/****************************************************************************
 * STORE FACTOR
 * ----------------------------
 *  
 *
 ***************************************************************************/
		inline int storeFactor(double factor,const char *output){
			factorsPool[output]=factor;
			return 0;
		}

};



#endif

