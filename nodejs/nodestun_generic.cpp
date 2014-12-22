
#include "commonincludes.hpp"
#include <node.h>
#include "nodestun_generic.h"


using namespace v8;

namespace NS_G
{
  bool throwErr(std::string err){
    ThrowException(Exception::TypeError(String::New(err.c_str())));
    return true;
  }

  std::string v8str2stdstr(Handle<String> ori){
    String::Utf8Value param1(ori);
    std::string to = std::string(*param1);
    return to;
  }
}
