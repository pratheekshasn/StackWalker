#include "GraphCreator.h"
#include <fstream>
#include <set>

std::ofstream JSONFile;

Node GetBranch(std::vector<std::string> callStack)
{
  // Have to DFS to check if callStack[0]] already exists. If it does, increment all its parents (?)
  Node* child = new Node(callStack[0], 1); // Top of the stack. Leaf of the call tree.
  Node* parent;

  for (size_t i = 1; i < callStack.size(); i++)
  {
    // Set parent's count to immediate child's count.
    parent = new Node(callStack[i], child->count); // Set count to LeafSampleCount();
    child->SetParent(*parent);
    parent->AddChild(*child);
    child = parent;
  }

  return *parent;
}

//Node* CreateIfNotExists(Node parent, std::string nodeName)
//{
//  // Add code for checking if node already exists.
//  return new Node(nodeName, 1);
//}
//
//Node* GetBranch(Node* parent, std::vector<std::string> callStack)
//{
//  if (callStack.size() == 0)
//    return NULL; // NULL?
//  Node* child = CreateIfNotExists(*parent, callStack[callStack.size() - 1]);
//  callStack.pop_back();
//  child = GetBranch(child, callStack);
//  if (child != NULL)
//  {
//    child->SetCount(child->GetChildCount());
//    return child;
//  }
//  else
//    return parent;
//}

//int GetMaxListLength(std::vector<Node> children)
//{
//  int max = -1;
//
//  for (size_t i = 0; i < children.size(); i++)
//  {
//    if (children[i].children.size() > max)
//      max = children[i].children.size();
//  }
//  return max;
//}

int max(int a, int b)
{
  return (a > b) ? a : b;
}

std::set<std::string> CreateSetOfChildren(std::vector<Node> children)
{
  std::set<std::string> funcSet;

  for (size_t i = 0; i < children.size(); i++)
  {
    funcSet.insert(children[i].name);
  }

  return funcSet;
}

Node MergeNodes(Node* root)
{
  /*
    root ---->n1->n2->n3
           -->n1->n2->n3

    Merge branches and set count on the duplicate nodes.
    */

  std::set<std::string> funcSet = CreateSetOfChildren(root->children);

  if (funcSet.size() != root->children.size())
  {
    // Merge.

    std::set<std::string>::iterator iter;
    for (iter = funcSet.begin(); iter != funcSet.end(); iter++)
    {
      std::string funcName = *iter;
      //std::vector<Node*> mergables;
      std::vector<int> duplicateIndices;

      for (size_t i = 0; i < root->children.size(); i++)
      {
        if (funcName == root->children[i].name)
          duplicateIndices.push_back(i);
      }

      //Node* first = mergables[0];

      Node* node = &(root->children[duplicateIndices[0]]);
      node->SetCount(max(1, duplicateIndices.size()));

      for (size_t i = 1; i < duplicateIndices.size(); i++)
      {
        // Add children of the node that will get erased.

        Node* nodeToErase = &root->children[i];
        for (size_t j = 0; j < nodeToErase->children.size(); j++)
        {
          node->AddChild(nodeToErase->children[j]);
        }
        root->children.erase(root->children.begin() + i);
      }
    }
  }

  for (size_t i = 0; i < root->children.size(); i++)
  {
    Node updatedNode = MergeNodes(&(root->children[i]));
    root->children[i] = updatedNode;
  }

  return *root;
}

Node* CreateTree(std::string key, std::vector<std::vector<std::string>> values)
{
  Node* root = new Node(key, values.size());
  //std::vector<std::vector<std::string>>::iterator iter = values.begin();
  for (size_t i = 0; i < values.size(); i++)
  {
    auto branch = GetBranch(values[i]);
    root->AddChild(branch); // Added * here.
  }

  // Merge nodes in branch.
  *root = MergeNodes(root);
  std::string JSON = root->SerialiseToJSON();

  JSONFile << JSON;
  return root;
}

//void UpdateCount(Node* node) // This is the VI.
//{
//  if (node->children.size() == 0) // leaf node.
//  {
//    node->SetCount(1);
//    return;
//  }
//  for each(Node child in(node->children)) { UpdateCount(&child); }
//}
//
//void SetCount(std::vector<Node*> forestRoot)
//{
//  for (size_t i = 0; i < forestRoot.size(); i++)
//  {
//    UpdateCount(forestRoot[i]);
//  }
//}

void CreateGraph(std::map<std::string, std::vector<std::vector<std::string>>> callTrees)
{
  std::vector<Node*>                                                     forest;
  std::map<std::string, std::vector<std::vector<std::string>>>::iterator iter = callTrees.begin();

  JSONFile.open("C:\\temp\\JSON.json", std::ios ::app);

  JSONFile << "[";
  while (iter != callTrees.end())
  {
    std::string                           key = iter->first;
    std::vector<std::vector<std::string>> value = iter->second;
    Node*                                 tree = CreateTree(key, value);
    forest.push_back(tree);
    iter++;

    if (iter != callTrees.end())
      JSONFile << ",";
  }
  JSONFile << "]";

  // This forest has repeated nodes, merge the tree.
  // Set count for number of occurances. Count seems okay for root VI nodes.

  /*Node* forestRoot = new Node("root", -1);
  forestRoot->children = forest;*/
  //SetCount(forest);
  //forest[0]).SerialiseToJSON();
}