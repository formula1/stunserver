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
#include <pthread.h>
#include "stunauth.h"
#include "nodestun_authpoller.h"
#include "nodestun_generic.h"
/*

Do Auth Check:
-Create a new Authenticator with a given AuthAttributes
-

*/

using namespace v8;

HRESULT Authenticator::SendAndWait(
  AuthAttributes* request,
  AuthResponse* response,
  Handle<Function> sender
)
{
  HandleScope scope;
  _isCompleted = false;

  Local<Object> AuthReq = Object::New();
  AuthReq->Set(String::NewSymbol("user"), String::New(request_.szUser));
  AuthReq->Set(String::NewSymbol("realm"), String::New(request_.szRealm));
  AuthReq->Set(String::NewSymbol("nonce"), String::New(request_.szNonce));
  AuthReq->Set(String::NewSymbol("password"), String::New(request_.szLegacyPassword));
  AuthReq->Set(String::NewSymbol("integrity"), Boolean::New(request_.fMessageIntegrityPresent));
  // attributes in the request

  Local<Object> AuthRes = Object::New();
  AuthRes->Set(String::NewSymbol("response_code"), Number::New(0));
  AuthRes->Set(String::NewSymbol("is_short_term"), Boolean::New(true));
  AuthRes->Set(String::NewSymbol("password"), String::New("password"));
  AuthRes->Set(String::NewSymbol("realm"), String::New("realm"));
  AuthRes->Set(String::NewSymbol("nonce"), String::New("private nonce"));

  Local<Function> Callback = FunctionTemplate::New(onNodeAuthCallback)->GetFunction();
  this->Wrap(Callback);
  Local<Value> args[3];
  args[0] = AuthReq;
  args[1] = AuthRes;
  args[2] = Callback;
  Logging::LogMsg(LL_DEBUG,"about to auth");
  sender->Call(sender,3,args);

  pthread_mutex_lock(&_mutex);
  while (_isCompleted == false)
  {
    pthread_cond_wait(&_cond, &_mutex);
  }
  pthread_mutex_unlock(&_mutex);
  memcpy(response, &response_, sizeof(AuthResponse));
  return S_OK;

}

// NOT SHOWN - that other NodeJS object mentioned above, upon getting the callback that the async auth request has completed, call this methods
Handle<Value> Authenticator::onNodeAuthCallback(const Arguments& args)
{
  HandleScope scope;
  if(args.Length() != 1){
    ThrowException(Exception::TypeError(String::New("Callback Expects only 1 Argument")));
    return scope.Close(Boolean::New(false));
  }
  if(!args[0]->IsObject()){
    ThrowException(Exception::TypeError(String::New("The callback expects an Object")));
    return scope.Close(Boolean::New(false));
  }
  Local<Object> AuthRes = args[0]->ToObject();
  int response_code = AuthRes->Get(String::NewSymbol("response_code"))->ToNumber()->Int32Value();
  bool shortterm = AuthRes->Get(String::NewSymbol("is_short_term"))->ToBoolean()->Value();
  std::string password = NS_G::v8str2stdstr(AuthRes->Get(String::NewSymbol("password"))->ToString());
  std::string realm = NS_G::v8str2stdstr(AuthRes->Get(String::NewSymbol("realm"))->ToString());
  std::string nonce = NS_G::v8str2stdstr(AuthRes->Get(String::NewSymbol("nonce"))->ToString());
  AuthResponse pResponse;
  switch(response_code){
    case 0: pResponse.responseType = Allow; break;
    case 1: pResponse.responseType = AllowConditional; break;
    case 2: pResponse.responseType = StaleNonce; break;
    case 3: pResponse.responseType = Reject; break;
    case 4: pResponse.responseType = Unauthorized; break;
  }
  if(shortterm){
    pResponse.authCredMech = AuthCredShortTerm;
  }else{
    pResponse.authCredMech = AuthCredLongTerm;
  }
  if(response_code == 1){
    strcpy (pResponse.szPassword,password.c_str());
  }
  if(!shortterm){
    strcpy (pResponse.szRealm,realm.c_str());
    strcpy (pResponse.szNonce,nonce.c_str());
  }
  Authenticator* obj = ObjectWrap::Unwrap<Authenticator>(args.This());
  obj->_isCompleted = true;
  pthread_cond_signal(&obj->_cond);
  obj->response_ = pResponse;
  return scope.Close(Undefined());
}
