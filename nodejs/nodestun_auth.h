#ifndef NODESTUN_AUTH_PROVIDER
#define NODESTUN_AUTH_PROVIDER

#include <node.h>
#include "commonincludes.hpp"
#include "stunauth.h"
#include "nodestun_authpoller.h"


class NodeStun_Auth : public IStunAuth, public CBasicRefCount{
public:
  virtual HRESULT DoAuthCheck(AuthAttributes* pAuthAttributes, AuthResponse* pResponse);
  NodeStun_Auth(v8::Handle<v8::Object> stun);
  v8::Handle<v8::Object> server_;
  ADDREF_AND_RELEASE_IMPL();
};


#endif
