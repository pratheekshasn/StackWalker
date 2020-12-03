#include "GraphCreator.h"

//Node GetBranch(Node* root, std::vector<std::string> callStack)
//{
//  // Have to DFS to check if callStack[0]] already exists. If it does, increment all its parents (?)
//  Node* child = new Node(callStack[0], 1); // Top of the stack. Leaf of the call tree.
//  Node* parent;
//
//  for (size_t i = 1; i < callStack.size(); i++)
//  {
//    // Set parent's count to immediate child's count.
//    parent = new Node(callStack[i], child->count); // Set count to LeafSampleCount();
//    child->SetParent(*parent);
//    parent->AddChild(*child);
//    child = parent;
//  }
//
//  return *parent;
//}

Node* CreateIfNotExists(Node* parent, std::string nodeName)
{
  // Add code for checking if node already exists.
  return new Node(nodeName, 1);
}

Node* GetBranch(Node* parent, std::vector<std::string> callStack)
{
  if (callStack.size() == 0)
    return NULL; // NULL?
  Node* child = CreateIfNotExists(parent, callStack[callStack.size() - 1]);
  callStack.pop_back();
  child = GetBranch(child, callStack);
  if (child == NULL)
    return parent;

  child->SetCount(child->GetChildCount());
}

Node* CreateTree(std::string key, std::vector<std::vector<std::string>> values)
{
  Node* root = new Node(key, values.size());
  //std::vector<std::vector<std::string>>::iterator iter = values.begin();
  for (size_t i = 0; i < values.size(); i++)
  {
    auto branch = GetBranch(root, values[i]);
    root->AddChild(*branch); // Added * here.
  }
  return root;
}

void CreateGraph(std::map<std::string, std::vector<std::vector<std::string>>> callTrees)
{
  std::vector<Node*>                                                     forest;
  std::map<std::string, std::vector<std::vector<std::string>>>::iterator iter = callTrees.begin();

  while (iter != callTrees.end())
  {
    std::string                           key = iter->first;
    std::vector<std::vector<std::string>> value = iter->second;
    forest.push_back(CreateTree(key, value));
    iter++;
  }
}