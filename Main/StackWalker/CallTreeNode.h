#pragma once
#include <string>
#include <vector>
#include <iostream>

//extern int gCount;

class CallTreeNode
{
public:
  std::string               name;
  int                       count = 0;
  double                    percentage = 0;
  std::vector<CallTreeNode> children;
  std::vector<CallTreeNode> parents;

  CallTreeNode(std::string name, int count) // Add to createdNodes when creating a node.
  {
    this->name = name;
    this->count = count;
    //gCount += 1;
    //std::cout << "Creating" << std::endl;
    //createdNodes.insert(name);
  }

  void SetParent(CallTreeNode parent) { this->parents.push_back(parent); }

  CallTreeNode() { /*gCount += 1;*/ }

  ~CallTreeNode()
  {
    ///*for (size_t i = 0; i < this->children.size(); i++)
    //{
    //  delete (&this->children[i]);
    //}*/
    //free(this);
    //gCount -= 1;
    //std::cout << "Deleting" << std::endl;
  }

  void AddChild(CallTreeNode child) { this->children.push_back(child); }

  void SetCount(int count) { this->count = count; }

  int GetChildCount() { return (int)this->children.size(); }

  std::string SerialiseToJSON();
  std::string GetSerialisedName();
  std::string GetSerialisedCount();
  std::string GetSerialisedPercentage();
  std::string GetSerialisedChildren();
};
