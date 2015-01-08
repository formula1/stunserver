/*
Copyright 2011 John Selbie

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include <node.h>
#include "commonincludes.hpp"
#include "stunauth.h"
#include "nodestun_authpoller.h"
#include "nodestun_auth.h"
/*

Do Auth Check:
-Create a new Authenticator with a given AuthAttributes
-

*/

using namespace v8;

NodeStun_Auth::NodeStun_Auth (Handle<Object> stun)
  : CBasicRefCount(),
    server_(stun)
  {
    HandleScope scope;
    this->AddRef();
  }
HRESULT NodeStun_Auth::DoAuthCheck(AuthAttributes* pAuthAttributes, AuthResponse* pResponse)
{

  Logging::LogMsg(LL_VERBOSE, "pre locking");
  if(Locker::IsActive()){
    Logging::LogMsg(LL_VERBOSE, "locked");
  }
  Locker v8Locker;
  Logging::LogMsg(LL_VERBOSE, "pre scope");
  HandleScope scope;
  Handle<ObjectTemplate> global = ObjectTemplate::New();
  Handle<Context> context = Context::New(NULL, global);
  Persistent<Context> context_ = Persistent<Context>::New(context);

  Logging::LogMsg(LL_VERBOSE, "pre authenticator");
  Authenticator auth;
  Logging::LogMsg(LL_VERBOSE, "pre register symbol");
  Local<String> sym = String::New("doAuth");
  Logging::LogMsg(LL_VERBOSE, "pre get value");
  Local<Value> val = server_->Get(sym);
  Logging::LogMsg(LL_VERBOSE, "pre cast fn");
  Handle<Function> fn = Handle<Function>::Cast(val);
  Logging::LogMsg(LL_DEBUG,"pre send and wait");
  HRESULT ret = auth.SendAndWait(pAuthAttributes, pResponse,fn);
  context_.Dispose();
  return ret;
}

//int NodeStun_Auth::AddRef(){return InternalAddRef();}
//int NodeStun_Auth::Release(){return InternalRelease();}
