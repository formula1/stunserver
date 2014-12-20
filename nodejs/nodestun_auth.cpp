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

#include "commonincludes.hpp"
#include <node.h>
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
  Authenticator auth = new Authenticator(pAuthAttributes,pResponse,stunserver);
  Handle<Object> AuthReq = Object::New();
  AuthReq->Set(String::NewSymbol("user"), String::New(request_.szUser));
  AuthReq->Set(String::NewSymbol("realm"), String::New(request_.szRealm));
  AuthReq->Set(String::NewSymbol("nonce"), String::New(request_.szNonce));
  AuthReq->Set(String::NewSymbol("password"), String::New(request_.szLegacyPassword));
  AuthReq->Set(String::NewSymbol("integrity"), Boolean::New(request_.fMessageIntegrityPresent));
  // attributes in the request

  Handle<Object> AuthRes = Object::New();
  AuthRes->Set(String::NewSymbol("response_code"), Number::New(0));
  AuthRes->Set(String::NewSymbol("is_short_term"), Boolean::New(true));
  AuthRes->Set(String::NewSymbol("password"), String::New("password"));
  AuthRes->Set(String::NewSymbol("realm"), String::New("realm"));
  AuthRes->Set(String::NewSymbol("nonce"), String::New("private nonce"));

  Handle<Object> Callback = FunctionTemplate::New(onNodeAuthCallback);
  auth->Wrap(Callback);

  Handle<Value> args[] = [AuthReq,AuthRes,Callback];
  auth.DoAuthCheck();
  server_->Get(String::NewSymbol("doAuth"))->ToFunction()->Call(server_,3,args);

}

class Authenticator : public ObjectWrap
{
  bool _isCompleted;
  pthread_cond _cond;
  pthread_mutex _mutex;
  AuthAttributes request_;
  AuthResponse response_;

  HRESULT DoAuthCheck(AuthAttributes pAuthAttributes, AuthResponse* pResponse)
  {
    _isCompleted = false;

    pthread_mutex_lock(&_mutex);
    while (_isCompleted == false)
    {
      pthread_cond_wait(&_cond, &_mutex);
    }
    pthread_mutex_unlock(&_mutex);
    return S_OK;
    
  }

  // NOT SHOWN - that other NodeJS object mentioned above, upon getting the callback that the async auth request has completed, call this methods
  void onNodeAuthCallback(const Arguments& args)
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
    int response_code = AuthRes->Get(String::NewSymbol("response_code"))->ToNumber()->Int32Value();
    bool shortterm = AuthRes->Get(String::NewSymbol("is_short_term"))->ToBoolean()->Value();
    char* password = AuthRes->Get(String::NewSymbol("password"))->ToString()->UTF8Value();
    char* realm = AuthRes->Get(String::NewSymbol("realm"))->ToString()->UTF8Value();
    char* nonce = AuthRes->Get(String::NewSymbol("nonce"))->ToString()->UTF8Value();
    AuthResponse pResponse;
    switch(response_code){
      case 0: pResponse.responseType = AuthCredentialMechanism.Allow; break;
      case 1: pResponse.responseType = AuthCredentialMechanism.AllowConditional; break;
      case 2: pResponse.responseType = AuthCredentialMechanism.StaleNonce; break;
      case 3: pResponse.responseType = AuthCredentialMechanism.Reject; break;
      case 4: pResponse.responseType = AuthCredentialMechanism.Unauthorized; break;
    }
    if(shortterm){
      pResponse.authCredMech = AuthCredentialMechanism.AuthCredShortTerm;
    }else{
      pResponse.authCredMech = AuthCredentialMechanism.AuthCredLongTerm;
    }
    if(response_code == 1){
      pResponse.szPassword = password;
    }
    if(!shortterm){
      pResponse.szRealm = realm;
      pResponse.szNonce = nonce;
    }
    Authenticator* obj = ObjectWrap::Unwrap<Authenticator>(args.This());
    memcpy(pResponse, obj->response_&, sizeof(AuthResponse));
    obj->_isCompleted = true;
    pthread_cond_signal(obj->_cond&);
  }
};
