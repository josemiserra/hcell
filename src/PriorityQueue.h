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
	 refloat(0,pq_keys.size()-1);
	 return;
	}


    /*Get the maximum element from the priority queue*/
	T top() // dequeue
	{
	//  assert(pq_keys.size() != 0);
	   int last = pq_keys.size() - 1;
		T tmp = pq_keys[0];
		pq_keys[0] = pq_keys[last];
		pq_keys[last] = tmp;
		pq_keys.pop_back();
		if(!empty()) sink(0,last-1);
		return tmp;
	}
	

inline void refloat(int low,int high)
{
 int root = low;
    while ((root*2)+1 <= high)
    {
        int leftChild = (root * 2) + 1;
        int rightChild = leftChild + 1;
        int swapIdx = root;
        /*Check if root is less than left child*/
        if (pq_keys[swapIdx] < pq_keys[leftChild])
        {
            swapIdx = leftChild;
        }
        /*If right child exists check if it is less than current root*/
        if ((rightChild <= high) && (pq_keys[swapIdx] < pq_keys[rightChild]))
        {
            swapIdx = rightChild;
        }
        /*Make the biggest element of root, left and right child the root*/
        if (swapIdx != root)
        {
            T tmp = pq_keys[root];
            pq_keys[root] = pq_keys[swapIdx];
            pq_keys[swapIdx] = tmp;
            /*Keep shifting right and ensure that swapIdx satisfies
            heap property aka left and right child of it is smaller than
            itself*/
            root = swapIdx;
        }
        else
        {
            break;
        }
    }
    return;
}
	
void sink(int low,int high)
{
	int childIdx = high;
    while (childIdx > low)
    {
        int parentIdx = (childIdx-1)/2;
        /*if child is bigger than parent we need to swap*/
        if (pq_keys[childIdx] > pq_keys[parentIdx])
        {
            T tmp = pq_keys[childIdx];
            pq_keys[childIdx] = pq_keys[parentIdx];
            pq_keys[parentIdx] = tmp;
            /*Make parent index the child and shift towards left*/
            childIdx = parentIdx;
        }
        else
        {
            break;
        }
    }
    return;
}

};

#endif
