/*
Boost Software License - Version 1.0 - August 17th, 2003

Permission is hereby granted, free of charge, to any person or organization
obtaining a copy of the software and accompanying documentation covered by
this license (the "Software") to use, reproduce, display, distribute,
execute, and transmit the Software, and to prepare derivative works of the
Software, and to permit third-parties to whom the Software is furnished to
do so, all subject to the following:

The copyright notices in the Software and this entire statement, including
the above license grant, this restriction and the following disclaimer,
must be included in all copies of the Software, in whole or in part, and
all derivative works of the Software, unless such copies or derivative
works are solely in the form of machine-executable object code generated by
a source language processor.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/


//  Name: LiC++ - Let it Crash in C++ - Patient Box Example
//  Copyright: Boost Software License - Version 1.0 - August 17th, 2003
//  Author: Christoph Woskowski, cwo_at_zuehlke.com
//
//  Description:
//    Common header of the LiC++ variant of the Patient Monitoring example.
//    It contains some global messages.
//    An explanation can be found here:
//    http://blog.zuehlke.com/en/is-it-safe-to-let-it-crash/

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
