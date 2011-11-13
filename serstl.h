#ifndef SERSTL_H_
#define SERSTL_H_

#include <vector>
#include <list>
#include <map>
#include <string>

#include <iostream>
#include <sstream>

#include <iterator>

namespace serstl {

namespace {
template <class T>
bool from_string(T& t, 
                     const std::string& s)
{
    std::istringstream iss(s);
      return !(iss >> t).fail();
}
template <typename T>
std::string to_string(const T& t) {
  std::ostringstream oss;
  oss << t;
  return oss.str();
}
void write_raw(std::ostream& out, const std::string& s) {
  out.write(s.c_str(),s.size());
}
}

class Error {
public:
  std::string what() {
    if (msg) return msg;
    return "";
  }
  bool isError() {return msg != NULL;}
  Error(const char* _msg):msg(_msg){}
  static Error noError() {return Error(NULL);}
private:
  const char* msg;
};

const unsigned int MEG = 1024 * 1024;
const unsigned int MAX_LEN = 10*MEG;

Error put(std::ostream& out, const std::string& data) {
  if (data.size() > MAX_LEN) return false;
  out << to_string(data.size()) << ":" << data;
  return Error::noError();
}

Error put(std::ostream& out, const int& data) {
  write_raw(out,"i");
  write_raw(out,to_string(data));
  write_raw(out,"e");
  return Error::noError();
}

template<typename T>
Error put(std::ostream& out, const std::list<T>& data) {
  write_raw(out,"l");
  for (typename std::list<T>::const_iterator it(data.begin());it != data.end();++it) {
    Error err = put(out,*it);
    if (err.isError()) return err;
  }
  write_raw(out,"e");
  return Error::noError();
}

template<typename T>
Error put(std::ostream& out, const std::vector<T>& data) {
  write_raw(out,"l");
  for (typename std::vector<T>::const_iterator it(data.begin());it != data.end();++it) {
    Error err = put(out,*it);
    if (err.isError()) return err;
  }
  write_raw(out,"e");
  return Error::noError();
}

template<typename T,typename U>
Error put(std::ostream& out, const std::pair<T,U>& data) {
  write_raw(out,"l");
  put(out,data.first);
  put(out,data.second);
  write_raw(out,"e");
  return Error::noError();
}

template<typename T,typename U>
Error put(std::ostream& out, const std::map<T,U>& data) {
  write_raw(out,"l");
  for (typename std::map<T,U>::const_iterator it(data.begin());it != data.end();++it) {
    Error err = put(out,*it);
    if (err.isError()) return err;
  }
  write_raw(out,"e");
  return Error::noError();
}

Error readnum(std::istream& in,int* output) {
  int num = 0;
  int c = '0';
  int digitsRead = -1;
  while (c >= '0' && c <= '9') {
    digitsRead++;
    num *= 10;
    num += c-'0';
    c = in.get();
  }
  if (!in.eof()) in.unget();
  if (digitsRead == 0) {
    return Error("premature EOS when expecting a number");
  }
  *output = num;
  return Error::noError();
}

Error readsignednum(std::istream& in,int* output) {
  int c = in.get();
  if (in.eof()) {
    return Error("premature EOS, when expecting a number");
  }
  if (c == '-') {
    Error err = readnum(in,output);
    if (err.isError()) return err;
    *output = -*output;
    return Error::noError();
  }
  if (c >= '0' && c <= '9') {
    in.unget();
    return readnum(in,output);
  }
  return Error("Prefix of a signed number is not numeric and not '-'");
}

Error get(std::istream& in, std::string* data) {
  int num;
  Error err = readnum(in,&num);
  if (err.isError()) {
    return err;
  }
  int c = in.get();
  if (c != ':') {
    return Error("non colon after number, when reading byte string");
  }
  if (num > int(MAX_LEN) || num < 0) return Error("byte string length larger than hardcoded maximal length");
  std::vector<char> buf(num);
  in.read(&buf.front(),num);
  *data = std::string(&buf.front(),num);

  return Error::noError();
}

Error get(std::istream& in, int* data) {
  int c = in.get();
  if (c != 'i') {
    in.unget();
    return Error("no 'i' prefix when reading an integer");
  }
  Error err = readsignednum(in,data);
  if (err.isError()) return err;
  
  int postint = in.get();

  if (postint != 'e') return Error("no 'e' postfix when reading an integer");
  return Error::noError();
}

template<typename T>
Error get(std::istream& in,std::vector<T>* data) {
  int prefix = in.get();
  if (prefix != 'l') {
    in.unget();
    return Error("no 'l' prefix when reading a vector");
  }
  while (true) {
    T elt;
    if (in.eof()) {
      return Error("premature ending of stream while reading a vector");
    }
    Error err = get(in,&elt);
    if (err.isError()) {
      if (in.eof()) {
        return Error("EOF before list end token");
      }
      int postfix = in.get();
      if (postfix != 'e') {
        return Error("no 'e' postfix when reading a vector");
      }
      return Error::noError();
    }
    data->push_back(elt);
  }
}

template<typename T>
Error get(std::istream& in, std::list<T>* data) {
  std::vector<T> v;
  Error err = get(in,&v);
  if (err.isError()) return err;
  std::copy(v.begin(),v.end(),std::back_inserter(*data));
}

template<typename T,typename U>
Error get(std::istream& in, std::pair<T,U>* data) {
  int prefix = in.get();
  if (prefix != 'l') {
    in.unget();
    return Error("no 'l' prefix for pair");
  }
  Error errFirst = get(in,&data->first);
  if (errFirst.isError()) return errFirst;
  Error errSecnd = get(in,&data->second);
  if (errSecnd.isError()) return errSecnd;
  
  int postfix = in.get();
  if (postfix != 'e') {
    return Error("pair postfix != e");
  }
  return Error::noError();
}

template<typename T,typename U>
Error get(std::istream& in, std::map<T,U>* data) {
  int prefix = in.get();
  if (prefix != 'l') {
    in.unget();
    return Error("no 'l' prefix for map");
  }

  while (true) {
    std::pair<T,U> p;
    if (get(in,&p).isError()) break;
    data->insert(p);
  }
  
  int postfix = in.get();
  if (postfix != 'e') {
    return Error("map postfix != e");
  }
  return Error::noError();
}

}

#endif /* SERSTL_H_ */
