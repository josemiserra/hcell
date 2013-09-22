#include "PoolManager.h"

shared_ptr<PoolManager> PoolManager::instance=0;

int PoolManager::maxObjects=0;
int PoolManager::maxImages=0;

int PoolManager::numImages=0;
int PoolManager::numObjects=0;

vector<shared_ptr<Slot<Mat>>> PoolManager::imagesPool;
vector<shared_ptr<Slot<vloP>>> PoolManager::objectsPool;


























