#ifndef NODESTUN_ARGS_H
#define NODESTUN_ARGS_H


#include "commonincludes.hpp"
#include <node.h>
#include "server.h"


class NodeStun_Args
{
public:
  static bool OneArgs(const v8::Arguments& args, v8::Local<v8::Object>& option_map);
  static bool ThreeArgs(const v8::Arguments& args, v8::Local<v8::Object>& option_map);
  static bool Object2Config(v8::Local<v8::Object>& option_map, CStunServerConfig& config);
};


#endif
