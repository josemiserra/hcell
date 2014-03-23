#include "Hcell.h"
#include "Pipeline.h"
#include "XmlManager.h"
#include "Action.h"
#include "Checker.h"
#include "utils.h"
#include <opencv2/opencv.hpp>
#include <sys/stat.h>
#include <errno.h>
using namespace std;

Hcell::Hcell(void)
{
}


Hcell::~Hcell(void)
{
	// cout<<"HCELL destroyed"<<endl;
}

/*
 * Function: Run
 * ----------------------------
 *  Gets input values as vector or chars and executes the pipeline.
 *
 *   argc and argv : same as main
 *  
 *
 */
int Hcell::run(int argc,char *argv[]){


		XmlManager xmlmanager;
		Checker checker;
		InputValues inputvalues;
		int checked;
		vector<Action *> actionsList;
		int64 t1,t2;


		ut::Trace tr = ut::Trace("HCell",__FILE__);
		

		t1= tr.GetTimeMs64();
	

		// Input checking 
	    checked =  commandPreprocess(argc,argv,&inputvalues); 
	    if(checked==-1)
		{
			cout<<("Error: bad input parameters. ")<<endl;
			cout<<("Usage hcell --verbose -i input_dir -o output_dir -f pipeline.xml")<<endl;
			return -2013;
		}
		 

        checkInputValues(&inputvalues);
		string s = inputvalues.filename;

		tr.message("XML Loaded:",s.c_str());

		// Gets the XML value and returns a tasklist of Actions
		actionsList = xmlmanager.getActionsList(s.c_str());

		// Pipeline receives input arguments and actions list
		Pipeline pipeline= Pipeline(actionsList,inputvalues);
	    pipeline.start();
		t2 = tr.GetTimeMs64();
		cout << "TIME: " << (t2-t1)/1000.0 <<endl;


		cv::destroyAllWindows();
		return 0; 
}

/*
 * Function: commandPreprocess
 * ----------------------------
 *  Uses the getopt library to get arguments and prepares them in the InputValues structure.
 *
 *   argc and argv : same as main
 *  
 *  verbose : extra info during processing
 *  brief: Not implemented yet. Is a simplified version of verbose
 *  input directory
 *  output directory
 *  filename
 *  mem : determines RAM memory usage. 
 *
 */

int Hcell::commandPreprocess(int argc,char *argv[],InputValues *inputvalues){


static int verbose_flag;
static int show_flag;
static int brief_flag;
int c;

    while (1)
    {        
        static struct option long_options[] =
        {
            {"verbose", no_argument, &verbose_flag, 1},
            {"brief",   no_argument, &brief_flag, 1},
			{"noshow",   no_argument, &show_flag, 1},
            {"input directory",     required_argument, 0, 'i'},
            {"output directory",  required_argument, 0, 'o'},
			{"filename",  required_argument, 0,'f'},
			{"mem",  no_argument, 0,'m'},
            { 0 , 0 , 0 , 0 }
        };

        int option_index = 0;
        c = getopt_long(argc, argv,"i:o:f:m:", long_options, &option_index);

        // Check for end of operation or error
        if (c == -1)
            break;

        // Handle options
        switch (c)
        {
        case 0:
            /* If this option set a flag, do nothing else now. */
            if (long_options[option_index].flag != 0)
                break;
            printf ("option %s", long_options[option_index].name);
            if (optarg)
                printf (" with arg %s \n", optarg);
            break;

        case ('i'):
            printf("option -i with value '%s'\n", optarg);
			inputvalues->indir=optarg;
            break;

        case ('o'):
            printf("option -o with value '%s'\n", optarg);
            inputvalues->outdir=optarg;
            break;

        case ('f'):
            printf("option -f with value '%s'\n", optarg);
			inputvalues->filename=optarg;
            break;
		case('m'):
            printf ("option -m with value '%s'\n", optarg);
			// Take string and convert it to int
			// inputvalues->memory= optarg;

            break;
		case '?':
            /* getopt_long already printed an error message. */
			cout<<("Usage hcell --verbose -i input_dir -o output_dir -f pipeline.xml \n")<<endl;
			cout<<("\n NAME \n hcell - Pipeline for High Troughput Screening processing")<<endl;
			cout<<("\n SYNOPSIS \n hcell [OPTION]... [FILE]... ")<<endl;
			cout<<("\n DESCRIPTION \n  -i, input directory \n\t Mandatory. Path to the directory were images to be processed are placed.")<<endl; 
            cout<<("\n  -o, output directory. \n\t Mandatory. Directory where output images will be placed. If does not exist, it will be created automatically. ")<<endl;
			cout<<("\n  --verbose \n\t Optional.Print additional information about processes, like input and output matrices.")<<endl;
			cout<<("\n  -f, filename \n\t Mandatory. XML filename with the formatted instructions to be applied to images. ")<<endl;
			cout<<("\n  -m, memory in Mb. \n\t Optional. Restricts the maximum of memory usage for the pipeline pool. Minimum is the size of a read image,");
			cout<<(" if the provided memory is less than that, size of 1 image will be used. Values exceeding maximum RAM usage, will not be taken in account.")<<endl;               
            cout<<("\n  --noshow, turns off all the ASHOW parameters and no windows are displayed. \n")<<endl;
			cout<<("\n  --brief, same as verbose but no information about matrices is shown. \n")<<endl;
			cout<<("  ##  HCELL version 0.7. MAR-2014 ## \n");
			exit(-2015);
			break;

        default:
			cout<<("ERROR: Bad input parameters")<<endl;
            exit(-2017);
            abort();
        }
    }

	    if (optind < argc || argc == 1)
    {
        printf ("Bad arguments.");
        while (optind < argc) printf ("%s ", argv[optind++]);
        printf ("\n");
		exit(-1987);
    }
	
    if (verbose_flag)
	{
        printf("verbose flag is set\n");
	    inputvalues->verbose_on=true;
		ut::Trace::traceIsActive=true;
	}
	else
	{
	 inputvalues->verbose_on=false;
	}
    if (show_flag)
	{
        printf("No show flag is set : images will not be shown.\n");
		inputvalues->noshow = true;
	}
	else
	{
		 inputvalues->noshow= false;
	}

    if(brief_flag)
	{
        printf("Verbose flag on in mode brief.\n");
		inputvalues->verbose_on = true;
		inputvalues->brief = true;
		ut::Trace::traceIsActive=true;
		ut::Trace::brief=true;
	}
	else
	{
	 ut::Trace::brief=false;
	 inputvalues->brief = false;
	}

    return 0;
}     

/*******************************

Check Input and Output directories

****************************/
void Hcell::checkInputValues(InputValues *inputvalues)
{

	int nlen;
	// check if the last bar is missing, then add it
	nlen = strlen(inputvalues->indir)/sizeof(char);
	if(inputvalues->indir[nlen-1]==('/'))
	{
		 inputvalues->indir[nlen-1]='\0';
	}


	nlen = strlen(inputvalues->outdir)/sizeof(char);
	if(inputvalues->outdir[nlen-1]==('/'))
	{
		 inputvalues->indir[nlen-1]='\0';
	}

	
	if ( access(inputvalues->outdir, 0 ) == 0 )
	   {
	      struct stat status;
	      stat( inputvalues->outdir, &status );

	      if ( status.st_mode & S_IFDIR )
	      {
	         cout << "OUTPUT directory exists." << endl;
	      }

	   }
	   else
	   {

		         if(mkdir(inputvalues->outdir,S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)==0)
		         {
		         cout<<"Output directory created"<<endl;
		         }
		         else{
		        	 
						cout<<("Output directory cannot be created.")<<endl;
						if(errno==EACCES) cout<<"Permission denied to create a new directory."<<endl;
						if(errno==ENAMETOOLONG) cout <<"The length of the path argument exceeds {PATH_MAX} or a pathname component is longer than {NAME_MAX}."<<endl;
						if(errno==ENOENT) cout<<"Non existent directory."<<endl;
						exit(-1956);
		        	 
		         }
	   }

	return;
}
/*
*
*  MAIN
*
*/

int main(int argc,char *argv[])
{
		
		ut::Trace tr = ut::Trace("HCell MAIN",__FILE__);

		Hcell hcell;
		
		hcell.run(argc,argv);

		cout << "Hcell. Execution Finished."<< endl;

        cin.ignore(1);
        return 0;

}
