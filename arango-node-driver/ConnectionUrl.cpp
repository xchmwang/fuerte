#include <iostream>
#include <memory>

#include "ConnectionUrl.h"

namespace arangodb { namespace dbnodejs {

NAN_METHOD(ConnectionUrl::New) {
  if (info.IsConstructCall()) {
      //ConnectionUrl* obj = new ConnectionUrl(Nan::ObjectWrap::Unwrap<Database>(info[0]->ToObject())->cppClass() , *Nan::Utf8String(info[1]));
      ConnectionUrl* obj = new ConnectionUrl();
      obj->Wrap(info.This());
      info.GetReturnValue().Set(info.This());
  } else {
    v8::Local<v8::Function> cons = Nan::New(constructor());
    info.GetReturnValue().Set(Nan::NewInstance(cons).ToLocalChecked());
  }
}

NAN_METHOD(ConnectionUrl::setServerUrl) {
  if (info.Length() != 1 ) {
    Nan::ThrowTypeError("Not 1 Argument");
  }
  std::string tmp_string(*Nan::Utf8String(info[0]),Nan::Utf8String(info[0]).length());
  auto connUrl = Nan::ObjectWrap::Unwrap<ConnectionUrl>(info.Holder()->ToObject())->cppClass();
  connUrl->setServerUrl(tmp_string);
}

NAN_METHOD(ConnectionUrl::setTailUrl) {
  if (info.Length() != 1 ) {
    Nan::ThrowTypeError("Not 1 Argument");
  }
  std::string tmp_string(*Nan::Utf8String(info[0]),Nan::Utf8String(info[0]).length());
  auto connUrl = Nan::ObjectWrap::Unwrap<ConnectionUrl>(info.Holder()->ToObject())->cppClass();
  connUrl->setTailUrl(tmp_string);
}

NAN_METHOD(ConnectionUrl::setDbName) {
  if (info.Length() != 1 ) {
    Nan::ThrowTypeError("Not 1 Argument");
  }
  std::string tmp_string(*Nan::Utf8String(info[0]),Nan::Utf8String(info[0]).length());
  auto connUrl = Nan::ObjectWrap::Unwrap<ConnectionUrl>(info.Holder()->ToObject())->cppClass();
  connUrl->setDbName(tmp_string);
}

}}
