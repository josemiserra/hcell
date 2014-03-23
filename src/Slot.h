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
			IMAGE *old_image;
			old_image = _image;
			_image = new IMAGE(image);
	#if __linux__
		#if GCC_VERSION > 40600		
	    	if(old_image!=nullptr) delete old_image;
		#else
		    if(!old_image) delete old_image;
		#endif
	#else
			if(old_image!=nullptr && &image!=old_image){ delete old_image; }
    #endif
		
		
		
		
		}
};

#endif // _SLOT_
