#include <map>
#include <string>
#include <vector>

void WriteKeyValue(std::string key, std::vector<std::vector<std::string>> value)
{
}

void CreateJSON(std::map<std::string, std::vector<std::vector<std::string>>> callTrees)
{
  std::map<std::string, std::vector<std::vector<std::string>>>::iterator iter = callTrees.begin();

  while (iter != callTrees.end())
  {
    std::string                           key = iter->first;
    std::vector<std::vector<std::string>> value = iter->second;
    WriteKeyValue(key, value);
  }
}