#include <boost/regex.hpp>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <vector>
#include <string>
#include <iostream>

#include "Pipeline.h"

#include "PModule.h"
#include "General.h"
#include "utils.h"
#include "Writer.h"
#include "ComputeFeatures.h"
#include "Checker.h"

using namespace std;


/******************************************************
*  In prepareline all initializations of actions after the 
* reading are done. 
********************************************************/

void Pipeline::preparePipeline(void){
	
	ut::Trace tr = ut::Trace("PIPELINE:preparePipeline",__FILE__);

	vector<vector<string>> _filestoLoad;
	vector<vector<string>> _filestoSave;
	
	
	vector<MType *> *parameterValues;
    vector<MType *>::iterator par_it;
	
	event_sequence = new int[this->_actionsList.size()+1];

	map<string,FeaturesPipe*> npipes;

	bool features_found = false;
	bool set_all = false;
	bool a_basic=false;
	bool a_moment=false;
	bool a_shape=false;
	bool a_haralick=false;
	int total_c;
	string n_name("HAR_");
	bool basic,shape, moment, haralick, all;
	string myRegExp;
	string fname;
	string action_name;
	string oname;

	int ca = 0;
	bitset<BIT_OPTIONS> opts;

	it = this->_actionsList.begin(); 
	end = _actionsList.end();
    shared_ptr<General> g(new General());

	 while ( it != end ) {

		  action_name = (*it)->getCurrentAction();

/***************************************************************************************************
*  LOADING FILES FROM DIRECTORY
*  
****************************************************************************************************/
		 if(action_name.compare("LOAD FILE")==0)
		 {
			  // Getting the regular expression
			 parameterValues = &(*it)->getParameters();
			 for(par_it = parameterValues->begin(); par_it!=parameterValues->end(); par_it++)
			 {
					 if((*par_it)->getTag().compare("REGEXP")==0)
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
			 vector<string> wappend;
			 string foutput_name;
			 string reference;
			 bool append = false;
			 bool counter = true;
			 parameterValues = &(*it)->getParameters();
			 for(par_it = parameterValues->begin();par_it!=parameterValues->end();par_it++)
				 {
					 if((*par_it)->getTag().compare("APPEND_DATE")==0)
					{
						
						if((dynamic_cast<MBoolType *>(*par_it))->getValue())
						{
							wdate = tr.currentDateTime();

						}
					}
					if((*par_it)->getTag().compare("APPEND_ORIGINAL_NAME")==0)
					{
					    if((dynamic_cast<MBoolType *>(*par_it))->getValue())
					    {
						append = true;
						//counter = false;
						}
					}
					if((*par_it)->getTag().compare("FOUTPUT")==0)
					{
						foutput_name=(dynamic_cast<MStringType *>(*par_it))->getValue();
					 }
					if((*par_it)->getTag().compare("INPUT")==0)
					{
						  reference=(dynamic_cast<MStringType *>(*par_it))->getValue();
     				 }
				}
			
			 if(append){
				getCommon(_filestoLoad,wappend);
			 }
			
			 int total_files_out = _filestoLoad[0].size();
			 _filestoSave.push_back(this->generateOutputFileNames(*_inpval.outdir,foutput_name.c_str(),wappend,wdate.c_str(),counter,total_files_out));
			

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
			all = action_name.compare("ALL FEATURES")==0;
						
			// check if my action in the list is a write file
			if( basic | shape | moment | haralick |all) 
			{
			 features_found=true;
			 parameterValues = &(*it)->getParameters();
			
			if(all)
			 {
				total_c=0;
				event_sequence[ca]= FUNCTIONS::ALL;
				for(par_it = parameterValues->begin();par_it!=parameterValues->end();par_it++)
				{
					
					if((*par_it)->getTag().compare("BASIC")==0) 
					{ 
						a_basic=((dynamic_cast<MBoolType *>(*par_it))->getValue()); 
						total_c++;
						opts|=opts.set(0);
						opts|=opts.set(1);
						opts|=opts.set(2);
						opts|=opts.set(3);
					}
					if((*par_it)->getTag().compare("MOMENT")==0)
					{ 
						a_moment=((dynamic_cast<MBoolType *>(*par_it))->getValue());
						total_c+=2;
						opts|=opts.set(8);
						opts|=opts.set(9);
						opts|=opts.set(10);
						opts|=opts.set(11);
					
					}
					if((*par_it)->getTag().compare("SHAPE")==0)
					{
						a_shape=((dynamic_cast<MBoolType *>(*par_it))->getValue()); 
						total_c++;
						opts|=opts.set(4);
						opts|=opts.set(5);
						opts|=opts.set(6);
						opts|=opts.set(7);								
					}
					if((*par_it)->getTag().compare("HARALICK")==0){ a_haralick=((dynamic_cast<MBoolType *>(*par_it))->getValue());  }
				}
				set_all = true;
			 }
			 if(basic)
			 {
						event_sequence[ca]= FUNCTIONS::BASIC;
						getBasicOptions(opts,*parameterValues);
			 }
			 if(shape)
			 {
				 event_sequence[ca]= FUNCTIONS::SHAPE;
				 this->getShapeOptions(opts,*parameterValues);
			 }
			 if(moment)
			 {
				 event_sequence[ca]= FUNCTIONS::MOMENT;
				 this->getMomentOptions(opts,*parameterValues);
			 }
			 if(haralick)
			 {
				 event_sequence[ca]= FUNCTIONS::HARALICK;
				// Texture files saved individually
			 }
			 
			 string origin;
			 string wdate;
			 vector<string> wappend;
			 
			 string foutput_name;
			 string ref;

			 bool save_feat = false;
			 bool append = false;
			 bool counter = true;
			 string str;

			 for(par_it = parameterValues->begin(); par_it!=parameterValues->end(); par_it++)
				{
					
					 if((*par_it)->getTag().compare("APPEND_DATE")==0)
					 {
						if((dynamic_cast<MBoolType *>(*par_it))->getValue())
						{
							wdate = tr.currentDateTime();
						}
					  }
					if((*par_it)->getTag().compare("APPEND_ORIGINAL_NAME")==0)
					{
					    if((dynamic_cast<MBoolType *>(*par_it))->getValue())
						{
						append = true;
						//counter = false;
						}
					}
					if((*par_it)->getTag().compare("FOUTPUT")==0)
					{
						fname=(dynamic_cast<MStringType *>(*par_it))->getValue();
						 str.append(this->_inpval.outdir);
						 str.append("/");
						 oname.append(str);
						 oname.append(fname);


					 }
					 if((*par_it)->getTag().compare("SAVE_INDIVIDUAL")==0)
					 {
						 save_feat =((dynamic_cast<MBoolType *>(*par_it))->getValue());
					 } // endif
					 if((*par_it)->getTag().compare("REFERENCE")==0)
					 {
					  	ref = (dynamic_cast<MStringType *>(*par_it))->getValue();
					 }

				} // endfor

				
				  if(all)
			   {
			    haralick = a_haralick;
			    basic= a_basic;
				shape= a_shape;
				moment= a_moment;
				
			   }  

			   if(haralick)
			   {

				        vector<string> wh_filestoSaveFeatures;
						if(append) getCommon(_filestoLoad,wappend);
				        wh_filestoSaveFeatures=(this->generateOutputFileNames(*_inpval.outdir,"FEATURES_HARALICK",wappend,wdate.c_str(),counter,_filestoLoad[0].size()));

				        FeaturesPipe *fhar = new FeaturesPipe;
						fhar->_count=0;
						if(all)
						{ 
							n_name.append(fname);
							fhar->setReference(n_name);
						} else
						{	
							fhar->setReference(fname);
						}
						fhar->_total=1;
						fhar->_fcount=0;
						for (vector<string>::iterator it = wh_filestoSaveFeatures.begin() ; it != wh_filestoSaveFeatures.end(); ++it)
						{
												   fhar->_ind_filenames.push_back((*it));
						}
						
					if(all)
						 {
							
							npipes.insert(std::pair<string,FeaturesPipe*>(n_name,fhar));
							n_name.clear();
							n_name.append("HAR_");
						 }
						 else
							 npipes.insert(std::pair<string,FeaturesPipe*>(fname,fhar));
						 haralick=false;
				   }


		
			 // If we already have created the filenames, we dont want to do it again
			 // The reference parameter serves as a reference
	
if(basic | shape | moment){
	if(npipes.count(oname)==0)
			 {
				  
				   if(append)
					{
						getCommon(_filestoLoad,wappend);
						// get the original file name
					}
				   vector<string> _filestoSaveFeatures;
					
					_filestoSaveFeatures=(this->generateOutputFileNames(*_inpval.outdir,"FEATURES",wappend,wdate.c_str(),counter,_filestoLoad[0].size()));
					

				   FeaturesPipe *fp = new FeaturesPipe;
				   fp->_count=0;
				   fp->_options = opts;
				   if(all)  fp->_options = opts;
				   fp->setReference(fname);
				   fp->setGenFilename(oname);
				   fp->fgout.open(fp->_gen_filename);
				   fp->_total=1;
				    if(all) fp->_total = total_c;
				   fp->_fcount=0;
				   npipes.insert(pair<string,FeaturesPipe*>(string(oname),fp));

					for (vector<string>::iterator it = _filestoSaveFeatures.begin() ; it != _filestoSaveFeatures.end(); ++it)
					{
						   fp->_ind_filenames.push_back((*it));
					}
			 }
			 else
			 {
			    FeaturesPipe *fp = npipes[oname];
				fp->_total++;
				fp->_options = (opts | fp->_options);
			 }
			  oname.clear();
			   } // end else haralick
			  ++ca;
			  ++it;
			 continue;
			 
		 } // end if Features
		
			this->event_sequence[ca]=FUNCTIONS::OTHERS;
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

	if(set_all)
	  {
		writer->buildHeaders(FEATURES::H_ALL);  // basic, shape, momen
	  }
	  else
	  {
		writer->buildHeaders();
	  }
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
	   this->event_sequence[ca]=FUNCTIONS::OTHERS;


		Action *cleaner= new Action(g);
		cleaner->setAction("CLEAN",NULL,NULL,0);
		this->_actionsList.push_back(cleaner);

   // Now, perform an iteration...a fake one.
	 int64 t1,t2;
	 t1= tr.GetTimeMs64();
     PModule::setShowOff();
	 this->simulate();	
	 PModule::setShowOn();
	 t2 = tr.GetTimeMs64();
	
	 if(this->_inpval.noshow) PModule::setShowOff();

	 cout << "ESTIMATED TIME: " << (_filestoLoad[0].size()*((t2-t1)/1000.0)) << " seconds." << endl;
	
	} 

void Pipeline::simulate()
	{	   
	    ut::Trace tr = ut::Trace("PIPELINE:simulate",__FILE__);

		string param_error;
	    Checker checker;
	
	   int total_loads = _filestoLoad.size(); 
	   int total_files = _filestoLoad[0].size();

	   if(total_files == 0)
	   {
	     tr.message("ERROR: No files found in the directory with the provided description.");
	     exit(-1907);
	   }
	   for(size_t i=0;i<_filestoLoad.size();i++)
	   {
		   if(_filestoLoad[i].size()!=total_files){
		   
			   tr.message("ERROR: unequal number of files. Must supply same number of files for every channel");
			   exit(-1908);
		   }
	   }


	   end = _actionsList.end();

		int n,no, total_actions;
		int *nf = new int[5];
		string action_name;
		int action;
		tr.message("Pipeline simulation starting...");
			
		it = this->_actionsList.begin();  //refresh my list
		total_actions = 0;
	//	tr.traceIsActive=false;
		while ( it != end )
		{

			     action_name = (*it)->getCurrentAction();
				 tr.message("Simulating :",action_name.c_str());
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
				  	    string current_fname = this->_filestoLoad[0][0];
						tr.message("Loading filename:",current_fname.c_str());
						(*it)->addParameter("FILENAME", current_fname.c_str()); 
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
						string current_fname = this->_filestoSave[0][0];
						(*it)->addParameter("FILENAME", current_fname.c_str()); 
					  }
				 }
				  
				  
				 if(!checker.checkAction(*(*it), param_error))
				 {		   
					 cout<< endl << "  ERROR: In parameters on function "<<action_name<<":"<< endl;
					 cout<< param_error <<endl;
					 exit(-2030);
				} 
				 /* Check if my action in the list is a write file
				    Since is a simulation, all functions that write can check parameters
				    but cannot be executed.
					This is a limitation, but the other options are:
						1) Delete everything in the same output directory...dangerous for the user!!
						2) Delete the files generated in the first iteration.
						   This option will be implemented in the version 0.8.
				  */
				 if(action != FUNCTIONS::WRITE_FILE && action != FUNCTIONS::ALL 
					 && action!= FUNCTIONS::BASIC   &&  action != FUNCTIONS::HARALICK
					 && action != FUNCTIONS ::MOMENT && action != FUNCTIONS::SHAPE) 
				     { (*it)->execute(0); } 
				  ++it;
				  total_actions++;
	   } // end while
		// Delete all files from Output?
		tr.message("You don't want to hear what I want to say...");
		tr.message("Simulation succesfully executed");
	
	delete [] nf;
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
	 //  int *c_files_out_feat = new int[total_files];
	   string current_fname, main_fname;
	   end = _actionsList.end();

	   // Counters for files start here
	   for(unsigned int i=0;i<total_loads;i++)
	   {
			c_files[i]=0;
	   }
	    
	   int total_writes = _filestoSave.size();
	   int *c_files_out = new int[total_writes];
	   for(int i=0;i< total_writes;i++)
	   { 
	     c_files_out[i]=0;
	   }
		int n,no, total_actions;
		int *nf = new int[5];
		string action_name;
		int action;




		tr.message("Pipeline starting...");
		for(size_t k=0;k<total_files;k++)
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
						(*it)->modifyParameterValue("FILENAME", new MStringType(current_fname,"FILENAME"));			
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
						(*it)->modifyParameterValue("FILENAME", new MStringType(current_fname,"FILENAME"));
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

	string hi;
	char *_dir = new char[512];
	strcpy( _dir, &dir);
	//strcat(_dir,"/");


    DIR *dp;
    struct dirent *dirp;
    if((dp  = opendir(_dir)) == NULL) {
        cout << "Error in directory (" << errno << ") opening " << dir << endl;
        exit(-2018);
    }


    boost::regex rx(_regexp);
    boost::cmatch res;

    while ((dirp = readdir(dp)) != NULL)
    {
        hi = string(dirp->d_name);


       if(boost::regex_match(hi.c_str(), res, rx))
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



vector<string> Pipeline::generateOutputFileNames(const char& dir,const char *_exp1,vector<string>_exp2,const char *date,bool counter,int tf)
{
	vector<string> filestosave;
	char *_dir = new char[512];
	bool appnam= (!_exp2.empty());
  
	int count = 0;
	int totalOuts = tf;
	string scount="";
	for(count=0;count<totalOuts;count++)
	{
	    strcpy( _dir, &dir);
	    strcat( _dir, "/");
		strcat(_dir,_exp1);
	if(appnam)
	{
		strcat(_dir,"_");
		strcat(_dir,_exp2[count].c_str());
	}
		strcat(_dir,"_");
		if(counter)
		{
#if GCC_VERSION > 40500
			scount = to_string(count);
#else
			scount = to_string(static_cast<long long>(count));
#endif

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

void Pipeline::getBasicOptions(bitset<BIT_OPTIONS> &options,vector<MType *> &parameterValues)
{
		 
		for(vector<MType *>::iterator par_it = parameterValues.begin();	par_it!= parameterValues.end(); par_it++)
			{			
					if((*par_it)->getTag().compare("MEAN")==0)
					 {
						if((dynamic_cast<MBoolType *>(*par_it))->getValue())
							options|=options.set(0);
						continue;
					 }
					 if((*par_it)->getTag().compare("SD")==0)
					 {	
						if((dynamic_cast<MBoolType *>(*par_it))->getValue())
							options|=options.set(1);
						continue;
					 }
					 if((*par_it)->getTag().compare("MAD")==0)
					 {
						
						if((dynamic_cast<MBoolType *>(*par_it))->getValue())
							options|=options.set(2);
						continue;
					 }
					 if((*par_it)->getTag().compare("QUANTILES")==0)
					 {
						
						if((dynamic_cast<MBoolType *>(*par_it))->getValue())
							options|=options.set(3);
						continue;
					 }
			 }
}

void Pipeline::getShapeOptions(bitset<BIT_OPTIONS> &options,vector<MType *> &parameterValues)
{
		 
		for(vector<MType *>::iterator par_it = parameterValues.begin(); par_it!= parameterValues.end(); par_it++)
			{			
					if((*par_it)->getTag().compare("AREA")==0)
					 {
						if((dynamic_cast<MBoolType *>(*par_it))->getValue())
							options|=options.set(4);
						continue;
					 }
					 if((*par_it)->getTag().compare("PERIMETER")==0)
					 {	
						if((dynamic_cast<MBoolType *>(*par_it))->getValue())
							options|=options.set(5);
						continue;
					 }
					 if((*par_it)->getTag().compare("RADIUS")==0)
					 {
						
						if((dynamic_cast<MBoolType *>(*par_it))->getValue())
							options|=options.set(6);
						continue;
					 }
					 if((*par_it)->getTag().compare("ROUNDNESS")==0)
					 {
						
						if((dynamic_cast<MBoolType *>(*par_it))->getValue())
							options|=options.set(7);
						continue;
					 }
			 }
}


void Pipeline::getMomentOptions(bitset<BIT_OPTIONS> &options,vector<MType *> &parameterValues)
{
		 
		for(vector<MType *>::iterator par_it = parameterValues.begin(); par_it!= parameterValues.end(); par_it++)
			{			
					if((*par_it)->getTag().compare("CENTROID")==0)
					 {
						if((dynamic_cast<MBoolType *>(*par_it))->getValue())
							options|=options.set(8);
						continue;
					 }
					 if((*par_it)->getTag().compare("AXIS")==0)
					 {	
						if((dynamic_cast<MBoolType *>(*par_it))->getValue())
							options|=options.set(9);
						continue;
					 }
					 if((*par_it)->getTag().compare("ECCENTRICITY")==0)
					 {
						
						if((dynamic_cast<MBoolType *>(*par_it))->getValue())
							options|=options.set(10);
						continue;
					 }
					 if((*par_it)->getTag().compare("THETA")==0)
					 {
						
						if((dynamic_cast<MBoolType *>(*par_it))->getValue())
							options|=options.set(11);
						continue;
					 }
			 }
}



























void Pipeline::getCommon(vector<vector<string>> files,vector<string> &common_names)
{
	ut::Trace tr = ut::Trace("PIPELINE:preparePipeline:getCommon string",__FILE__);
	int count = 0;
	  // Take the first load of each file
	for(int i=0;i<files[0].size();i++)
	{
		string compare1=(files[0])[i]; //First file, let�s say DAPI
		string compare2;  
		string commonPrefix("");
		int j=1;
		for( ;j<files.size();j++)
		{
			compare2=(files[j])[i]; // next file, let�s say FITC
			count=0;
			while(compare1[count] == compare2[count] && count < compare2.size())
			{
			 count++;
			}
			// if I have more than two files, I can check for errors
			if(compare2.substr(0,count).compare(compare1.substr(0,count))!=0)
			{
			  tr.message("ERROR: unequal number of files. Must supply same number of files for every channel and name then correctly.");
			   exit(-1909);
			}
			commonPrefix = compare1.substr(0,count);		 
			compare1=commonPrefix;	
		} 
		// Now I have the common Prefix
		// if is less than 2, just use the first file
		if(commonPrefix.size() < 2)
		{
		 commonPrefix=(files[j-1])[i];
		 commonPrefix = commonPrefix.substr(0,commonPrefix.size()-4); // Remove extension
		}
		else
		{
			if(commonPrefix.size()==(files[j-1][i]).size()) commonPrefix = commonPrefix.substr(0,commonPrefix.size()-4); // Remove extension
		}
		common_names.push_back(commonPrefix);
	 
	}
	  // Check how many substrings have in common
}
