#include "Node.h"
#include <map>
#include <string>
#include <vector>

Node GetBranch(std::vector<std::string> callStack)
{
  Node* leaf = new Node(callStack[0], 1);
  Node* parent;

  for (size_t i = 1; i < callStack.size(); i++)
  {
    parent = new Node(callStack[i], 1);
    parent->AddChild(*leaf);
    leaf = parent;
  }

  return *parent;
}

Node* CreateTree(std::string key, std::vector<std::vector<std::string>> values)
{
  Node* root = new Node(key, values.size());
  //std::vector<std::vector<std::string>>::iterator iter = values.begin();
  for (size_t i = 0; i < values.size(); i++)
  {
    auto branch = GetBranch(values[i]);
    root->AddChild(branch);
  }
  return root;
}

void CreateGraph(std::map<std::string, std::vector<std::vector<std::string>>> callTrees)
{
  std::vector<Node*>                                                      forest;
  std::map<std::string, std::vector<std::vector<std::string>>>::iterator iter = callTrees.begin();

  while (iter != callTrees.end())
  {
    std::string                           key = iter->first;
    std::vector<std::vector<std::string>> value = iter->second;
    forest.push_back(CreateTree(key, value));
    iter++;
  }
}