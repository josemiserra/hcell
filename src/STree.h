#ifndef _STREE_
#define _STREE_

#include <stack>
#include <vector>
#include <string>
#include <memory>
#include <limits>
#include <opencv2/opencv.hpp>
#include "PoolManager.h"
#include "proctools.h"


using namespace std;
using namespace ut;
/**
    STree.h
    Purpose: Calculates a binary tree based on the input given from a the Interpreter.
	         The interpreter analyzes a grammar and takes tokens as strings, but
			 strings must be stored properly in a structure for run-time evaluation.
			 In other words, we can have this 3 kind of inputs completely different:
			 a) <INPUT>3.0</INPUT>
			 b) <INPUT>3.0*Image01+5</INPUT>
			 c) <INPUT>Factor*9+MAX(Image01)

			 First case is evaluated as double, but in b) and c), we need a structure for keeping
			 the data. That structure is a STree, a binary which stores the parsing coming from the
			 interpreter.
			 E.g.    3.0     *   Image01   +   5
			         (MTypeDouble)   BINOP (*)  (MTypeString)  BINOP(+)  (MTypeInt) 

					 Result a Matrix 
    @author JMS
    @version 1.0 02/2014
*/



class STree
{

private:

	typedef struct node
	{
		int code;  // Encodes type of node : Interpreter::DOUBLE,NAME,...
		string value; // Type class
		shared_ptr<node> left;  // left node in tree
		shared_ptr<node> right;  // right node in tree
		shared_ptr<node> previous; // previous node in tree
	}Node;

	typedef shared_ptr<Node> sp_node;

	string _origin;
	stack<sp_node> node_pile;
	sp_node _root;
	sp_node _current_node;

	shared_ptr<PoolManager> pool;

/*****************************************************************
	NAME: resetPile
    DESCRIPTION: remove all the elements from the Pile

******************************************************************/
	 	void resetPile()
		{
			while (!node_pile.empty())
			{
			node_pile.pop();
			} 
		}
  public:
	enum ID{ROOT,OPEN,CLOSE,INTEGER,DOUBLE,NAME,BINOP,UNOP};

	
	STree(void)
	{ 
		this->init();
		pool = PoolManager::getInstance();
	};
	~STree(void)
	{
		_root->previous = nullptr;  // destroy root
		this->resetPile();

	};

	void setOrigin(string origin){
		_origin = origin;
	}
	
	// Checks the viability of parsing. 
    static bool isTree(string input,string &error)   // it is suposed that is already bigger than 3 chars
	{
		int pos;
		 pos=input.find_first_not_of(" \r\t\n");
		 if(pos==std::string::npos)
		 {
			 error = "ATTENTION: Empty Value in parameter not allowed!!!!";
			 return false;
		 }
		pos=input.find_first_of("\\*-+/|&%!^()><");  // if I find this characters, I have operators
	    if(pos==std::string::npos)
			return false; // In this case, I have a common stand.
		 else
			return true; // 
	};





/*****************************************************************
	NAME: init
    DESCRIPTION: Creates root node (unique in the tree). 
				 _current_node points to _root.
				 _root needs to point to itself in order to avoid problems with operator assignment.
******************************************************************/
	void init()
	{
	 if(!node_pile.empty()) this->resetPile();
	 _root = sp_node(new node());
	 _root->code = ROOT;
	 _root->previous = _root;
	 node_pile.push(_root);
	 _current_node = _root;
	}; 

/*****************************************************************
	NAME: addNode
    DESCRIPTION: Gets an id and value from the Interpreter and creates a node
	that adds to the tree

    @param id 

	In this case only OPEN and CLOSE are taken in account. OPEN and CLOSE
	parenthesis are not needed once read because they can be inferred
	from the tree structure.
	E.g.  (A+B)*5         *
						/	\
	                   +     5
					  / \
					 A   B

		  A+(B*5)		  +
						/   \
					   A     *
							/ \
						   B   5
**********************************************************************/
	void addNode(int id)
	{
		if(id==OPEN)
		{
				 // Create open node type
				 sp_node open = sp_node(new Node());
				 open->code = OPEN;
				 // add it to the pile!
				 node_pile.push(open);
				 // Open is a fake temporal node which allows us to operate
				 // but we don´t want to lose the link to the previous structure
				 open->previous = _current_node;
				 _current_node = open;
				 return;
		}
		if(id==CLOSE)
		{
			sp_node top_node = node_pile.top(); // get my closest open parenthesis (top in the pile)
			
			if(top_node->previous->left==nullptr) // If the previous to the "(" is empty in left...
			{
				top_node->previous->left = top_node->left; // I know open_nodes only can link to the left
			}
			else
			{
				top_node->previous->right= top_node->left; 
			}
			top_node->left->previous = top_node->previous; // reassign previous nodes
			if(top_node->previous->code!=ROOT) _current_node = top_node->previous;  // and my current node is now the previous to close the parenthesis
			else _current_node = top_node->left;
			node_pile.pop(); // popping will derreference our open node, and it will be destroyed.		
			
		}
	
	}; 

	/*

	NAME: addNode
    DESCRIPTION: Gets an id and value from the Interpreter and creates a node
	that adds to the tree

    @param id 
	       value 


	 Ex. parsing (A+B)
	    ITERATION  NODE POINTERS inside the node
		  1   PILE root   
		      TREE root
	      2   PILE root open 
		      TREE root  previous => root, left => null, right => null
				   open  previous => root, left => null, right => null
		  3   PILE root open
			  TREE root previous => root, left => null, right => null
			       open previous => root, left => A, right => null
				   A    previous => open, left => null, right => null
		  4   PILE root open
			  TREE root previous => root, left => null, right => null
			       open previous => root, left => +, right => null
				   A    previous => +, left => null, right => null
				   +    previous => open, left => A, right => null
		  5   PILE root open
			  TREE root previous => root, left => null, right => null
			       open previous => root, left => +, right => null
				   A    previous => +, left => null, right => null
				   +    previous => open, left => A, right => B
				   B    previous => +, left => null, right => null

		  6   PILE root   
			  TREE root previous => root, left => +, right => null
				   A    previous => +, left => null, right => null
				   +    previous => root, left => A, right => B
				   B    previous => +, left => null, right => null
 	*/

	void addNode(int id,string _value)
	{
				// create node
				sp_node idnode = sp_node(new Node);
				idnode->code = id;
				idnode->value=_value;

				// If is a binary operator, we need to sink to the left side
				if(id==BINOP)
				{		
					// If is UNOP we need to jump it.
					while(_current_node->code != ROOT   && _current_node->previous->code==UNOP )
					{
					  _current_node = _current_node->previous;
					}
					// Operators of level 1, must be refloat for keeping operator precedence
					if(isLevel1(_value))  refloat(_current_node);
				    swap(_current_node,idnode);
					
				}
				else  // Is a node hosting values or UNOP
				{
					if(_current_node->left==nullptr)
					{
						_current_node->left = idnode;
					}
					else
					{
					    _current_node->right = idnode;
					}
					idnode->previous = _current_node;
					_current_node     = idnode;
				}

	}; // adds identifier

	/*****************************************************************
	NAME:inorderPrint
    DESCRIPTION: Prints the binary tree contents to a string
	 InOrder Print all the items in the tree to which in_node points.
     Items in the left subtree are printed first, followed by the item
	 in the in_node node, followed by the items in the right subtree.

    @param initial node
    @return string in sequence

******************************************************************/
	void inorderPrint(sp_node in_node,string &sequence){

		if (in_node != nullptr) {  // Am I a leaf? 
           inorderPrint( in_node->left,sequence);    // Print items in left subtree.
		   sequence+= in_node->value; 
           inorderPrint( in_node->right,sequence);   // Print items in right subtree.
        }

		return;
	}
/*****************************************************************
	NAME: print()
    DESCRIPTION: Shuttle function for printing the STree

******************************************************************/
	 string print() {
        string sequence="";
		this->inorderPrint(this->_root,sequence);
		return sequence;
	 } 

/*****************************************************************
	NAME: evalTreeAsDouble  evalTreeAsInt
    DESCRIPTION: Given the syntactic tree returns a double resulting from 
	the evaluation of the tree

    @return double 

	This function does not check semantics, meaning, the contents must be
	inherently correct. 

******************************************************************/

	 double evalTreeAsDouble(int pid)
	 {
		 Mat result(1, 1, DataType<double>::type);
		 evalTreeAsNumber(_root->left,result,pid); 
		 // if my matrix is bigger than 1 element, then there is something wrong
		 // e.g. all my identifiers are matrices and we can´t get a unique double number
		 if(result.rows>1||result.cols>1){ return -1.0; }
		 return result.at<double>(0,0);
	 }

	 int evalTreeAsInt(int pid)
	 {

		 Mat result(1, 1, DataType<double>::type);
		 evalTreeAsNumber(_root->left,result,pid); 
		 if(result.rows>1||result.cols>1){ return -1; }
		 return (int) result.at<double>(0,0);
	 }



/*****************************************************************
	NAME: evalTreeAsDouble  evalTreeAsInt
    DESCRIPTION: Given the syntactic tree returns a double resulting from 
	the evaluation of the tree

    @return double 

	This function does not check semantics, meaning, the contents must be
	inherently correct. 

******************************************************************/

	 void evalTreeAsNumber(Mat &res,int pid)
	 {
		 evalTreeAsNumber(_root->left,res,pid);
	 }
	 // change to iterative
	 void evalTreeAsNumber(sp_node in_node,Mat &result,int pid)
	 {

		 if(in_node !=nullptr)
		 {  
		   Mat C(1, 1, DataType<double>::type);
		   if(result.empty()) C.copyTo(result);
		   int code = in_node->code;
		   string nstr = in_node->value;
		   Mat A(result);
		   Mat B(result); 
		   switch(code)
		   {
			 case(BINOP):
			   evalTreeAsNumber( in_node->left,A,pid);
			   evalTreeAsNumber( in_node->right,B,pid);
			   proctools::binaryOp(A,B,C,nstr);
			   C.copyTo(result);
			   break;
			case(UNOP):
			   evalTreeAsNumber(in_node->left,A,pid);
			   proctools::unaryOp(A,C,nstr);
               C.copyTo(result);
			   break;
			case(DOUBLE):
				char *pEnd;
				result.at<double>(0,0) = strtod(nstr.c_str(),&pEnd);
			    break;
		    case(INTEGER):
				result.at<double>(0,0) = (double)(atoi(nstr.c_str()));
			    break;
			case(NAME):
			   // then return a factor in result
			   ut::utils::combnames(nstr,pid,nstr);
			   if(pool->containsFactor(nstr.c_str()))
			   { 
			    result.at<double>(0,0) = pool->getFactor(nstr.c_str());
			   }
			   else
			   {		  
				   Mat *R;
				   R = pool->getImage(nstr.c_str());
				   if(R != nullptr)   // Means that is an output (object that has not been created yet)
				   {
					   result = Mat(*R);
					   R->copyTo(result);
				   }
			   }
			   break;
			default: // or ROOT
			  break;
           }
		   return;
	 }
		 }

/*****************************************************************
	NAME: evalTreeAsString
 

******************************************************************/
	 string evalTreeAsString(int pid,string name)
	 {
		 // save result into pool
		 
		 Mat A;
		 evalTreeAsNumber(_root->left,A,pid);
		 utils::combnames(name,pid,name);
		 pool->storeImage(A,name);
		 return name;
	 }
	 

	 void getListOfNames(vector<string> &imagesList,vector<string> &factorsList)
	 {
		sp_node current;
        stack<sp_node> s;
		current = _root;
        while (!s.empty() || current != nullptr) {
			if(current)
			{
					s.push(current);
					current = current->left;
			}
			else
			{		current = s.top();
					s.pop();
					if(current->code == NAME){
						if(pool->containsFactor(current->value.c_str()))
						{
							factorsList.push_back(current->value);
						}
						else
						{
							imagesList.push_back(current->value);
						}
					}
					current = current->right;
			}

	 
	 }	 
	 };

    void getEmptyName(string &empty_name)
	{
		sp_node current;
        stack<sp_node> s;
		current = _root;
		int count = 0;
        while (!s.empty() || current != nullptr) {
			if(current)
			{
					s.push(current);
					current = current->left;
			}
			else
			{		current = s.top();
					s.pop();
					if(current->code == NAME){
						if(!pool->containsImage(current->value.c_str()))
						{
							empty_name = current->value;
							count++;
						}
					}
					current = current->right;
			}	 
	 }	 
		if(count>1)
		{
		  cout<<"Output expression has more than one unknown identifier. Please correct it!"<<endl;
		  exit(-1962);
		}
		else
		{
		if(count==0) empty_name = "";
		}
		return;
	 };


	 bool isLevel1(string value){
		return(value.find_first_of("+-")!=std::string::npos);
	 }


	 void refloat(sp_node inode)
	 {
		while(_current_node->previous->code != ROOT  && _current_node->previous->code != OPEN)
		{
			_current_node = _current_node->previous;
		}
		return;
		
	}
	 
	 void swap(sp_node &current_node,sp_node &inode)
	 { 
	  current_node->previous->left = inode; // change left pointer of previous, now to my new node
	  inode->previous = current_node->previous; 
	  inode->left = current_node;  // assign
	  current_node = inode;
	 }

////////////////////////////////////////////////////////////////////////////

};
#endif