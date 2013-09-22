#include "Writer.h"
#include <stdio.h>
#include <string.h>

FeaturesPipe* Writer::getFeaturePipe(const char* reference)
{
	    // first find the reference
		for(vector<FeaturesPipe*>::iterator it= this->_fpipe.begin(); it!=this->_fpipe.end();++it)
		{
			if(strcmp((*it)->_reference,reference)==0)
			{
			return (*it);
			}
		}

	    return nullptr;
	}
