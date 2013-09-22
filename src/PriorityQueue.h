#ifndef _PQUEUE_
#define _PQUEUE_


#include <iostream>
#include <vector>


using namespace std;
/*A simple priority Queue class. Using a vector to store
keys ensures that data structure can grow dynamically. Also
see we have vector of int. In a real world example we will
have vector of some real items like process information for
doing scheduling based on priority.*/
template <typename T> class PriorityQueue
{
protected:
	vector<T> pq_keys;

public:
    PriorityQueue()
	{
		pq_keys.push_back(T()); //reserve element number 0
	}
	PriorityQueue(int res)
	{
		pq_keys.reserve(res+1);
		pq_keys.push_back(T()); //reserve element number 0
	}
	~PriorityQueue()
	{
	 
	};

	bool empty()
	{
		return pq_keys.size()==1;
	}

    /*Insert a new item into the priority queue*/
 inline void push(T item)
	{
	 pq_keys.emplace_back(item);
	// std::cout << "1 pix in" << std::endl;
	 refloat(pq_keys.size()-1);
	 return;
	}
    /*Get the maximum element from the priority queue*/
	T top()
	{
	//  assert(pq_keys.size() != 0);
    int last = pq_keys.size() - 1;
    T tmp = pq_keys[1];
    pq_keys[1] = pq_keys[last];
    pq_keys[last] = tmp;
    pq_keys.pop_back();
    if(!empty()) sink(1);
    return tmp;
	}
	

inline void refloat(int hole)
{
	T tmp = pq_keys[hole];
	while ( hole >1 && tmp > pq_keys[hole/2]){
		pq_keys[hole] = pq_keys[hole/2];
		hole = hole/2;
		}
		pq_keys[hole] = tmp;
    return;
}
	
void sink(int low)
{
	T temp = pq_keys[low];
	unsigned int son = low*2;
    while (son <= (pq_keys.size()-1))
    {
     //   if(son!= (pq_keys.size()-1))
	 //		 if (pq_keys[son+1]>pq_keys[son]) son++;
		
		if(pq_keys[son]>temp) 
		{
			pq_keys[low] = pq_keys[son];
			low = son;
			son = low*2;
		}
		else
		{
		 break;
		}
    }
	pq_keys[low] = temp;
    return;
}

};

#endif
