#include "Pool.h"

shared_ptr<Pool> Pool::instance;

int Pool::maxObjects=0;
int Pool::maxImages=0;

int Pool::numImages=0;
int Pool::numObjects=0;

vector<shared_ptr<Slot<Mat>>> Pool::imagesPool;
vector<shared_ptr<Slot<vloP>>> Pool::objectsPool;
map<string,double> Pool::factorsPool;

























