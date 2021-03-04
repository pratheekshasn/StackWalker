#include "GraphCreator.h"
#include <fstream>
#include <iostream>
#include <memory>
#include <set>

std::shared_ptr<CallTreeNode> GetBranch(std::vector<std::string> callStack)
{
  // Have to DFS to check if callStack[0]] already exists. If it does, increment all its parents (?)
  std::shared_ptr<CallTreeNode> child =
      std::make_shared<CallTreeNode>(callStack[0], 1); // Top of the stack. Leaf of the call tree.
  std::shared_ptr<CallTreeNode> parent;

  for (size_t i = 1; i < callStack.size(); i++)
  {
    // Set parent's count to immediate child's count.
    parent = std::make_shared<CallTreeNode>(callStack[i],
                                            child->count); // Set count to LeafSampleCount();
    //child->SetParent(*parent);
    parent->AddChild(*child);
    child = parent;
  }

  return parent;
}

//CallTreeNode* CreateIfNotExists(CallTreeNode parent, std::string nodeName)
//{
//  // Add code for checking if node already exists.
//  return new CallTreeNode(nodeName, 1);
//}
//
//CallTreeNode* GetBranch(CallTreeNode* parent, std::vector<std::string> callStack)
//{
//  if (callStack.size() == 0)
//    return NULL; // NULL?
//  CallTreeNode* child = CreateIfNotExists(*parent, callStack[callStack.size() - 1]);
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

//int GetMaxListLength(std::vector<CallTreeNode> children)
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

std::set<std::string> CreateSetOfChildren(std::vector<CallTreeNode> children)
{
  std::set<std::string> funcSet;

  for (size_t i = 0; i < children.size(); i++)
  {
    funcSet.insert(children[i].name);
  }

  return funcSet;
}

CallTreeNode SetPercentages(CallTreeNode* root, int rootSampleCount)
{
  for (size_t i = 0; i < root->children.size(); i++)
  {
    CallTreeNode* node = &root->children[i];
    node->percentage = (node->count / rootSampleCount) * 100.0;

    *node = SetPercentages(node, rootSampleCount);
  }

  return *root;
}

std::shared_ptr<CallTreeNode> MergeNodes(std::shared_ptr<CallTreeNode> root)
{
  /*
	root ---->n1->n2->n3
	-->n1->n2->n3
	Merge branches and set count on the duplicate nodes.
	*/
  std::set<std::string> funcSet = CreateSetOfChildren(root->GetChildren());
  if (funcSet.size() != root->GetChildCount())
  {
    // Merge.
    std::set<std::string>::iterator iter;
    for (iter = funcSet.begin(); iter != funcSet.end(); iter++)
    {
      std::string      funcName = *iter;
      std::vector<int> duplicateIndices;
      for (int i = 0; i < root->GetChildCount(); i++)
      {
        if (funcName == root->GetChildAt(i)->GetName())
          duplicateIndices.push_back((int)i);
      }

      //CallTreeNode*  childAt = root->GetChildAt(duplicateIndices[0]);
      CallTreeNode* node = (root->GetChildAt(duplicateIndices[0])); // &(childAt);
      node->SetCount(max(1, (int)duplicateIndices.size()));
      for (size_t i = (duplicateIndices.size() - 1); i >= 1; i--)
      {
        // Add children of the node that will get erased.
        //CallTreeNode  dupChild = (root->GetChildAt(duplicateIndices[i]));
        CallTreeNode* nodeToErase = (root->GetChildAt(duplicateIndices[i])); // &dupChild;
        for (int j = 0; j < nodeToErase->GetChildCount(); j++)
        {
          CallTreeNode* adoptedChild = nodeToErase->GetChildAt(j);
          node->AddChild(*adoptedChild);
        }
        root->DeleteChildAt(duplicateIndices[i]);
      }
    }
  }
  for (int i = 0; i < root->GetChildCount(); i++)
  {
    // This level has finished merging at this point. So count the number of samples.
    auto updatedNode = MergeNodes(std::make_shared<CallTreeNode>(*(root->GetChildAt(i))));
    root->SetUpdatedChildAt(i, *updatedNode);
  }
  return root;
}

std::shared_ptr<CallTreeNode> CreateTree(std::string                           key,
                                         std::vector<std::vector<std::string>> values)
{
  std::shared_ptr<CallTreeNode> root = std::make_shared<CallTreeNode>(key, (int)values.size());
  //std::vector<std::vector<std::string>>::iterator iter = values.begin();
  for (size_t i = 0; i < values.size(); i++)
  {
    if (values[i].size() > 0)
    {
      std::shared_ptr<CallTreeNode> branch = GetBranch(values[i]);
      root->AddChild(*branch); // Added * here.
    }
  }

  //CallTreeNode* node = (root->GetChildAt(0)); // &(childAt);
  //node->SetCount(5);

  // Merge nodes in branch.
  root = MergeNodes(root);
  //root = std::make_shared<CallTreeNode>(mergedRoot); // Maybe don't use get()

  //root = SetPercentages(&root, (&(root))->count);

  return root;
}

//void UpdateCount(CallTreeNode* node) // This is the VI.
//{
//  if (node->children.size() == 0) // leaf node.
//  {
//    node->SetCount(1);
//    return;
//  }
//  for each(CallTreeNode child in(node->children)) { UpdateCount(&child); }
//}
//
//void SetCount(std::vector<CallTreeNode*> forestRoot)
//{
//  for (size_t i = 0; i < forestRoot.size(); i++)
//  {
//    UpdateCount(forestRoot[i]);
//  }
//}

void CreateGraphAndJSON(std::map<std::string, std::vector<std::vector<std::string>>> callTrees)
{
  std::vector<CallTreeNode>                                              forest;
  std::map<std::string, std::vector<std::vector<std::string>>>::iterator iter = callTrees.begin();

  std::ofstream JSONFile;

  JSONFile.open("C:\\temp\\JSON.json", std::ios ::trunc);

  JSONFile << "[";
  while (iter != callTrees.end())
  {
    std::string                           key = iter->first;
    std::vector<std::vector<std::string>> value = iter->second;
    std::shared_ptr<CallTreeNode>         tree = CreateTree(key, value);
    //(&tree)->percentage = (tree.count / (int)callTrees.size()) * 100.0;
    forest.push_back(*tree);

    std::string JSON = (*tree).SerialiseToJSON();

    //delete tree;
    JSONFile << JSON;

    iter++;

    if (iter != callTrees.end())
      JSONFile << ",";
  }
  JSONFile << "]";

  // This forest has repeated nodes, merge the tree.
  // Set count for number of occurances. Count seems okay for root VI nodes.

  /*CallTreeNode* forestRoot = new CallTreeNode("root", -1);
  forestRoot->children = forest;*/
  //SetCount(forest);
  //forest[0]).SerialiseToJSON();
}