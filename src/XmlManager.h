
#ifndef _XMLManager_
#define _XMLManager_


#if defined( WIN32 ) && defined( TUNE )
	#include <crtdbg.h>
	_CrtMemState startMemState;
	_CrtMemState endMemState;
	#include <windows.h>
	#include <tchar.h>
#endif
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include "tinyxml.h"
#include "Action.h"
#include "ActionLoader.h"

#pragma once
class XmlManager
{

public:
	XmlManager(void);
	~XmlManager(void);

	bool XmlTest (const char* testString, const char* expected, const char* found);
	bool XmlTest( const char* testString, int expected, int found);
	std::vector<Action *> getActionsList(const char* filename);
	void findEndTagError(const char* filename);
	void printUntil(const char* filename, int line,string error);
	void XmlCheckStructure(TiXmlNode* rootnode);
};


#endif // _XMLManager_