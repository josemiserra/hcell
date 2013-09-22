#include "PModule.h"
#include "FileProcessing.h"
#include "ImageProcessing.h"
#include "SegmentationProcessing.h"
#include "ComputeFeatures.h"
#include <memory>

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
		fProc=nullptr;
		imProc=nullptr;
		segProc=nullptr;
		cFeat = nullptr;
	}
	~ModuleSelector()
	{
		// cout<< "MODULESELECTOR finished" <<endl;
	}

	
	Action* imageProc(const char *FNAME,const char **nparams,const char **params,int countP){
	    
		 if( imProc == nullptr )
		 {
			imProc = shared_ptr<ImageProcessing>(new ImageProcessing());
		 }
		Action *action = new Action(imProc);
		action->setAction(FNAME,nparams,params,countP);
		return action;
	}
	

	Action* fileProc(const char *FNAME,const char **nparams,const char **params,int countP)
	{
		if(fProc==nullptr)
		{
			fProc = shared_ptr<FileProcessing>(new FileProcessing());
		}
		Action *action = new Action(fProc);
		action->setAction(FNAME,nparams,params,countP);
		return action;
	}

	Action* segmentationProc(const char *FNAME,const char **nparams,const char **params,int countP)
	{
		if(segProc==nullptr)
		{
			segProc = shared_ptr<SegmentationProcessing>(new SegmentationProcessing());
		}
		Action *action = new Action(segProc);
		action->setAction(FNAME,nparams,params,countP);
		return action;
	}

	Action* computeFeatures(const char *FNAME,const char **nparams,const char **params,int countP){
		if(cFeat==nullptr)
		{
			cFeat = shared_ptr<ComputeFeatures>(new ComputeFeatures());
		}
		Action *action = new Action(cFeat);
		action->setAction(FNAME,nparams,params,countP);
		return action;
	}

};
