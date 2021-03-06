//#include "c_plus_plus_serializer.h"
//#include <map>
//#include <vector>
//#include <Bits.h>
//
//class Custom
//{
//public:
//  int         a;
//  std::string b;
//  std::vector<std::string> c;
//
//  friend std::ostream& operator<<(std::ostream& out, Bits<int> my)
//  {
//    out << bits(my.t.a) << bits(my.t.b) << bits(my.t.c);
//    return (out);
//  }
//
//  friend std::istream& operator>>(std::istream& in, Bits<int> my)
//  {
//    in >> bits(my.t.a) >> bits(my.t.b) >> bits(my.t.c);
//    return (in);
//  }
//
//  friend std::ostream& operator<<(std::ostream& out, class Custom& my)
//  {
//    out << "a:" << my.a << " b:" << my.b;
//
//    out << " c:[" << my.c.size() << " elems]:";
//    for (auto v : my.c)
//    {
//      out << v << " ";
//    }
//    out << std::endl;
//
//    return (out);
//  }
//};
//
//static void save_map_key_string_value_custom(const std::string filename)
//{
//  std::cout << "save to " << filename << std::endl;
//  std::ofstream out(filename, std::ios::binary);
//
//  std::map<std::string, class Custom> m;
//
//  auto c1 = Custom();
//  c1.a = 1;
//  c1.b = "hello";
//  std::initializer_list<std::string> L1 = {"vec-elem1", "vec-elem2"};
//  std::vector<std::string>           l1(L1);
//  c1.c = l1;
//
//  auto c2 = Custom();
//  c2.a = 2;
//  c2.b = "there";
//  std::initializer_list<std::string> L2 = {"vec-elem3", "vec-elem4"};
//  std::vector<std::string>           l2(L2);
//  c2.c = l2;
//
//  m.insert(std::make_pair(std::string("key1"), c1));
//  m.insert(std::make_pair(std::string("key2"), c2));
//
//  out << bits(m);
//}
//
//static void load_map_key_string_value_custom(const std::string filename)
//{
//  std::cout << "read from " << filename << std::endl;
//  std::ifstream in(filename);
//
//  std::map<std::string, class Custom> m;
//
//  in >> bits(m);
//  std::cout << std::endl;
//
//  std::cout << "m = " << m.size() << " list-elems { " << std::endl;
//  for (auto i : m)
//  {
//    std::cout << "    [" << i.first << "] = " << i.second;
//  }
//  std::cout << "}" << std::endl;
//}
//
//void map_custom_class_example(void)
//{
//  std::cout << "map key string, value class" << std::endl;
//  std::cout << "============================" << std::endl;
//  save_map_key_string_value_custom(std::string("map_of_custom_class.bin"));
//  load_map_key_string_value_custom(std::string("map_of_custom_class.bin"));
//  std::cout << std::endl;
//}