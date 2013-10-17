#include "XmlManager.h"

#define MAX_PARAMS 50


using namespace std;

static int gPass = 0;
static int gFail = 0;

bool XmlManager::XmlTest (const char* testString, const char* expected, const char* found, bool noEcho )
{
	bool pass = !strcmp( expected, found );
	if ( pass )
		printf ("[pass]");
	else
		printf ("[fail]");

	if ( noEcho )
		printf (" %s\n", testString);
	else
		printf (" %s [%s][%s]\n", testString, expected, found);

	if ( pass )
		++gPass;
	else
		++gFail;
	return pass;
}


bool XmlManager::XmlTest( const char* testString, int expected, int found, bool noEcho )
{
	bool pass = ( expected == found );
	if ( pass )
		printf ("[pass]");
	else
		printf ("[fail]");

	if ( noEcho )
		printf (" %s\n", testString);
	else
		printf (" %s [%d][%d]\n", testString, expected, found);

	if ( pass )
		++gPass;
	else
		++gFail;
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

XmlManager::XmlManager(void)
{
}


XmlManager::~XmlManager(void)
{
}


vector<Action *> XmlManager::getActionsList(const char* filename){

	    ActionLoader actionloader;   
		vector<Action *> actionsList;

	    TiXmlDocument doc( filename );
		bool loadOkay = doc.LoadFile();

		if ( !loadOkay )
		{
			cout<< "Could not load file. Error="<< doc.ErrorDesc() <<endl;
			
			exit( -2011 );
		}

		cout<<"** Reading pipeline from disk: ** \n" <<endl;
		doc.Print( stdout );


		TiXmlNode* node = 0;
		TiXmlElement* paramElement = 0;
		TiXmlElement* itemElement = 0;
	    TiXmlElement* element=0;


		// Get the "Pipeline" element.
		node = doc.RootElement();

#ifdef DEBUG
		assert( node );
		XmlTest( "Root element exists.", true, ( node != 0 && node->ToElement() ) );
		XmlTest ( "Root element value is 'PIPELINE'.", "PIPELINE",  node->Value());
#endif

		// Walk all the elements in a node by value.
		int count,countP;
		const char *FNAME,*MNAME;
		const char *nameparams[MAX_PARAMS];
		const char *params[MAX_PARAMS];

		count = 0;
		for( node = node->FirstChild( "MODULE" );
			 node;
			 node = node->NextSibling( "MODULE" ) )
		{
			 MNAME= dynamic_cast<TiXmlElement*>(node)->Attribute("name"); //Module Name
			
			for( element = node->FirstChildElement("FUNCTION");
				 element;
				 element = element->NextSiblingElement())
				{	
				FNAME=element->Attribute("name"); // Function Name
				countP=0;
				for( paramElement=element->FirstChildElement();
					 paramElement;
					 paramElement=paramElement->NextSiblingElement())
				{
					 nameparams[countP] = paramElement->Value(); // Attributes
					 params[countP] = paramElement->GetText();
					 countP++;
				}
				 actionsList.push_back(actionloader.getAction(MNAME,FNAME,nameparams,params,countP));
				 count++;
				}
			
		
		}
	

#ifdef DEBUG
		XmlTest( "'Module' children of the 'Pipeline' element, using First/Next.", 1, count );
#endif

		return actionsList;
		}
