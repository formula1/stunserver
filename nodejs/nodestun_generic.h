#ifndef NODESTUN_GENERIC
#define NODESTUN_GENERIC


#include "commonincludes.hpp"
#include <node.h>


namespace NS_G
{
  std::string v8str2stdstr(v8::Handle<v8::String> ori);
  bool throwErr(const std::string err);
};


#endif
