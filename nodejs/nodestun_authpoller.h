#ifndef NODESTUN_AUTH_POLLER
#define NODESTUN_AUTH_POLLER

#include <node.h>
#include "commonincludes.hpp"
#include <pthread.h>
#include "stunauth.h"


class Authenticator : public node::ObjectWrap {
public:
  bool _isCompleted;
  pthread_cond_t _cond;
  pthread_mutex_t _mutex;
  AuthAttributes request_;
  AuthResponse response_;
  HRESULT SendAndWait(
  AuthAttributes* request,
  AuthResponse* response,
  v8::Handle<v8::Object> sender
  );
  static v8::Handle<v8::Value> onNodeAuthCallback(const v8::Arguments& args);
};


#endif
