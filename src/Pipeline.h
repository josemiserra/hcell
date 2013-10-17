#ifndef _PIPELINE_
#define _PIPELINE_

#include <iostream>
#include <vector>
#include <bitset>
#include "Action.h"
#include "PipelineGraph.h"
#include "MAllTypes.h"
#include "Hcell.h"

#define BIT_OPTIONS 12

using namespace std;

enum FUNCTIONS
{
LOAD_FILE,
WRITE_FILE,
BASIC,
SHAPE,
MOMENT,
HARALICK,
OTHERS,
ALL
};


class Pipeline
{
protected:
	vector<Action*> _actionsList;
	vector<vector<string>> _filestoLoad; 
	vector<vector<string>> _filestoSave; 
	vector<string> _features;
	InputValues _inpval;
	vector<Action *>::const_iterator it; 
    vector<Action *>::const_iterator end;
	PipelineGraph _pgraph;
	int *event_sequence;

	void preparePipeline();
	void getFilesFromDir(const char* dir);
public:
	Pipeline(vector<Action*> actionsList,InputValues inputvalues){
	
	 _actionsList = actionsList;
	 _inpval = inputvalues;
	 _pgraph.addList(actionsList);
	 preparePipeline();

	}
	~Pipeline(void){
		while(!_actionsList.empty())
				{
				 delete _actionsList.back(), _actionsList.pop_back();
				}

	};


void start(void);
vector<string> getFilesFromDir( const char & dir,const char *regexp);
vector<string> generateOutputFileNames(const char& dir,const char *_exp1,vector<string>_exp2,const char *date,bool counter,int);
void getBasicOptions(bitset<BIT_OPTIONS> &options,vector<string> &parameterNames,vector<MType *> &parameterValues);
void getShapeOptions(bitset<BIT_OPTIONS> &options,vector<string> &parameterNames,vector<MType *> &parameterValues);
void getMomentOptions(bitset<BIT_OPTIONS> &options,vector<string> &parameterNames,vector<MType *> &parameterValues);
void getCommon(vector<vector<string>> files,vector<string> &common_names);
};

#endif
