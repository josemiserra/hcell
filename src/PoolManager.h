#ifndef _POOL_MANAGER_
#define _POOL_MANAGER_

#include "Pool.h"

using namespace std;

class PoolManager
{
private:
	shared_ptr<Pool> _pool;
	static shared_ptr<PoolManager> instance;
	static unsigned int _poolsize;

public:

     static shared_ptr<PoolManager> getInstance()
      {
            if (instance == 0)
            {
					instance = shared_ptr<PoolManager>(new PoolManager);
            }
            return instance;
       }
	
	 PoolManager(void)
	{
	  	if(_poolsize<10)
		{
			_pool = Pool::getInstance();
		}
		else
		{
			_pool = Pool::getInstance(_poolsize,_poolsize*0.8);
		}

	};

	~PoolManager(void){};

	void cleanPools(){ _pool->cleanPools();}


	void static setMemory(unsigned int memory)
	{
	 _poolsize = memory/8;  // Assumption 8 Mb per image
	}
	
/****************************************************************************
 * Image exists
 * ----------------------------
 ***************************************************************************/
		bool containsImage(const char*im)
		{
			return _pool->containsImage(im);
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
	inline	Mat* getImage(string input)
	{
		return _pool->getImage(input.c_str());
	}


/****************************************************************************
 * STORE IMAGE
 * ----------------------------
 *  
 *
 ***************************************************************************/
inline int storeImage(Mat &image,string output)
{
	  return _pool->storeImage(image,output.c_str()); 
}

/****************************************************************************
 * Objects (LoP) exists
 * ----------------------------
 ***************************************************************************/
		bool containsObject(const char*ob)
		{
			return _pool->containsObject(ob);
		}


/****************************************************************************
 * GET LIST OF POINTS
 * ----------------------------
 *  
 *
 ***************************************************************************/
		inline	vloP* getlObj(const char *input){
			return _pool->getlObj(input);
		}


/****************************************************************************
 * STORE LIST of POINTS
 * ----------------------------
 *  
 *
 ***************************************************************************/
		inline int storelObj(vloP _vlop,const char *output){
			return _pool->storelObj(_vlop,output);
		}



/****************************************************************************
 *  Factor exists
 * ----------------------------
 ***************************************************************************/
		bool containsFactor(const char*ref)
		{
			return _pool->containsFactor(ref);
		}


/****************************************************************************
 * GET FACTOR
 * ----------------------------
 *
 ***************************************************************************/
		inline double getFactor(const char* ref)
		{
			return _pool->getFactor(ref);
		}
		inline double getFactor(const char *input,const char* ref)
		{
			return _pool->getFactor(input,ref);
		 }

/****************************************************************************
 * STORE FACTOR
 * ----------------------------
 *  
 *
 ***************************************************************************/
		inline int storeFactor(double factor,const char *output){
			return _pool->storeFactor(factor,output);
		}

};

#endif