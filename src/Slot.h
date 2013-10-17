#ifndef _SLOT_
#define _SLOT_

#include <opencv2/opencv.hpp>
#include <iostream>

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
#if __linux__
	#if GCC_VERSION > 40600			
		_image = nullptr;
	#endif
#else
		_image = nullptr;
#endif
		} 
		Slot(const char* id)
        {
		_id = new string(id);
		
#if __linux__
	#if GCC_VERSION > 40600			
		_image = nullptr;
	#endif
#else
	_image = nullptr;
#endif
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
	#if __linux__
		#if GCC_VERSION > 40600		
	    	if(_id!=nullptr) delete id;
		#else
		    if(!id) delete id;
		#endif
	#else
			if(_id!=nullptr) delete id;
    #endif	
	    	_id=new string(id);
		}


		inline  IMAGE* getValue()
        {
            return _image;
        }

		inline  void setValue(IMAGE &image) 
		{  
	#if __linux__
		#if GCC_VERSION > 40600		
	    	if(_image!=nullptr) delete _image;
		#else
		    if(!image) delete _image;
		#endif
	#else
			if(_image!=nullptr) delete _image;
    #endif
		_image = new IMAGE(image);
		}
};

#endif // _SLOT_
