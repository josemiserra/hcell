#include "utils.h"

namespace ut{

bool Trace::traceIsActive=false;
bool Trace::brief = false;
std::vector<cv::Scalar> utils::colorTab;
int utils::colorcounter;


const string utils::colourNames[TOTAL_COLOR_NAMES]={ "GREY","BLACK","RED","BLUE",
	"MAGENTA","YELLOW","BROWN","GREEN","CYAN","PURPLE","WHITE","RANDOM"};

}

