#include "Hcell.h"
#include "utils.h"


 #include "Pipeline.h"
 #include "XmlManager.h"
 #include "Action.h"

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
		 

		string s = inputvalues.filename;

		tr.message("XML Loaded:",s.c_str());

		// Gets the XML value and returns a tasklist of Actions
		actionsList = xmlmanager.getActionsList(s.c_str());

		// Pipeline receives input arguments and actions list
		Pipeline pipeline= Pipeline(actionsList,inputvalues);
	    pipeline.start();
		t2 = tr.GetTimeMs64();
		cout << "TIME: " << (t2-t1)/1000.0 <<endl;


		return 0; 
}

/*
 * Function: commandPreprocess
 * ----------------------------
 *  Uses the getopt library to get arguments and prepares them in the InputValues structure.
 *
 *   argc and argv : same as main
 *  
 *
 */

int Hcell::commandPreprocess(int argc,char *argv[],InputValues *inputvalues){


static int verbose_flag;
int c;


    while (1)
    {        
        static struct option long_options[] =
        {
            {"verbose", no_argument, &verbose_flag, 1},
            {"brief",   no_argument, &verbose_flag, 0},
            {"input directory",     required_argument, 0, 'i'},
            {"output directory",  required_argument, 0, 'o'},
			{"filename",  required_argument, 0,'f'},
            { 0 , 0 , 0 , 0 }
        };

        int option_index = 0;
        c = getopt_long(argc, argv,"i:o:f:", long_options, &option_index);

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
		case '?':
            /* getopt_long already printed an error message. */
            break;

        default:
            abort();
        }
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
    if (optind < argc)
    {
        printf ("non-option ARGV-elements: ");
        while (optind < argc) printf ("%s ", argv[optind++]);
        printf ("\n");
		return -2012;
    }
    return 0;
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
