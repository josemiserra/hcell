#ifndef _SLOT_
#define _SLOT_

#include <opencv2/opencv.hpp>
#include <iostream>
#include "utils.h"




using namespace cv;

template <class IMAGE>
class Slot
{
	protected:	
		string *_id;
		IMAGE *_image;

    public:
		Slot()
		{
		_id=new string("");
		_image = nullptr;
		} 
		Slot(const char* id)
        {
		_id = new string(id);
		_image = nullptr;
		 }

		 ~Slot()
		 {
			 delete _image;
			 delete _id;
		 }

        Slot(const char * id,IMAGE image)
        {
           _id = new string(id);
		   _image = new IMAGE(image);
        }

		inline	const char* getId()
		{
		return _id->c_str();
		}

	    void setId(const char* id)
		{
	    	if(_id!=nullptr) delete id;
	    	_id=new string(id);
		}


		inline  IMAGE* getValue()
        {
            return _image;
        }

		inline  void setValue(IMAGE &image) 
		{  
		if(_image!=nullptr) delete _image;
		_image = new IMAGE(image);
		}
};

#endif // _SLOT_
