#pragma once
#ifndef _GraphCreator_H_

#define _GraphCreator_H_
#include "CallTreeNode.h"
#include <map>
#include <string>
#include <vector>

CallTreeNode GetBranch(std::vector<std::string> callStack);
//CallTreeNode* GetBranch(CallTreeNode* parent, std::vector<std::string> callStack);
CallTreeNode CreateTree(std::string key, std::vector<std::vector<std::string>> values);
void  CreateGraphAndJSON(std::map<std::string, std::vector<std::vector<std::string>>> callTrees);
CallTreeNode SetPercentages(CallTreeNode* root, int rootSampleCount);

#endif // !_GraphCreator_H_
