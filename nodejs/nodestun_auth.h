#ifndef NODESTUN_AUTH_PROVIDER
#define NODESTUN_AUTH_PROVIDER

#include "commonincludes.hpp"
#include "stunauth.h"
#include <node.h>

class NodeStun_Auth : public IStunAuth {
public:
  NodeStun_Auth(v8::Handle<v8::Object> stun);
  virtual HRESULT DoAuthCheck(AuthAttributes* pAuthAttributes, AuthResponse* pResponse);
  v8::Handle<v8::Object> server_;
};

#endif
