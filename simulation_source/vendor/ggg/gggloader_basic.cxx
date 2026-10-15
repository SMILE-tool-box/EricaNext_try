#include "gggloader.h"

double GGG_LIB::atof(const std::string str) {
  double sol;
  std::stringstream ss;
  ss << str;
  ss >> sol;
  return sol;
}

int GGG_LIB::atoi(const std::string str) {
  int sol;
  std::stringstream ss;
  ss << str;
  ss >> sol;
  return sol;
}


char GGG_LIB::get_one(std::string *str) {
  char sol = (*str)[0];
  std::string buf;
  for (unsigned int i = 0; i < (str->length()) - 1; i++) {
    buf += (*str)[i + 1];
  }
  *str = buf;
  return sol;
}

std::string GGG_LIB::split(std::string *str, const char del) {
  char buf;
  std::string input = *str;
  std::string sol = "";
  
  for (unsigned int i = 0; i < input.length(); i++) {
    buf = get_one(str);
    if (buf == 0x09)  continue;//tab
    //return 0x0D+0x0A
    if (buf == ' ' && del != ' ')  continue;
    
    if (buf == del) {
      break;
    }
    else {
      sol += buf;
    }
  }
  
  return sol;
}


void GGG_LIB::rmb_dblquot(std::string *str) {
  std::string str_buf = "";
  unsigned int length = str->length();
  for (unsigned int i = 0; i < length; i++) {
    char ch = get_one(str);
    if (ch != '\\' && ch != '\"') {
      str_buf += ch;
    }
  }
  *str = str_buf;
  return;
}


int GGG_LIB::sign(int a){
  if(a==0){
    return 0;
  }
  else if(a>0){
    return 1;
  }
  else{
    return -1;
  }
}
