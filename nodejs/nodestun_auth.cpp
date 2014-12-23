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
  : server_(stun)
  {

  }
HRESULT NodeStun_Auth::DoAuthCheck(AuthAttributes* pAuthAttributes, AuthResponse* pResponse)
{
  Logging::LogMsg(LL_VERBOSE, "inside");
  Authenticator auth;
  Handle<Function> fn = Handle<Function>::Cast(server_->Get(String::NewSymbol("doAuth")));
  Logging::LogMsg(LL_DEBUG,"doauthcheck");
  return auth.SendAndWait(pAuthAttributes, pResponse,fn);
}
