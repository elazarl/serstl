#include "serstl.h"

template <typename T>
std::ostream& operator << (std::ostream& os, const std::vector<T>& vec)
{
    std::copy( vec.begin(), vec.end(), std::ostream_iterator<T>( os ,",") );
    return os;
}

template<typename T>
void getEquals(const T& expected) {
  std::stringstream ss;
  serstl::put(ss,expected);

  T t;
  serstl::Error err = serstl::get(ss,&t);
  if (err.isError()) std::cerr << "get failed: " << err.what() << std::endl;

  if (ss.eof()) std::cerr << "too much input consumed\n";
  if (ss.peek() && !ss.eof()) {
    std::cerr << "too little input consumed\n";
    std::string rest;
    ss >> rest;
    std::cerr << "rest of input:" << rest << std::endl;
  }
  if (t != expected) {
    std::cerr << "packed  : " << ss.str() << std::endl;
    /*std::cerr << "expected: " << expected << std::endl;
    std::cerr << "result  :  " << t << std::endl;*/
  }
}

int main(int,char**) {
  using namespace std;
  /*vector<vector<string> > vv;
  stringstream ss("llee");
  get(ss,&vv);
  string tmpstr;
  ss >> tmpstr;
  cout << "GOT " << vv.size() << ".\n";
  cout << "got " << tmpstr << ".\n";
  return 0;*/

  vector<int> v_int;
  v_int.push_back(1);
  v_int.push_back(2);
  v_int.push_back(3);

  getEquals(v_int);
  std::cout << "tested vector<int>\n";

  vector<string> v_str,v_str_result;
  v_str.push_back("bobo");
  v_str.push_back("eskimosi");
  v_str.push_back("momo");

  getEquals(v_str);
  std::cout << "tested vector<string>\n";

  vector<vector<int> > v_empty;
  getEquals(v_empty);
  std::cout << "tested empty vector\n";

  vector<vector<int> > v_v_int;
  v_v_int.push_back(v_int);
  v_v_int.push_back(vector<int>());
  
  getEquals(v_v_int);
  std::cout << "tested vector<vector<int>>\n";

  pair<int,string> p(12,"aa");
  getEquals(p);
  std::cout << "tested pair<int,string>\n";

  map<string,int> m;
  m["a"] = 1;
  m["b"] = 2;
  m["ab"] = 12;
  getEquals(m);
  std::cout << "tested map<string,int>\n";

  map<string,vector<pair<int,string> > > m_s_v_p_i_s;
  vector<pair<int,string> > v_p_i_s;
  v_p_i_s.push_back(pair<int,string>(12,"gogo"));
  m_s_v_p_i_s["aa"] = v_p_i_s;
  getEquals(m_s_v_p_i_s);
  std::cout << "tested map<string<vector<pair<int,string>>>\n";
}
