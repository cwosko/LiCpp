#ifndef COMMON_H_
#define COMMON_H_

#include "LiC++.h"

using namespace std;
using namespace LiCpp;

struct msg_query {
  msg_query(ThreadId src_) : src(src_) {};
  ThreadId src;
};

struct msg_value {
  msg_value(ThreadId src_, std::vector<double> value_) : src(src_), value(value_) {};
  ThreadId src;
  std::vector<double> value;
};

#endif /* COMMON_H_ */
