#pragma once
#include <string>
#include <vector>

class Node
{
public:
  std::string       name;
  int               count;
  std::vector<Node> children;

  Node(std::string name, int count) // Add to createdNodes when creating a node.
  {
    this->name = name;
    this->count = count;

    //createdNodes.insert(name);
  }

  Node() {}

  void AddChild(Node child) { this->children.push_back(child); }

  void IncrementCount() { this->count++; }
};
