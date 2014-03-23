#ifndef _INTERPRETER_
#define _INTERPRETER_

/******************************************************

Interpreter that accepts the grammar:


 Start_Symbol =  EXPRESSION
 EXPRESSION   =  LHS  '+' RHS |
                 LHS  '-' RHS |
                 LHS  '&' RHS |
			     LHS  '*' RHS | 
                 LHS  '/' RHS |
				 LHS '|' RHS |
                | RHS

RHS   = '-' Value | Value

Value        =  StringLiteral | NumberLiteral | '(' EXPRESSION ')' | Operator '(' EXPRESSION ')'

StringLiteral = {Letter}{AlphaNumeric}*

NumberLiteral = Integer | Float

Integer        = {Digit}+
Float          = {Digit}+ '.' {Digit}+
where
Digit = [0-9]
Letter = [a-z][A-Z]
Alphanumeric = [a-z][A-Z][0-9][_.]
Operator = a list of operators = MAX,MIN,MEAN...

*****************************************************************/
#include <climits>
#include <string>
#include <ctype.h>
#include "STree.h"

using namespace std;



class Interpreter
{
public:
	Interpreter(void){};
	~Interpreter(void){};
	enum COMPILER_TAG{ CERROR, STAND, TREE};

	typedef string::iterator Iterator;

    static const char* binaryOperators[];
	static const char* unaryOperators[];

	// Checks the viability of parsing. 
	static int checkInputObject(string input,string &error)
	{		 
		string _error = "";
		if(STree::isTree(input,_error)) return TREE;
		else
			if(_error.empty()) return STAND;
			else return CERROR;
	};


   static bool getSyntacticTree(string input,STree &myTree, string &error)
	{
		 
		if(!startGrammar(myTree,input))
		{
		  error = "ERROR parsing input expression. \n";
		  error += myTree.print();	
		  return false;
		}
	    return true;
	
	
	}
	

/**********************************
  GRAMMAR Rule evaluators

  Simply made by iterators. For future more complex grammar can be substituted by boost::Spirit, ANTLR or similar.
  Problem of this grammar implementation is recursivity.
  However, we are expecting simple user expressions easy to manage.

*********************************/



 /* Start_Symbol =  EXPRESSION */
static bool  startGrammar(STree &stree,string str)
{
	if(str.length()>2048) return false;
	str.erase(remove_if(str.begin(), str.end(), isspace), str.end()); // remove all spaces
	Iterator it = str.begin();
	Iterator end = str.end();
	stree.setOrigin(str);
	return MatchesExpression(stree,it,end);
}

static Iterator remove_if(Iterator beg, Iterator end,int (*pred)(int))
{
    Iterator dest = beg;
    for (Iterator itr = beg;itr != end; ++itr)
        if (!pred(*itr))
            *(dest++) = *itr;
    return dest;
}

/*
 EXPRESSION   =  LHS  '+' RHS |
                 LHS  '-' RHS |
                 LHS  '&' RHS |
			     LHS  '*' RHS | 
                 LHS  '/' RHS |
				 LHS '|' RHS |
                | LHS */
static bool MatchesExpression(STree &stree, Iterator& it,Iterator end) 
{
  // stree.addNode("EXP");
   bool matchLHS;
   matchLHS = MatchesLHS(stree,it,end); 
   if(it==end) return matchLHS;
   if(	matchLHS      //  'LHS'
			&&  MatchesBinaryOperator(stree,it,end)    //   operator +,-, &, |, *, /
            &&  MatchesRHS(stree,it,end)        //  'RHS'
           ) return true;
   else return matchLHS;                     
}

// RHS   = '-' Value | Value  (check for negative values)
static  bool MatchesLHS(STree &stree,Iterator& it,Iterator end) 
{  
	if(*it=='-') 
	{ 
	  stree.addNode(STree::UNOP,"-"); 
	  it++; 
	  if(it==end) return false;
	}
	return(MatchesValue(stree,it,end));                     
}

static  bool MatchesRHS(STree &stree,Iterator& it,Iterator end) 
{  
	if(*it=='-') 
	{ 
	  stree.addNode(STree::UNOP,"-"); 
	  it++; 
	  if(it==end) return false;
	}
	return(MatchesExpression(stree,it,end));                     
}

//   operator A operator B , e.g. A + B
static bool MatchesBinaryOperator(STree &stree,Iterator &it,Iterator it_end)
{
	string end("END");
	int i=0;
	while(end.compare(binaryOperators[i])!=0)
	{
		if(*it == binaryOperators[i][0]) 
		{      
			   string value(binaryOperators[i]);
			   stree.addNode(STree::BINOP,value);  it+=value.length();
			   if(it==it_end) return false;
			   return true;
		}
		i++;
	}
	return false;
}

/*
Value        =  StringLiteral | NumberLiteral | '(' EXPRESSION ')' | Operator '(' EXPRESSION ')'
*/

static  bool MatchesValue(STree &stree,Iterator& it,Iterator end) 
{
      
	if(isalpha(*it))  // then is an identifier or operator 
   { 
	   Iterator copy = it;  
	   string myOperator;
	   if(isOperator(copy,myOperator,end))
	   {
	     it=copy;
		 stree.addNode(STree::UNOP,myOperator);
		 stree.addNode(STree::OPEN);
		 if((it+1)==end) return false; // prevent errors 
		 if((*it=='('?(++it,true):false)
			&& MatchesExpression(stree,it,end)
			&& (it!=end)  // check again
			&&(*it==')'?(++it,true):false))
		 {
			 stree.addNode(STree::CLOSE);
		     return true;
		 }
		 else return false;
	   }
	   // Otherwise is an identifier.
	   // We don´t increment the iterator!!! We want to save our ID 
	   return(MatchesAlphaNumeric(stree,it,end));    
   }
   if(isdigit(*it))
   { 
	   return MatchesDigits(stree,it,end); 
   }
   if(*it=='(')
   {
	   stree.addNode(STree::OPEN);
	   it++; 
	   if(MatchesExpression(stree,it,end)
		  &&(it!=end)
		  &&(*it==')'?(++it,true):false)
		  )
	   {
	   stree.addNode(STree::CLOSE);
	   return true;
	   }
   }
   return false;
}

//   operator A operator B , e.g. A + B
static  bool isOperator(Iterator &it,string &myOperator,Iterator it_end)
{

    string end("END");
	int i=0;
	if((it+1)==it_end) return false;
	while(end.compare(unaryOperators[i])!=0)
	{
		if(*it == unaryOperators[i][0] &&  // We check the two first characters
		   *(it+1) == unaryOperators[i][1]) 
		{      
			   string value(unaryOperators[i]);
			   string orig;
			   for(int k=1;k<value.length();k++) if((it+k)==it_end) return false;
			   orig.insert(orig.begin(),it,it+value.length());
			   if(orig.compare(value)==0)
			   {
				   myOperator = value;
				   it=it+value.length();
				   return true;}
		}
		i++;
	}
	return false;
}


/*
NumberLiteral = Integer | Float

Integer        = {Digit}+
Float          = {Digit}+ '.' {Digit}+

*/
static bool MatchesDigits(STree &stree,Iterator& it,Iterator it_end)         //[0-9]Digits?
{
	string myValue="";
	Iterator end= it;
	int point_counter = 0;

	while( isdigit(*end) || *end=='.')
	{
		if(*end=='.') point_counter++;
		end++;
		if(end==it_end) break;
	}
	myValue.insert(myValue.begin(),it,end);
	if(!point_counter) stree.addNode(STree::INTEGER,myValue); // save in the three the ID
	else stree.addNode(STree::DOUBLE,myValue);
	if(point_counter>1) return false;
	else{ 
		it=end;
		return true;}
}

/*
StringLiteral = {Letter}{AlphaNumeric}*
*/
static bool MatchesAlphaNumeric(STree &stree, Iterator& it,Iterator it_end)         // Match ID
{
	string myValue="";
	Iterator end= it;
	int count = 0;
	while(*end=='_' ||  *end=='.' || isalnum(*end) || count>256)
	{
		end++;
		count++;
		if(end==it_end) break; 
	}
	myValue.insert(myValue.begin(),it,end);

	stree.addNode(STree::NAME,myValue); // save in the three the ID
	if(count<3 || count>256) return false;
	else{ 
		it=end;
		return true; }
}


};


#endif // Interpreter
