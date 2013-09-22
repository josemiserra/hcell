
#include <regex>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <vector>
#include <string>
#include <iostream>

#include "Pipeline.h"

#include "General.h"
#include "utils.h"
#include "Writer.h"
#include "ComputeFeatures.h"


#ifdef _WIN32
    #include <unordered_map>
    #include <memory>
#else
    #include <tr1/unordered_map>
    #include <tr1/memory>
#endif

using namespace std;


/******************************************************
*  In prepareline all initializations of actions after the 
* reading are done. 
********************************************************/

void Pipeline::preparePipeline(void){
	
	ut::Trace tr = ut::Trace("PIPELINE:preparePipeline",__FILE__);

	vector<vector<string>> _filestoLoad;
	vector<vector<string>> _filestoSave;
	
	vector<string> *parameterNames;
	vector<MType *> *parameterValues;

	vector<string>::iterator parn_it;
    vector<MType *>::iterator par_it;
	
	event_sequence = new int[this->_actionsList.size()+1];


	map<string,FeaturesPipe*> npipes;

	bool features_found = false;

	// This function takes an actions list and build a graph based on it
	// Under development for next versions. Now empty.
	this->_pgraph.buildgraph();

	bool basic,shape, moment, haralick, others;
	string myRegExp;
	string fname;
	string action_name;
	string hname;

	int ca = 0;
	bitset<BIT_OPTIONS> opts;

	it = this->_actionsList.begin(); 
	end = _actionsList.end();

	 while ( it != end ) {

		  action_name = (*it)->getCurrentAction();

/***************************************************************************************************
*  LOADING FILES FROM DIRECTORY
*  
****************************************************************************************************/
		 if(action_name.compare("LOAD FILE")==0)
		 {
			  // Getting the regular expression
			 parameterNames=&(*it)->getParameterNames();
			 parameterValues = &(*it)->getParameters();
			 for(parn_it=parameterNames->begin(),par_it = parameterValues->begin();
				 parn_it!=parameterNames->end();
				 parn_it++,par_it++)
			 {
					 if((*parn_it).compare("REGEXP")==0)
					 {
						 myRegExp=(dynamic_cast<MStringType *>(*par_it))->getValue();						 
						 _filestoLoad.push_back(this->getFilesFromDir(*_inpval.indir,myRegExp.c_str()));
					 }

			 }
			 event_sequence[ca]= FUNCTIONS::LOAD_FILE;
			 ca++; // counter for events
			  ++it; // counter for actions
			 continue;
		 }
/***************************************************************************************************
* WRITING IMAGES AND GENERATING THEIR FILENAMES
*  
****************************************************************************************************/

		 if(action_name.compare("WRITE FILE")==0)
		 {
			 
			 string wdate  ="";
			 string wappend ="";
			 string foutput_name;
			 const char* reference;
			 bool append = false;
			 bool counter = true;
			 parameterNames=&(*it)->getParameterNames();
			 parameterValues = &(*it)->getParameters();
			 for(parn_it=parameterNames->begin(),par_it = parameterValues->begin();
				 parn_it!=parameterNames->end();
				 parn_it++,par_it++)
				 {

					 if((*parn_it).compare("APPEND_DATE")==0)
					{
						
						if((dynamic_cast<MBoolType *>(*par_it))->getValue())
						{
							wdate = tr.currentDateTime();

						}
					}
					if((*parn_it).compare("APPEND_ORIGINAL_NAME")==0)
					{
					    if((dynamic_cast<MBoolType *>(*par_it))->getValue())
					    {
						append = true;
						counter = false;
						}
					}
					if((*parn_it).compare("FOUTPUT")==0)
					{
						foutput_name=(dynamic_cast<MStringType *>(*par_it))->getValue();
					 }
					if((*parn_it).compare("INPUT")==0)
					{
						  reference=(dynamic_cast<MStringType *>(*par_it))->getValue();
						  reference = this->_pgraph.traceBack(string(reference),*(*it)).c_str();
					 }
				}
			
			 if(append){
				wappend = reference;
			 }
			
			 int total_files_out = _filestoLoad[0].size();
			 _filestoSave.push_back(this->generateOutputFileNames(*_inpval.outdir,foutput_name.c_str(),wappend.c_str(),wdate.c_str(),counter,total_files_out));
			

			 event_sequence[ca]= FUNCTIONS::WRITE_FILE;
			 ca++;
			  ++it;
			 continue;

		 }
/**********************************************************************************************
*   COMPUTE FEATURES 
*   Create the feature files.
*		    There is one general file which store general features per image.
*			Optionally, another file with data can store individual objects statistics.
***********************************************************************************************/
			basic = action_name.compare("BASIC")==0;
			shape = action_name.compare("SHAPE")==0;
			moment = action_name.compare("MOMENT")==0;
			haralick = action_name.compare("HARALICK")==0;					
			others = action_name.compare("OTHERS")==0;
						
			// check if my action in the list is a write file
			if( basic | shape | moment | haralick |others) 
			{
			 features_found=true;
			 parameterNames=&(*it)->getParameterNames();
			 parameterValues = &(*it)->getParameters();
			
			 if(basic)
			 {
						event_sequence[ca]= FUNCTIONS::BASIC;
						getBasicOptions(opts,*parameterNames,*parameterValues);
			 }
			 if(shape)
			 {
				 event_sequence[ca]= FUNCTIONS::SHAPE;
				 this->getShapeOptions(opts,*parameterNames,*parameterValues);
			 }
			 if(moment)
			 {
				 event_sequence[ca]= FUNCTIONS::MOMENT;
				 this->getMomentOptions(opts,*parameterNames,*parameterValues);
			 }
			 if(haralick)
			 {
				 event_sequence[ca]= FUNCTIONS::HARALICK;
				// Texture files saved individually
			 }
			 if(others)
			 {
				 event_sequence[ca]= FUNCTIONS::OTHERS;	
				 // to determine
			 }
			 
			 string origin;
			 string wdate  ="";
			 string wappend ="";
			 string foutput_name;
			 string ref;

			 bool save_feat = false;
			 bool append = false;
			 bool counter = true;
			 string str;

			 for(parn_it=parameterNames->begin(),par_it = parameterValues->begin();
				 parn_it!=parameterNames->end();
				 parn_it++,par_it++)
				{
					
					 if((*parn_it).compare("APPEND_DATE")==0)
					 {
						if((dynamic_cast<MBoolType *>(*par_it))->getValue())
						{
							wdate = tr.currentDateTime();
						}
					  }
					if((*parn_it).compare("APPEND_ORIGINAL_NAME")==0)
					{
					    if((dynamic_cast<MBoolType *>(*par_it))->getValue())
						{
						append = true;
						counter = false;
						}
					}
					if((*parn_it).compare("FOUTPUT")==0)
					{
						hname=(dynamic_cast<MStringType *>(*par_it))->getValue();

						 
						 str.append(this->_inpval.outdir);
						 str.append("/");
						 str.append(hname);


					 }
					 if((*parn_it).compare("SAVE_INDIVIDUAL")==0)
					 {
						 save_feat =((dynamic_cast<MBoolType *>(*par_it))->getValue());
					 } // endif
					 if((*parn_it).compare("REFERENCE")==0)
					 {
					  	ref = (dynamic_cast<MStringType *>(*par_it))->getValue();
					 }

				} // endfor


			   if(haralick)
			   {

				        vector<string> wh_filestoSaveFeatures;
				        wh_filestoSaveFeatures=(this->generateOutputFileNames(*_inpval.outdir,"FEATURES_HARALICK",wappend.c_str(),wdate.c_str(),counter,_filestoLoad[0].size()));

				        FeaturesPipe *fhar = new FeaturesPipe;
						fhar->_count=0;
						fhar->setReference(hname);
						fhar->_total=1;
						fhar->_fcount=0;
						for (vector<string>::iterator it = wh_filestoSaveFeatures.begin() ; it != wh_filestoSaveFeatures.end(); ++it)
						{
												   fhar->_ind_filenames.push_back((*it));
						}
						 npipes.insert(std::pair<string,FeaturesPipe*>(hname,fhar));
						 haralick=false;
				   }


		else{
			 // If we already have created the filenames, we dont want to do it again
			 // The reference parameter serves as a reference
			if(npipes.count(ref)==0)
			 {
				  
				   if(append)
					{
						wappend = ref;
						// get the original file name
					}
				   vector<string> _filestoSaveFeatures;
					
					_filestoSaveFeatures=(this->generateOutputFileNames(*_inpval.outdir,"FEATURES",wappend.c_str(),wdate.c_str(),counter,_filestoLoad[0].size()));
					

				   FeaturesPipe *fp = new FeaturesPipe;
				   fp->_count=0;
				   fp->_options = opts;
				   fp->setReference(ref);
				   fp->setGenFilename(str);
				   fp->fgout.open(fp->_gen_filename);
				   fp->_total=1;
				   fp->_fcount=0;
				   npipes.insert(pair<string,FeaturesPipe*>(ref,fp));

					for (vector<string>::iterator it = _filestoSaveFeatures.begin() ; it != _filestoSaveFeatures.end(); ++it)
					{
						   fp->_ind_filenames.push_back((*it));
					}
			 }
			 else
			 {
			    FeaturesPipe *fp = npipes[ref];
				fp->_total++;
				fp->_options = (opts | fp->_options);
			 }
			 
			   } // end else haralick
			  ++ca;
			  ++it;
			 continue;
			 
		 } // end if Features
		
			this->event_sequence[ca]=FUNCTIONS::FOO;
            ++ca;
			++it;
      } 
	

if(features_found)
{
	  Writer *writer = new Writer();
	  for(auto it = npipes.begin(); it!=npipes.end();++it)
	  {
		
		  writer->addPipe((*it).second);
	  }

	  writer->buildHeaders();
	  ComputeFeatures::featureWriter = writer;
}

	   string current_fname;

	   for (vector<vector<string>>::iterator it = _filestoLoad.begin() ; it != _filestoLoad.end(); ++it)
	   {
		   vector<string> temp_load;
		   for (vector<string>::iterator it2 = it->begin() ; it2 != it->end(); ++it2)
		   {
				current_fname = this->_inpval.indir;
				current_fname.append(*it2);
				temp_load.push_back(current_fname);
				current_fname.clear();
		   }	
		   this->_filestoLoad.push_back(temp_load);
	   }

	// Same with the files to Save 
	   current_fname.clear();

	   for (vector<vector<string>>::iterator it = _filestoSave.begin() ; it != _filestoSave.end(); ++it)
	   {
		   vector<string> temp_load;
		   for (vector<string>::iterator it2 = it->begin() ; it2 != it->end(); ++it2)
		   {
			   current_fname = *it2;
			   temp_load.push_back(current_fname);
				current_fname.clear();
		   }	
		   this->_filestoSave.push_back(temp_load);
	   }
	
     // Add one more for the cleaning
	   this->event_sequence[ca]=FUNCTIONS::FOO;


	    shared_ptr<General> g(new General());
		Action *cleaner= new Action(g);
		cleaner->setAction("CLEAN",NULL,NULL,0);
		this->_actionsList.push_back(cleaner);

	} 

void Pipeline::start(void){
	   
	    ut::Trace tr = ut::Trace("PIPELINE:start",__FILE__);

	   /*
	    Usually we load from different files, like 2, 3 or more dyes. 
	    total_loads is the total number of modules like (.*)DAPI.tiff and (.*)FITC.tiff could correspond to 2 modules Load File . 
		total_files is the total number of files

		But then we need to count ant anytime in which stage are we.
		cfiles counts for each file in which load stage is it cfile[560] = 0, then 1, ...

	   */

	   unsigned int total_loads = _filestoLoad.size();
	   unsigned int total_files = _filestoLoad[0].size();
	   int *c_files = new int[total_loads];
	   int *c_files_out = new int[total_loads];
	 //  int *c_files_out_feat = new int[total_files];
	   string current_fname, main_fname;


	   for(unsigned int i=0;i<_filestoLoad.size();i++)
	   {
		   if(_filestoLoad[i].size()!=total_files){
		   
			   tr.message("ERROR: unequal number of files. Must supply same number of files for every channel");
			   exit(-1908);
		   }
	   }

	   end = _actionsList.end();

	   // Counters for files start here
	   for(int i=0;i<total_loads;i++)
	   {
			c_files[i]=0;
			c_files_out[i]=0;
	   }
	    
		int n,no, total_actions;
		int *nf = new int[5];
		string action_name;
		int action;




		tr.message("Pipeline starting...");
		for(int k=0;k<total_files;k++)
		{
		    it = this->_actionsList.begin();  //refresh my list
			cout<<".."<< k+1 <<" of "<<total_files<<endl;
			// counters
			n=0;
			no = 0;
			memset(nf,0,sizeof(int)*5);
			total_actions = 0;
			while ( it != end )
			{
			    
				 action_name=(*it)->getCurrentAction();
				 tr.message("Executing :",action_name.c_str());
				 action = this->event_sequence[total_actions];
/**********************************************************************************************
*  LOAD FUNCTIONS
*
*
***********************************************************************************************/
				// check if my action in the list is a load file
				 if(action == FUNCTIONS::LOAD_FILE)
				{
						// pop the file
				  	    current_fname = this->_filestoLoad[n][c_files[n]];
						(c_files[n])++;
						n++;
						tr.message("Loading filename:",current_fname.c_str());
						if(k>0)
						{
						   (*it)->modifyParameterValue("FILENAME", new MStringType(current_fname,"FILENAME"));
						}
						else
						{
						  (*it)->addParameter("FILENAME", current_fname.c_str()); 
						}				
				}
/**********************************************************************************************
*  WRITE IMAGE FILE
*
*
***********************************************************************************************/
				else
				{
					// check if my action in the list is a write file
					if(action == FUNCTIONS::WRITE_FILE)
					 {
						 // pop the file
						current_fname = this->_filestoSave[no][c_files_out[no]];
						(c_files_out[no])++;
						no++;
						tr.message("Writing image to:",current_fname.c_str());
						if(k>0)
						{
							(*it)->modifyParameterValue("FILENAME", new MStringType(current_fname,"FILENAME"));
						}
						else
						{			
						     (*it)->addParameter("FILENAME", current_fname.c_str()); 
						}
					}
				 }
	  
				  (*it)->execute(k); 
				  ++it;
				  total_actions++;
					}

	   
	   } // end while

delete [] nf;
delete [] c_files;
delete [] c_files_out;
delete [] event_sequence;

} // end start Pipeline



vector<string> Pipeline::getFilesFromDir(const char& dir,const char *_regexp)
{

	vector<string> filestopipeline;
	regex rx(_regexp,std::regex_constants::egrep);

	string hi;
	char *_dir = (char *)malloc(250);
	
	
	smatch res;

	strcpy( _dir, &dir);
	//strcat(_dir,"/");


    DIR *dp;
    struct dirent *dirp;
    if((dp  = opendir(_dir)) == NULL) {
        cout << "Error in directory (" << errno << ") opening " << dir << endl;
        exit(-2018);
    }

    while ((dirp = readdir(dp)) != NULL)
    {
        hi = string(dirp->d_name);
        if(regex_match(hi,res,rx))
        {
        	string n = "/";
        	n.append(hi);
        	filestopipeline.push_back(n);
        	cout << "Matched true \""<< res.str() <<"\""<< endl;
        	cout << hi << endl;
        }

    }
    closedir(dp);
    if(filestopipeline.size()==0)
    {
    	 cout << "Error, files with regular expression "<<_regexp<<" not found!!  "<< endl;
    	 exit(-1959);
    }
    delete [] _dir;
	return filestopipeline;
    }



vector<string> Pipeline::generateOutputFileNames(const char& dir,const char *_exp1,const char *_exp2,const char *date,bool counter,int tf)
{
	vector<string> filestosave;
	char *_dir = (char *)malloc(512);
	
  
	int count = 0;
	int totalOuts = tf;
	string scount="";
	for(count=0;count<totalOuts;count++)
	{
	    strcpy( _dir, &dir);
	    strcat( _dir, "/");
		strcat(_dir,_exp1);
		strcat(_dir,"_");
		strcat(_dir,_exp2);
		strcat(_dir,"_");
		if(counter)
		{
			scount = to_string(count);
			strcat(_dir,scount.c_str());
		}
		strcat(_dir,"_");
		strcat(_dir,date);
		filestosave.push_back(_dir);
		memset(_dir,0,512);
	}
	delete [] _dir;
	return filestosave;
}

void Pipeline::getBasicOptions(bitset<BIT_OPTIONS> &options,vector<string> &parameterNames,vector<MType *> &parameterValues)
{
	vector<MType *>::iterator par_it = parameterValues.begin();		 
		for(vector<string>::iterator parn_it= parameterNames.begin(); parn_it!= parameterNames.end(); parn_it++,par_it++)
			{			
					if((*parn_it).compare("MEAN")==0)
					 {
						if((dynamic_cast<MBoolType *>(*par_it))->getValue())
							options|=options.set(0);
						continue;
					 }
					 if((*parn_it).compare("SD")==0)
					 {	
						if((dynamic_cast<MBoolType *>(*par_it))->getValue())
							options|=options.set(1);
						continue;
					 }
					 if((*parn_it).compare("MAD")==0)
					 {
						
						if((dynamic_cast<MBoolType *>(*par_it))->getValue())
							options|=options.set(2);
						continue;
					 }
					 if((*parn_it).compare("QUANTILES")==0)
					 {
						
						if((dynamic_cast<MBoolType *>(*par_it))->getValue())
							options|=options.set(3);
						continue;
					 }
			 }
}

void Pipeline::getShapeOptions(bitset<BIT_OPTIONS> &options,vector<string> &parameterNames,vector<MType *> &parameterValues)
{
	vector<MType *>::iterator par_it = parameterValues.begin();		 
		for(vector<string>::iterator parn_it= parameterNames.begin(); parn_it!= parameterNames.end(); parn_it++,par_it++)
			{			
					if((*parn_it).compare("AREA")==0)
					 {
						if((dynamic_cast<MBoolType *>(*par_it))->getValue())
							options|=options.set(4);
						continue;
					 }
					 if((*parn_it).compare("PERIMETER")==0)
					 {	
						if((dynamic_cast<MBoolType *>(*par_it))->getValue())
							options|=options.set(5);
						continue;
					 }
					 if((*parn_it).compare("RADIUS")==0)
					 {
						
						if((dynamic_cast<MBoolType *>(*par_it))->getValue())
							options|=options.set(6);
						continue;
					 }
					 if((*parn_it).compare("ROUNDNESS")==0)
					 {
						
						if((dynamic_cast<MBoolType *>(*par_it))->getValue())
							options|=options.set(7);
						continue;
					 }
			 }
}


void Pipeline::getMomentOptions(bitset<BIT_OPTIONS> &options,vector<string> &parameterNames,vector<MType *> &parameterValues)
{
	vector<MType *>::iterator par_it = parameterValues.begin();		 
		for(vector<string>::iterator parn_it= parameterNames.begin(); parn_it!= parameterNames.end(); parn_it++,par_it++)
			{			
					if((*parn_it).compare("CENTROID")==0)
					 {
						if((dynamic_cast<MBoolType *>(*par_it))->getValue())
							options|=options.set(8);
						continue;
					 }
					 if((*parn_it).compare("AXIS")==0)
					 {	
						if((dynamic_cast<MBoolType *>(*par_it))->getValue())
							options|=options.set(9);
						continue;
					 }
					 if((*parn_it).compare("ECCENTRICITY")==0)
					 {
						
						if((dynamic_cast<MBoolType *>(*par_it))->getValue())
							options|=options.set(10);
						continue;
					 }
					 if((*parn_it).compare("THETA")==0)
					 {
						
						if((dynamic_cast<MBoolType *>(*par_it))->getValue())
							options|=options.set(11);
						continue;
					 }
			 }
}
