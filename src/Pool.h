#include <iostream>
#include <vector>

template <class T> class  Pool
{ 
protected:
	std::vector<T *> mypool;
	int _size;
	int _next;
	int _numFreeBlocks;

public: 
	Pool() 
	{ 
	
	} 
	~Pool()
	{
		this->cleanPool();
	} 

void createPool(int size) 
{ 
	mypool.reserve(size);
	_size = size;
	_next = 0;
	_numFreeBlocks = size;
} 

/**************************************************************/
void cleanPool() 
{ 
	for(int i=0;i< _size; i++)
	{
		 T* temp;
		 temp = mypool[i];
		 delete temp;
		_numFreeBlocks++;
	}
	_next=0;
} 

bool isEmpty(){

	return (_numFreeBlocks==_size);
}


T* addrFromIndex(int i)
{
  return mypool[i];
}

/**************************************************************************/

T* allocate() 
{ 
	T* temp = new T();
	 _numFreeBlocks--;

	if(_numFreeBlocks>0)
	{	  
	   mypool.push_back(temp);
      _next++;	
	 
	  return temp;
	}
	
	 mypool.erase(mypool.begin(),mypool.begin()+_size/2);	
	_numFreeBlocks+=_size/2;
	_next=0;
	 mypool[_next] = temp; 
	_next++;
	return temp;
} 


};