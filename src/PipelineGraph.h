#pragma once
#include <iostream> // std::cout
#include <utility> // std::pair
#include <vector>
#include "Action.h"
// #include <boost/graph/graph_traits.hpp>
// #include <boost/graph/adjacency_list.hpp>
// #include <boost/graph/topological_sort.hpp>


class PipelineGraph
{
	vector<Action*> *_actionslist;
public:
	 PipelineGraph(){};
	~PipelineGraph(void);

	void addList(vector<Action*> &actionslist)
	{
		// _actionslist = &actionslist;
  	}
	void buildgraph();
	string traceBack(string origin_name,Action &act)
	{
		return "NAME";
	}
};

