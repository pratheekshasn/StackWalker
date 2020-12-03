#pragma once
#ifndef _GraphCreator_H_

#define _GraphCreator_H_
#include "Node.h"
#include <map>
#include <string>
#include <vector>

Node GetBranch(std::vector<std::string> callStack);
//Node* GetBranch(Node* parent, std::vector<std::string> callStack);
Node* CreateTree(std::string key, std::vector<std::vector<std::string>> values);
void  CreateGraph(std::map<std::string, std::vector<std::vector<std::string>>> callTrees);

#endif // !_GraphCreator_H_
