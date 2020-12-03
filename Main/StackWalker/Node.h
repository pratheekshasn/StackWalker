#pragma once
#include <string>
#include <vector>

class Node
{
public:
  std::string       name;
  int               count;
  std::vector<Node> children;
  std::vector<Node> parents;

  Node(std::string name, int count) // Add to createdNodes when creating a node.
  {
    this->name = name;
    this->count = count;

    //createdNodes.insert(name);
  }

  void SetParent(Node parent) { this->parents.push_back(parent); }

  Node() {}

  void AddChild(Node child) { this->children.push_back(child); }

  void SetCount(int count) { this->count = count; }

  int GetChildCount() { return this->children.size(); }
};
