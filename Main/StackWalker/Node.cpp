#include "Node.h"
#include <sstream>

std::string ReplaceAll(std::string str, const std::string& from, const std::string& to);

std::string Node::SerialiseToJSON()
{
  //char* serialisedString = "";
  /*sprintf(serialisedString, "{\"name\":\"%s\",\"count\":%d,\"children\":[%s]}", GetSerialisedName(),
          GetSerialisedCount(), GetSerialisedChildren());*/

  /*sprintf(serialisedString, "{\"name\":\"%s\",\"count\":%d,\"children\":[%s]}", this->name,
          this->count, GetSerialisedChildren());*/

  std::stringstream serialisedString;

  serialisedString << "{";
  serialisedString << GetSerialisedName() << ",";
  serialisedString << GetSerialisedCount() << ",";
  serialisedString << GetSerialisedChildren();
  serialisedString << "}";
  return serialisedString.str();
}

std::string Node::GetSerialisedName()
{
  std::stringstream serialisedString;
  serialisedString << "\"name\":\"" << ReplaceAll(this->name, "\\", "\\\\")
                   << "\""; // ReplaceAll(this->name, "\\", "\\\\");
  return serialisedString.str();
}

std::string Node::GetSerialisedCount()
{
  std::stringstream serialisedString;
  serialisedString << "\"count\":" << this->count;
  return serialisedString.str();
}

std::string Node::GetSerialisedChildren()
{
  if (this->children.size() == 0)
  {
    return "\"children\":[]";
  }

  std::stringstream serialisedString;
  serialisedString << "\"children\":[";

  for (size_t i = 0; i < this->children.size(); i++)
  {
    serialisedString << this->children[i].SerialiseToJSON();
    if (i < this->children.size() - 1)
      serialisedString << ",";
  }
  serialisedString << "]";
  return serialisedString.str();
}
