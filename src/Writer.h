#ifndef _WRITER_
#define _WRITER_


#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <bitset>
#include <stdio.h>


using namespace std;


struct FeaturesPipe
{
	const char* _reference;
	string _sreference;
	int _total; // total number of modules related to this features subpipeline
	int _count; // current number of modules "  "  "
	int _fcount; // total number of files
	bitset<12> _options;

	string _gen_filename;
	string _gheader;
	ofstream fgout;
	
	string _iheader;
	vector<string> _ind_filenames;
	
	vector<double> _genResults;   // this ones act as buffers
	vector<vector<double>> _indResults;  
	
	void setReference(string reference)
	{
		_sreference = reference;
		_reference = reference.c_str();
	}
	void setGenFilename(string gfilename)
	{
		_gen_filename = gfilename;
	}
};


class Writer
{
private:
	 vector<FeaturesPipe*> _fpipe;
	 vector<string> header;


public:

	Writer()
	{
		header.push_back(" b.mean \t"); // 0 
		header.push_back(" b.sd \t"); //1
		header.push_back(" b.mad \t"); //2
		header.push_back(" b.q0.01 \t"); //3
		header.push_back(" b.q0.05 \t");
		header.push_back(" b.q0.5 \t");
		header.push_back(" b.q0.95 \t");
		header.push_back(" b.q0.99 \t"); // 

		header.push_back(" s.area \t"); // 4
		header.push_back(" s.perimeter \t");
		header.push_back(" s.radius.mean \t"); // 6
		header.push_back(" s.radius.max \t");
		header.push_back(" s.radius.min \t");
		header.push_back(" s.roundness \t"); //7

		header.push_back(" m.cx \t"); // 8
		header.push_back(" m.cy \t");
		header.push_back(" m.major.axis \t"); //9
		header.push_back(" m.minor.axis \t");
		header.push_back(" m.eccentricity \t");
		header.push_back(" m.theta \t");
	};

	~Writer(void)
	{
			cout<< "WRITER finished" <<endl;
			closePipes();
	};

	void addPipe(FeaturesPipe *fpipe)
	{
		_fpipe.push_back(fpipe);	
	}

	void closePipes()
	{
		for(vector<FeaturesPipe*>::iterator it = this->_fpipe.begin();it!=this->_fpipe.end();++it)
				{
					(*it)->fgout.close();
				}
		while(!this->_fpipe.empty())
							{
								delete this->_fpipe.back(),  this->_fpipe.pop_back();
							}
	}

	void buildHeaders()
	{
		int count;
		FeaturesPipe *fp;
		for(vector<FeaturesPipe*>::iterator it= this->_fpipe.begin(); it!=this->_fpipe.end();++it)
		{
		
			fp=(*it);
	  
			fp->_gheader +=" File \t";
		    fp->_gheader +=" NObj \t";
			fp->_iheader +=" NObj \t";
	  
			int hcount=0;
			for(unsigned int i=0;i<fp->_options.size();i++,hcount++)
			{
			
			
			 if(fp->_options[i])
				{
			        					
					    fp->_gheader+=" mean_"+header[hcount];
						fp->_gheader+=" sd_"+header[hcount];
						fp->_iheader+=header[hcount];
						if(i==3)
						{ 
							count = 0;
							
							while(count<4)	//quantiles
							{
								count++;
								fp->_gheader+=" mean_"+header[hcount+count];
								fp->_gheader+=" sd_"+header[hcount+count];
								fp->_iheader+=header[hcount+count];
								
								
							}
						}
						
						if(i==8 || i==9) //centroid and axis
						{

								fp->_gheader+=" mean_"+header[hcount+1];
								fp->_gheader+=" sd_"+header[hcount+1];
								fp->_iheader+=header[hcount+1];						
						}

						if(i==6)
						{
							count=0;
							while(count<2)	//radius
							{
								count++;
								fp->_gheader+=" mean_"+header[hcount+count];
								fp->_gheader+=" sd_"+header[hcount+count];
								fp->_iheader+=header[hcount+count];
							}
						}
					}
					if(i==3) hcount+=4;
					if(i==6) hcount+=2;
					if(i==8 || i==9) hcount++;
			} // end for
	  }

		fp->fgout << fp->_gheader <<endl;
		
	};

/***************************************************************************
*
* Maybe needed if we want to retrieve some data, like the options 
*
***************************************************************************/
	FeaturesPipe* getFeaturePipe(const char* reference);

/***************************************************************************
*  WRITE
*  Careful!! This is an additive method completely entangled with the FeaturesPipe struct.
*  The idea is to use the struct as an accumulator of data and at the end of the pipe, make the whole writing.
*  For store a new whole file, use writeNew.
***************************************************************************/
	void write(vector<double> &genValues,vector<vector<double>> &indValues,const char* reference, bool save_ind)
	{
		FeaturesPipe *fp;
		// obtain the struct associated to the image.
		fp = getFeaturePipe(reference);
		// if I still have parts of the pipeline to save in the same place
		if(fp->_count < fp->_total)
		{
			if(fp->_genResults.empty())   // If it�s the first time I add general values...
				fp->_genResults = genValues;
			else    // if not, I insert the new results after the old ones
				fp->_genResults.insert(fp->_genResults.end(),genValues.begin(),genValues.end());
			
			if(save_ind)  // Now I check if I want to save individual values and I do the same...
			{
				if(fp->_indResults.empty())
				{
				for(vector<vector<double>>::iterator it=indValues.begin();it!=indValues.end();++it)
					{
					fp->_indResults.push_back(*it);
					}	
				}
				else
				{
					vector<vector<double>>::iterator itP = indValues.begin();
					for(vector<vector<double>>::iterator it = fp->_indResults.begin();					
					    it!= fp->_indResults.end(),itP !=indValues.end() ;
						++it,++itP)
					{
						(*it).insert((*it).end(),(*itP).begin(),(*itP).end());
					}

				}
			}
			fp->_count++;
		}
		if(fp->_count==fp->_total) // When the number of writes is equal to the total
			{
				fp->_count=0;
				this->toFile(fp,save_ind);
			}

	}


	void toFile(FeaturesPipe *fp,bool save_ind)
	{

		// First extract the name of the file
			string filename = fp->_ind_filenames[fp->_fcount];
			fp->_fcount++;

			fp->fgout<< filename <<" \t";

			fp->fgout << fp->_indResults.size() <<"\t";
		// save the general features
		for(vector<double>::iterator it= fp->_genResults.begin(); it!=fp->_genResults.end();++it)
		{
			fp->fgout << (*it) << " \t" ; // Suspicious 

		}
		 fp->fgout<<"\n";
		 // we need to flush out the stream 

		// Now open the individual file if we want to save
		if(save_ind)
		{
		
			ofstream f_ind;
			f_ind.open(filename.c_str());
			f_ind << fp->_iheader << " \n";
			int i=0;
		// Find the file in which we are.
		for(vector<vector<double>>::iterator it = fp->_indResults.begin();					
											   it!= fp->_indResults.end();
												++it,i++)
		{
			     f_ind<<i <<" \t";	
				for(vector<double>::iterator it2 = (*it).begin();					
											 it2!= (*it).end();
											 ++it2)	
				{
				  f_ind << (*it2) <<" \t" ;
				}
				f_ind<<" \n";
		}
		f_ind.close();
		}
		fp->_genResults.clear();
		fp->_indResults.clear();

		if(fp->_fcount%50 == 0) {
			fp->fgout.close(); 
			fp->fgout.open(fp->_gen_filename,ios::app);
		}
		
		}




	// Needed for repeated writing (trick for haralick scales)
	void dontCountFile(const char* ref)
	{
		FeaturesPipe *fp;
		fp = getFeaturePipe(ref);
		fp->_fcount--;
	}

	/***************************************************************************
*  WRITE
*  Gets a header, a filename and the values to save. Individual values in a general form are
*  saved in a different file, with same name plus general.
*  
***************************************************************************/
	void writeNew(const char* header,const char *filename,const char *extra,vector<vector<double>> &indValues)
	{
		// Open file with filename
		FeaturesPipe *fp;
		// obtain the struct associated to the image.
		fp = getFeaturePipe(filename);
		string _filename = fp->_ind_filenames[fp->_fcount];
		// Add extra info
		_filename.append(extra);
		// Save header
		// header
		// First extract the name of the file
		std::ofstream ff;

		fp->_fcount++;

		ff.open(_filename);

		ff << header << endl;
		int i=0;
		// Find the file in which we are.
		for(vector<vector<double>>::iterator it = indValues.begin();					
											   it!= indValues.end();
												++it,i++)
		{
			     ff << i <<" \t";	
				for(vector<double>::iterator it2 = (*it).begin();					
											 it2!= (*it).end();
											 ++it2)	
				{
				  ff << (*it2) <<" \t" ;
				}
				  ff <<" \n";
		}
		ff.close();
		

	}

};
#endif
