#include "ComputeFeatures.h"
std::map<std::string, ComputeFeatures::Function > ComputeFeatures::tFunc;
FeatureCalculator *ComputeFeatures::featcalc;
Writer *ComputeFeatures::featureWriter;
std::map<std::string, int> ComputeFeatures::eMap;