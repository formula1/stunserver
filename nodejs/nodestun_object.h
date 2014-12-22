#ifndef STUNSERVER_H
#define STUNSERVER_H

#include <node.h>
#include "server.h"
#include "nodestun_auth.h"


class NodeStun : public node::ObjectWrap {
public:
  static void Init(v8::Handle<v8::Object> exports);

private:
  NodeStun(CStunServerConfig config, v8::Handle<v8::Object> nt);
  ~NodeStun();

  static v8::Handle<v8::Value> New(const v8::Arguments& args);
  static v8::Handle<v8::Value> Start(const v8::Arguments& args);
  static v8::Handle<v8::Value> Stop(const v8::Arguments& args);
  static v8::Handle<v8::Value> AbstractThrow(const v8::Arguments& args);
  static v8::Persistent<v8::Function> constructor;
  static void DoNothing(uv_idle_t* handle, int status);
  CStunServerConfig instance_config_;
  CStunServer* instance_server_;
  uv_idle_t idler;
  NodeStun_Auth* stunAuth;
  v8::Handle<v8::Object> NodeThis;
};

#endif
