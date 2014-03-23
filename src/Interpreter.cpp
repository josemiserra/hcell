#include "Interpreter.h"

const char* Interpreter::unaryOperators[] =
{
	// --------------------------------------------------------------//
	 "LOG","MAX","MIN","MEAN","SD","SQRT","TRACE","NOT","!","END"

}; 

// * per element product, x does a matrix multiplication
// \ absolute difference
const char* Interpreter::binaryOperators[]=
{
	"+","-",".-","*","/","&","|",".*","^","END"
};
