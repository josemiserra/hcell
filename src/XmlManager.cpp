#include "XmlManager.h"
#include "tinyxml.h" 
#include <iostream>
#include <fstream>
#include <sstream>
#include "Checker.h"


using namespace std;

XmlManager::XmlManager(void)
{
}


XmlManager::~XmlManager(void)
{
}


bool XmlManager::XmlTest (const char* testString, const char* expected, const char* found)
{
	bool pass = !strcmp( expected, found );
	if ( ! pass )
	{
		printf ("[fail]");
		printf (" %s\n", testString);
	}

	return pass;
}


bool XmlManager::XmlTest( const char* testString, int expected, int found)
{
	bool pass = ( expected == found );
	if (! pass )
	{
		printf ("[fail]");
		printf (" %s\n", testString);
	}

	return pass;
}


void NullLineEndings( char* p )
{
	while( p && *p ) {
		if ( *p == '\n' || *p == '\r' ) {
			*p = 0;
			return;
		}
		++p;
	}
}

void XmlManager::printUntil(const char* filename, int errorline,string msg)
{
	ifstream in_stream;
	in_stream.open(filename);
	int cnt=1;
	for( std::string line; getline( in_stream, line ); )
	{
	    
		 if(cnt==errorline)
		 {
			 cout<< cnt <<msg << line <<endl;
			 break;	
		 }
		 else
		 {
			 cout<< cnt <<":\t"<<line<<endl;
			 cnt++;
		 }
	}
    in_stream.close();

}

/***************************************************
* void  findEndTagError(const char* filename)
*  Searches for missing end tags.
*	Costly function, only used after the error is found.
*	Tell the line where the error happened.
*
*************************************************/
void XmlManager::findEndTagError(const char* filename)
{
	typedef struct tag
	{ 
		string value;
	    int tagline;
	};
	
	vector<tag> tags;
	ifstream in_stream;
	in_stream.open(filename);
	int cnt = 1;
	string comment="!--";
    // Read all tags, based on opening marks
	int pos,posn;
	int errorLine= 1;
	int errorCom = 1;
	for( std::string line; getline( in_stream, line );errorLine++)
	{
		  utils::trim(line);
		  pos= line.find_first_of("<",0);
		  while(pos==string::npos)
		  {
			  utils::trim(line); 
			  getline( in_stream, line );
			  errorLine++;
			  pos= line.find_first_of("<",0);
		  } 
		  
		  // Remove comments
		 if(line.substr(pos,4).compare("<!--")==0)
		 {	
			 errorCom=errorLine;
			 // cout<<"Comment on line "<< errorCom <<endl;

			 while(line.find("-->",pos)>line.length() && !in_stream.eof())
			 {
				 getline( in_stream, line );
			     errorLine++;
			 }
			 if(in_stream.eof())
			 { 
				 cout << "ERROR: COMMENT without end starting in line"<<endl;
				 this->printUntil(filename,errorCom,": ## ERROR ## --->");
			 }
			 continue;
		 } 
		 // If it is not a comment, save value and continue reading the line		 
		do
		{
			 posn=line.find_first_of("<",pos+1);
			 tag ntag;
			 ntag.value= line.substr(pos,posn-pos);
			// cout<<ntag.value<<endl;
			 ntag.tagline = errorLine;
			//  cout<<ntag.tagline <<endl;
			 tags.push_back(ntag);

			 if(posn!=string::npos) posn=line.find_first_of("<",posn);	
			 pos=posn;
		}while(posn!=string::npos);

		
	}
	in_stream.close();
	
	//
	// Now, for each tag, we need to find the value and the closest next end tag.
	// 
	string value;
	for(auto it =tags.begin();  it != tags.end(); it++)
	{	
		// If Comment or special, delete
		if((it->value.substr(0,2)).compare("<?")==0||(it->value.substr(0,3)).compare("<!-")==0)
		{ 
			(it->value)="--T"; 
			continue;
		}
		// Remove auto-closed tags
	    if(it->value.find("/>",0)<it->value.length())
		{ 
				(it->value)="--T"; 
			    continue;
		}
		//Get value
		int pos = it->value.find_first_of(" >",0);
		value = it->value.substr(1,pos-1);
		if(value.compare("-T")==0) continue;
		// cout<< value << " ..Searching tag." << endl;
		bool found=false;
		for(auto it2 =(it+1);  it2 != tags.end() && !found; it2++ )
		{		
			// cout<<it2->value<<endl;
			// Possibility of /
			if((it2->value).compare("--T")==0) continue;
			if(it2->value.find(value,0)<it2->value.length()) // If I find value
			{	
				// If is not an end tag, then, error...
				
				if((it2->value).substr(0,2).compare("</")==0)  // If value found and is end tag
				{ 
					
					(it2->value)="--T";   // Delete end tag and substitute by empty line
					(it->value)= "--T";   // Delete beginning tag and substitute by empty line.
					found=true;  // We need to find always a closing tag
				 }
				else  // Here is my error
				{
					this->printUntil(filename,it->tagline,": ## ERROR: Closing tag ## --->");
					exit(-2020);
				}
			 }
		} 
		if(!found)
		{
			cout<< value <<endl;
			this->printUntil(filename,it->tagline,": ## ERROR: Closing tag not found for :## --->");
			exit(-2021);
		}

	}
	 

}


void XmlManager::XmlCheckStructure(TiXmlNode* rootnode)
{
	    
	    TiXmlNode* node = 0;
		TiXmlNode* node2 = 0;

		node=rootnode;
		bool rootPass;

		assert( node );
		rootPass=XmlTest( "Root element not found", true, ( node != 0 && node->ToElement() ));
		rootPass=XmlTest ( "Root element value is not 'PIPELINE'. Change it", "PIPELINE",  node->Value());

		if(!rootPass) exit(-2022);

		// Are all the next level nodes modules?
		int cnt=0;
		bool pass;
		for( node = node->FirstChild();
			 node;
			 node = node->NextSibling() )
		{
			if((node->Type()!=node->TINYXML_COMMENT))
			{	
			cnt++;
			pass=XmlTest ( "Node next to root in value is not 'MODULE'. Change it", "MODULE",  node->Value());
			
			if(!pass)
			{				
				// then print wrong node
				cout<< "Module :"<<cnt<<endl;
				node->Print(stdout,0);
				cout<<endl;
				exit(-2023);
			}
			 // Are all the levels next module a function?
			 for( node2 = node->FirstChild();
				  node2;
				  node2 = node2->NextSibling())
			 {
			  if((node->Type()!=node->TINYXML_COMMENT))
			  {
			   pass=XmlTest ( "Node next to MODULE in value is not 'FUNCTION'. Change it.", "FUNCTION",  node2->Value());
			   if(!pass && (node->Type()!=node->TINYXML_COMMENT))
			   {
				// then print wrong node
				cout<< "Function :"<<cnt<<endl;
				node2->Print(stdout,0);
				cout<<endl;
				exit(-2025);
			  }
			 }
			}
		   }	 // close if comment
		}  // close for
}

vector<Action *> XmlManager::getActionsList(const char* filename){

	    ActionLoader actionloader;  
		Checker checker;
		vector<Action *> actionsList;

	    TiXmlDocument doc( filename );
		bool loadOkay = doc.LoadFile();

		if ( !loadOkay )
		{

			// If a tag is not well closed, tell the line
			cout<< "Could not load file. ERROR ="<< doc.ErrorDesc() <<endl;			
			if(doc.ErrorId()==TiXmlBase::TIXML_ERROR_READING_END_TAG)
			{
				findEndTagError(filename);
			}
			exit( -2026 );
		}

		cout<<"** Checking XML pipeline file from disk: ** \n" <<endl;
	//	doc.Print( stdout );

		XmlCheckStructure(doc.RootElement());


		TiXmlNode* node = 0;
		TiXmlElement* paramElement = 0;
		TiXmlElement* itemElement = 0;
	    TiXmlElement* element=0;


		// Get the "Pipeline" element.
		node = doc.RootElement();

		// Walk all the elements in a node by value.
		int count,countP;
		const char *FNAME,*MNAME;
		const char *nameparams[MAX_PARAMS];
		const char *params[MAX_PARAMS];

		count = 0;
		bool passOkay=true;

		for( node = node->FirstChild( "MODULE" );
			 node;
			 node = node->NextSibling( "MODULE" ) )
		{
			 MNAME= dynamic_cast<TiXmlElement*>(node)->Attribute("name"); //Module Name
			 
			 if(!checker.checkModule(MNAME))
			 {
			   node->Print(stdout,0);			   
			   cout<< endl << "Module "<<MNAME<< "  does not exist." << endl;
			   cout<< endl << "Available modules:"<<endl;
			   for (int i=0; i<TOTAL_MODULES; i++)
				{
				cout << Checker::Modules[i] << endl;
				}
			   exit(-2027);
			 }

			for( element = node->FirstChildElement("FUNCTION");
				 element;
				 element = element->NextSiblingElement())
				{	
				FNAME=element->Attribute("name"); // Function Name
				
				if(!checker.checkFunction(MNAME,FNAME))
				{
					 element->Print(stdout,0);			   
					 cout<< endl << "Function "<<FNAME<< "  does not exist in Module:" << MNAME << endl;
					 cout<<"Available functions in "<<MNAME << ":"<<endl;
					 checker.printModuleFunctions(MNAME);
					 exit(-2028);
				}

				countP=0;
				for( paramElement=element->FirstChildElement();
					 paramElement;
					 paramElement=paramElement->NextSiblingElement())
				{
					 nameparams[countP] = paramElement->Value(); // Attributes
					 params[countP] = paramElement->GetText();
					 countP++;
				}
			
			
			try
			{
				// Types are better checked when parameters are converted
				string error;
				if(!checker.checkParameters(MNAME,FNAME,nameparams,params,countP,error)) // check validity of each parameter 
					throw error;
				else 
				  {		  
					  actionsList.push_back(actionloader.getAction(MNAME,FNAME,nameparams,params,countP));
				   }
			}
			catch(string Str)
			{
				element->Print(stdout,0);			   
				cout << "\n **Bad Conversion parameter in function:" << FNAME << "** "<< Str;
				exit(-2029);
		    }
			
			count++;
			}		
		
		}
		return actionsList;
	};