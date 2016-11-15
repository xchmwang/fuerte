#ifndef FUERTE_NODE_CONNECTION_URL_H
#define FUERTE_NODE_CONNECTION_URL_H

#include <fuerte/Server.h>
#include <fuerte/ConnectionUrl.h>
#include <nan.h>

namespace arangodb { namespace dbnodejs {

class ConnectionUrl : public Nan::ObjectWrap {
 public:
  static NAN_MODULE_INIT(Init) {
    v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
    tpl->SetClassName(Nan::New("ConnectionUrl").ToLocalChecked());
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    Nan::SetPrototypeMethod(tpl, "setServerUrl", ConnectionUrl::setServerUrl);
    Nan::SetPrototypeMethod(tpl, "setDbName", ConnectionUrl::setDbName);
    Nan::SetPrototypeMethod(tpl, "setTailUrl", ConnectionUrl::setTailUrl);

    constructor().Reset(tpl->GetFunction());
    target->Set( Nan::New("ConnectionUrl").ToLocalChecked() , tpl->GetFunction()); //put in module init?!
  }

  static NAN_METHOD(New);
  static NAN_METHOD(setServerUrl);
  static NAN_METHOD(setDbName);
  static NAN_METHOD(setTailUrl);

  arangodb::dbinterface::ConnectionUrl* cppClass() {
    return &_cpp;
  }

 private:
   arangodb::dbinterface::ConnectionUrl _cpp;
   ConnectionUrl()
     : _cpp()
     {}

 static Nan::Persistent<v8::Function>& constructor() {
   static Nan::Persistent<v8::Function> ctor;
   return ctor;
 }

};

}}
#endif

