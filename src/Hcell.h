
/*
*  HCELL
*
*  Main progam, gets the input values and run the pipeline.
*
 */



#ifndef _HCELL_
#define _HCELL_



#include <iostream>
#include <stdio.h>
#include <getopt.h>
#include <vector>
#include <string>

struct InputValues{
		const char *filename;
		const char *indir;
		const char *outdir;
		bool verbose_on;
};



class Hcell
{
public:
	Hcell(void);
	~Hcell(void);
	int run(int argc,char *argv[]);
protected:
    int commandPreprocess(int argc,char *argv[],InputValues *inputvalues);
};


#endif
