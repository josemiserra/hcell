#ifndef _POOL_MANAGER_
#define _POOL_MANAGER_

#include <vector>
#include <iostream>
#include <functional>
#include <memory>
#include <string>
#include "utils.h"
#include "Slot.h"
#include "MType.h"


using namespace cv;

/* Note, that this class is a singleton. */
class PoolManager
{
    private:
        
		enum STORED{ IMAGE,OBJECT};


		static vector<shared_ptr<Slot<Mat>>> imagesPool;
		static vector<shared_ptr<Slot<vloP>>> objectsPool;
		
		static int numImages;
		static int maxImages; 
		static int numObjects;
		static int maxObjects;

        static shared_ptr<PoolManager> instance;
       

		PoolManager(int ipm, int op){

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


        static shared_ptr<PoolManager> getInstance()
        {
            if (instance == 0)
            {
                instance = shared_ptr<PoolManager>(new PoolManager(100,80));
            }
            return instance;
        }


        static shared_ptr<PoolManager> getInstance(int imp,int op)
        {
            if (instance == 0)
            {
                instance = shared_ptr<PoolManager>(new PoolManager(imp,op));
            }
            return instance;
        }

		~PoolManager(){
			     std::cout<<"Destroy pool!"<<std::endl;
			}




		static void createImagePool(int pool_size)
		{
		 
			PoolManager::imagesPool.reserve(pool_size);
			PoolManager::maxImages = pool_size;
			PoolManager::numImages = 0;
		}

		static void createObjectsPool(int pool_size)
		{

			PoolManager::objectsPool.reserve(pool_size);
			PoolManager::maxObjects = pool_size;
			PoolManager::numObjects = 0;	 
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
		   PoolManager::numImages++;
		   return sp1;
		}
		else
		{
			for(int i=0;i<PoolManager::numImages;i++)
			{
				auto tmp = imagesPool[i];
				if(strcmp(tmp->getId(),id)==0)  return tmp;  // it�s already in my pool
			}

			auto sp1 = make_shared<Slot<Mat>>(id);
			imagesPool.push_back(sp1); // if not, I must save...
			PoolManager::numImages++;
			return sp1;
		}

    	}


	inline	shared_ptr<Slot<vloP>> getResourceObject(const char* id){

		if(objectsPool.empty())
		{
			auto sp1 = make_shared<Slot<vloP>>(id);
			PoolManager::numObjects++;
			objectsPool.push_back(sp1);
			return sp1;
		}
		else
		{
		   for( int i=0;i<PoolManager::numObjects;i++)
		   {
			 auto sp1 =  objectsPool[i];
			 if(strcmp(sp1->getId(),id)==0) return sp1;
		    }

		   auto sp1 = make_shared<Slot<vloP>>(id);
		   PoolManager::numObjects++;
		   objectsPool.push_back(sp1);
		   return sp1;
		}

        }


/****************************************************************************
 * cleanPool
 * ----------------------------
 *  
 *
 ***************************************************************************/

		// Removes all objects from the pool
		void cleanPools(){
			if(PoolManager::numImages>PoolManager::maxImages || PoolManager::numObjects>PoolManager::maxObjects )
			{
			  
				if(!imagesPool.empty())
				{
					imagesPool.clear();
				}
			   while(!objectsPool.empty())
				{
					shared_ptr<Slot <vloP>> T =objectsPool.back();
					T->getValue()->clear();
				}
			  if(!objectsPool.empty())
			  {
				  objectsPool.clear();
			  }

			PoolManager::numImages = 0;
			PoolManager::numObjects =0;
			imagesPool.reserve(PoolManager::maxImages);
			objectsPool.reserve(PoolManager::maxObjects);
			}
		}



/****************************************************************************
 * Element exists
 * ----------------------------
 ***************************************************************************/
		bool containsImage(const char*im)
		{
		 
			for(int i=0;i<PoolManager::numImages;i++)
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



};



#endif

