#include "PModule.h"
#include "FileProcessing.h"
#include "ImageProcessing.h"
#include "SegmentationProcessing.h"
#include "ComputeFeatures.h"
#include <memory>
#include "utils.h"

class ModuleSelector
{
protected:
	shared_ptr<PModule> imProc;
	shared_ptr<PModule> fProc;
	shared_ptr<PModule> segProc;
	shared_ptr<PModule> cFeat;

public:
	 
	ModuleSelector()
	{
#if __linux__
	#if GCC_VERSION > 40500
		fProc=nullptr;
		imProc=nullptr;
		segProc=nullptr;
		cFeat = nullptr;
	#endif
#else
		fProc=nullptr;
		imProc=nullptr;
		segProc=nullptr;
		cFeat = nullptr;
#endif
	}
	~ModuleSelector()
	{
		// cout<< "MODULESELECTOR finished" <<endl;
	}

	
	Action* imageProc(const char *FNAME,const char **nparams,const char **params,int countP){
#if __linux__
	#if GCC_VERSION > 40500
		 if( imProc == nullptr )
	#else
		 if(!imProc)
	#endif
#else	
		 if( imProc == nullptr )
#endif
		 {
			imProc = shared_ptr<ImageProcessing>(new ImageProcessing());
		 }
		Action *action = new Action(imProc);
		action->setAction(FNAME,nparams,params,countP);
		return action;
	}
	

	Action* fileProc(const char *FNAME,const char **nparams,const char **params,int countP)
	{
#if __linux__
	#if GCC_VERSION > 40500
		 if( fProc == nullptr )
	#else
		 if(!fProc)
	#endif
#else	
		 if( fProc == nullptr )
#endif
		{
			fProc = shared_ptr<FileProcessing>(new FileProcessing());
		}
		Action *action = new Action(fProc);
		action->setAction(FNAME,nparams,params,countP);
		return action;
	}

	Action* segmentationProc(const char *FNAME,const char **nparams,const char **params,int countP)
	{
#if __linux__
	#if GCC_VERSION > 40500
		 if( segProc == nullptr )
	#else
		 if(!segProc)
	#endif
#else	
		 if( segProc == nullptr )
#endif
		{
			segProc = shared_ptr<SegmentationProcessing>(new SegmentationProcessing());
		}
		Action *action = new Action(segProc);
		action->setAction(FNAME,nparams,params,countP);
		return action;
	}

	Action* computeFeatures(const char *FNAME,const char **nparams,const char **params,int countP){
#if __linux__
	#if GCC_VERSION > 40500
		 if( cFeat == nullptr )
	#else
		 if(!cFeat)
	#endif
#else	
		 if( cFeat == nullptr )
#endif
		{
			cFeat = shared_ptr<ComputeFeatures>(new ComputeFeatures());
		}
		Action *action = new Action(cFeat);
		action->setAction(FNAME,nparams,params,countP);
		return action;
	}

};
