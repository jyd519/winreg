#include <nan.h>

#include "winreg.hpp"

#include <locale>
#include <codecvt>

std::wstring Utf8ToUtf16(const std::string &str);

void ThrowRegError(const winreg::RegException& e);

class RegKey : public Nan::ObjectWrap
{
  public:
    static NAN_MODULE_INIT(Init);

    static NAN_METHOD(New);

    static NAN_METHOD(Create);
    static NAN_METHOD(Open);
    static NAN_METHOD(Close);
    static NAN_METHOD(GetString);
    static NAN_METHOD(GetExpandString);
    static NAN_METHOD(GetMultiString);
    static NAN_METHOD(GetDword);
    static NAN_METHOD(SetString);
    static NAN_METHOD(SetExpandString);
    static NAN_METHOD(SetDword);
    static NAN_METHOD(DeleteValue);
    static NAN_METHOD(DeleteKey);
    static NAN_METHOD(EnumSubKeys);
    static NAN_METHOD(EnumValues);
    static NAN_GETTER(IsValid);

    static Nan::Persistent<v8::FunctionTemplate> constructor;

  private:
    winreg::RegKey _key;

    NAN_METHOD(createKey);
    NAN_METHOD(openKey);
};

Nan::Persistent<v8::FunctionTemplate> RegKey::constructor;

NAN_MODULE_INIT(RegKey::Init)
{
    v8::Local<v8::FunctionTemplate> ctor = Nan::New<v8::FunctionTemplate>(RegKey::New);
    constructor.Reset(ctor);
    ctor->InstanceTemplate()->SetInternalFieldCount(1);
    ctor->SetClassName(Nan::New("RegKey").ToLocalChecked());

    Nan::SetPrototypeMethod(ctor, "open", Open);
    Nan::SetPrototypeMethod(ctor, "create", Create);
    Nan::SetPrototypeMethod(ctor, "close", Close);
    Nan::SetPrototypeMethod(ctor, "getString", GetString);
    Nan::SetPrototypeMethod(ctor, "getExpandString", GetExpandString);
    Nan::SetPrototypeMethod(ctor, "getMultiString", GetMultiString);
    Nan::SetPrototypeMethod(ctor, "getDword", GetDword);
    Nan::SetPrototypeMethod(ctor, "setString", SetString);
    Nan::SetPrototypeMethod(ctor, "setDword", SetDword);
    Nan::SetPrototypeMethod(ctor, "setExpandString", SetExpandString);
    Nan::SetPrototypeMethod(ctor, "deleteValue", DeleteValue);
    Nan::SetPrototypeMethod(ctor, "deleteKey", DeleteKey);
    Nan::SetPrototypeMethod(ctor, "enumSubKeys", EnumSubKeys);
    Nan::SetPrototypeMethod(ctor, "enumValues", EnumValues);

    auto itpl = ctor->InstanceTemplate();
    Nan::SetAccessor(itpl, Nan::New<v8::String>("isValid").ToLocalChecked(), IsValid);

    target->Set(Nan::New("RegKey").ToLocalChecked(), ctor->GetFunction());
}

NAN_METHOD(RegKey::New)
{
    // throw an error if constructor is called without new keyword
    if (!info.IsConstructCall())
    {
        return Nan::ThrowError(Nan::New("RegKey::New - called without new keyword").ToLocalChecked());
    }

    // create a new instance and wrap our javascript instance
    RegKey *k = new RegKey();
    k->Wrap(info.Holder());
  
    k->createKey(info);
}

NAN_METHOD(RegKey::createKey) {
    if (info.Length() == 0) {
        //
    } else if (info.Length() == 2) {
        // expect arguments to be numbers
        if (!info[0]->IsNumber() || !info[1]->IsString())
        {
            return Nan::ThrowError(Nan::New("RegKey::New - expected arguments to be numbers and string").ToLocalChecked());
        }

        HKEY hkey = (HKEY)Nan::To<int32_t>(info[0]).FromJust();
        std::string p = *Nan::Utf8String(info[1]);
        this->_key.Create(hkey, Utf8ToUtf16(p));
    } else if (info.Length() == 3) {
        // expect arguments to be numbers
        if (!info[0]->IsNumber() || !info[1]->IsString() || !info[2]->IsNumber())
        {
            return Nan::ThrowError(Nan::New("RegKey::New - invalid arguments").ToLocalChecked());
        }
        HKEY hkey = (HKEY)Nan::To<int32_t>(info[0]).FromJust();
        std::string p = *Nan::Utf8String(info[1]);
        DWORD access = (DWORD)Nan::To<int32_t>(info[2]).FromJust();
        this->_key.Create(hkey, Utf8ToUtf16(p), access);
    }

    // return the wrapped javascript instance
    info.GetReturnValue().Set(info.Holder());
}


NAN_METHOD(RegKey::openKey) {
    if (info.Length() == 0) {
        //
    } else if (info.Length() == 2) {
        // expect arguments to be numbers
        if (!info[0]->IsNumber() || !info[1]->IsString())
        {
            return Nan::ThrowError(Nan::New("RegKey::New - expected arguments to be numbers and string").ToLocalChecked());
        }

        HKEY hkey = (HKEY)Nan::To<int32_t>(info[0]).FromJust();
        std::string p = *Nan::Utf8String(info[1]);
        this->_key.Open(hkey, Utf8ToUtf16(p));
    } else if (info.Length() == 3) {
        // expect arguments to be numbers
        if (!info[0]->IsNumber() || !info[1]->IsString() || !info[2]->IsNumber())
        {
            return Nan::ThrowError(Nan::New("RegKey::New - invalid arguments").ToLocalChecked());
        }
        HKEY hkey = (HKEY)Nan::To<int32_t>(info[0]).FromJust();
        std::string p = *Nan::Utf8String(info[1]);
        DWORD access = (DWORD)Nan::To<int32_t>(info[2]).FromJust();
        this->_key.Open(hkey, Utf8ToUtf16(p), access);
    }

    // return the wrapped javascript instance
    info.GetReturnValue().Set(info.Holder());
}

NAN_METHOD(RegKey::Create) {
    RegKey *obj = Nan::ObjectWrap::Unwrap<RegKey>(info.Holder());
    obj->createKey(info);
}

NAN_METHOD(RegKey::Open) {
    RegKey *obj = Nan::ObjectWrap::Unwrap<RegKey>(info.Holder());
    obj->openKey(info);
}

NAN_METHOD(RegKey::Close) {
    RegKey *obj = Nan::ObjectWrap::Unwrap<RegKey>(info.Holder());
    obj->_key.Close();
}

NAN_GETTER(RegKey::IsValid) {
    RegKey *obj = Nan::ObjectWrap::Unwrap<RegKey>(info.Holder());
    info.GetReturnValue().Set(obj->_key.IsValid());
}

NAN_METHOD(RegKey::GetString)
{
    RegKey *obj = Nan::ObjectWrap::Unwrap<RegKey>(info.Holder());
    try {
      std::string p = *Nan::Utf8String(info[0]);
      auto v = obj->_key.GetStringValue(Utf8ToUtf16(p));
      std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> convert;
      info.GetReturnValue().Set(Nan::New(convert.to_bytes(v)).ToLocalChecked());
    } catch( const winreg::RegException& e) {
      ThrowRegError(e);
    } catch (const std::exception& e) {
      Nan::ThrowError(e.what());
    }
}

NAN_METHOD(RegKey::GetDword)
{
    RegKey *obj = Nan::ObjectWrap::Unwrap<RegKey>(info.Holder());
    try {
      std::string p = *Nan::Utf8String(info[0]);
      auto v = obj->_key.GetDwordValue(Utf8ToUtf16(p));
      info.GetReturnValue().Set((uint32_t)v);
    } catch( const winreg::RegException& e) {
      ThrowRegError(e);
    } catch (const std::exception& e) {
      Nan::ThrowError(e.what());
    }
}


NAN_METHOD(RegKey::GetExpandString)
{
  auto option = winreg::RegKey::ExpandStringOption::Expand;
  if (info.Length() > 0) {
    auto dontExpand = Nan::To<bool>(info[0]).FromJust();
    if (dontExpand) {
      option = winreg::RegKey::ExpandStringOption::DontExpand;
    }
  }

  RegKey *obj = Nan::ObjectWrap::Unwrap<RegKey>(info.Holder());
  try {
    std::string p = *Nan::Utf8String(info[0]);
    auto v = obj->_key.GetExpandStringValue(Utf8ToUtf16(p), option);
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> convert;
    info.GetReturnValue().Set(Nan::New(convert.to_bytes(v)).ToLocalChecked());
  } catch (const winreg::RegException& e) {
    ThrowRegError(e);
  } catch (const std::exception& e) {
    Nan::ThrowError(e.what());
  }
}


NAN_METHOD(RegKey::GetMultiString)
{
  RegKey *obj = Nan::ObjectWrap::Unwrap<RegKey>(info.Holder());
  try {
    std::string p = *Nan::Utf8String(info[0]);
    auto vec = obj->_key.GetMultiStringValue(Utf8ToUtf16(p));
    auto arr = Nan::New<v8::Array>(vec.size());
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> convert;
    for (size_t i = 0; i < vec.size(); ++i) {
      Nan::Set(arr, i, Nan::New(convert.to_bytes(vec[i])).ToLocalChecked());
    }
    info.GetReturnValue().Set(arr);
  } catch (const winreg::RegException& e) {
    ThrowRegError(e);
  } catch (const std::exception& e) {
    Nan::ThrowError(e.what());
  }
}

NAN_METHOD(RegKey::SetString) {
  RegKey *obj = Nan::ObjectWrap::Unwrap<RegKey>(info.Holder());
  try {
    if (info.Length() != 2 || !info[0]->IsString() || !info[1]->IsString()) {
      return Nan::ThrowError(Nan::New("Invalid argment: two string arguments is required").ToLocalChecked());
    }

    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> convert;

    Nan::Utf8String name(info[0]);
    Nan::Utf8String value(info[1]);
    obj->_key.SetStringValue(convert.from_bytes(*name), convert.from_bytes(*value));

    info.GetReturnValue().Set(obj->handle());
  } catch (const winreg::RegException& e) {
    ThrowRegError(e);
  } catch (const std::exception& e) {
    Nan::ThrowError(e.what());
  }
}

NAN_METHOD(RegKey::SetExpandString) {
  RegKey *obj = Nan::ObjectWrap::Unwrap<RegKey>(info.Holder());
  try {
    if (info.Length() != 2 || !info[0]->IsString() || !info[1]->IsString()) {
      return Nan::ThrowError(Nan::New("Invalid argment: two string arguments is required").ToLocalChecked());
    }

    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> convert;

    Nan::Utf8String name(info[0]);
    Nan::Utf8String value(info[1]);
    obj->_key.SetExpandStringValue(convert.from_bytes(*name), convert.from_bytes(*value));

    info.GetReturnValue().Set(obj->handle());
  } catch (const winreg::RegException& e) {
    ThrowRegError(e);
  } catch (const std::exception& e) {
    Nan::ThrowError(e.what());
  }
}

NAN_METHOD(RegKey::SetDword) {
  RegKey *obj = Nan::ObjectWrap::Unwrap<RegKey>(info.Holder());
  try {
    if (info.Length() != 2 || !info[0]->IsString() || !info[1]->IsInt32()) {
      return Nan::ThrowError(Nan::New("Invalid argment: name(string), value(number) required").ToLocalChecked());
    }

    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> convert;

    Nan::Utf8String name(info[0]);
    obj->_key.SetDwordValue(convert.from_bytes(*name), Nan::To<uint32_t>(info[1]).FromJust());

    info.GetReturnValue().Set(obj->handle());
  } catch (const winreg::RegException& e) {
    ThrowRegError(e);
  } catch (const std::exception& e) {
    Nan::ThrowError(e.what());
  }
}
NAN_METHOD(RegKey::DeleteValue) {
  RegKey *obj = Nan::ObjectWrap::Unwrap<RegKey>(info.Holder());
  try {
    if (info.Length() != 1 || !info[0]->IsString()) {
      return Nan::ThrowError(Nan::New("Invalid argment: a string argument is required").ToLocalChecked());
    }

    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> convert;

    Nan::Utf8String s(info[0]);
    obj->_key.DeleteValue(convert.from_bytes(*s));

    info.GetReturnValue().Set(obj->handle());
  } catch (const winreg::RegException& e) {
    ThrowRegError(e);
  } catch (const std::exception& e) {
    Nan::ThrowError(e.what());
  }
}

NAN_METHOD(RegKey::DeleteKey) {
  RegKey *obj = Nan::ObjectWrap::Unwrap<RegKey>(info.Holder());
  try {
    if (info.Length() != 2 || !info[0]->IsString() || !info[1]->IsInt32()) {
      return Nan::ThrowError(Nan::New("Invalid argment: name, false").ToLocalChecked());
    }

    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> convert;

    Nan::Utf8String s(info[0]);
    obj->_key.DeleteKey(convert.from_bytes(*s), Nan::To<uint32_t>(info[1]).FromJust());

    info.GetReturnValue().Set(obj->handle());
  } catch (const winreg::RegException& e) {
    ThrowRegError(e);
  } catch (const std::exception& e) {
    Nan::ThrowError(e.what());
  }
}

NAN_METHOD(RegKey::EnumSubKeys) {
  RegKey *obj = Nan::ObjectWrap::Unwrap<RegKey>(info.Holder());
  try {
    auto v = obj->_key.EnumSubKeys();
    auto arr = Nan::New<v8::Array>(v.size());
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> convert;
    for (size_t i = 0; i < v.size(); ++i) {
      Nan::Set(arr, i, Nan::New(convert.to_bytes(v[i])).ToLocalChecked());
    }
    info.GetReturnValue().Set(arr);
  } catch (const winreg::RegException& e) {
    ThrowRegError(e);
  } catch (const std::exception& e) {
    Nan::ThrowError(e.what());
  }
}

NAN_METHOD(RegKey::EnumValues) {
  RegKey *obj = Nan::ObjectWrap::Unwrap<RegKey>(info.Holder());
  try {
    auto v = obj->_key.EnumValues();
    auto arr = Nan::New<v8::Array>(v.size());
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> convert;
    for (size_t i = 0; i < v.size(); ++i) {
      auto obj = Nan::New<v8::Object>();
      obj->Set(Nan::New("name").ToLocalChecked(), Nan::New(convert.to_bytes(v[i].first)).ToLocalChecked());
      obj->Set(Nan::New("type").ToLocalChecked(), Nan::New((uint32_t)v[i].second));
      Nan::Set(arr, i, obj);
    }
    info.GetReturnValue().Set(arr);
  } catch (const winreg::RegException& e) {
    ThrowRegError(e);
  } catch (const std::exception& e) {
    Nan::ThrowError(e.what());
  }
}

void ThrowRegError(const winreg::RegException& e) {
  auto err = Nan::Error(e.what());
  auto obj = err->ToObject();
  obj->Set(Nan::New("name").ToLocalChecked(), Nan::New("RegError").ToLocalChecked());
  obj->Set(Nan::New("code").ToLocalChecked(), Nan::New(e.ErrorCode()));
  Nan::ThrowError(err);
}

std::wstring Utf8ToUtf16(const std::string &str)
{
  std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> convert;
  return convert.from_bytes(str);
}

NAN_MODULE_INIT(InitModule)
{
    RegKey::Init(target);
    target->Set(Nan::New("HKEY_CLASSES_ROOT").ToLocalChecked(), Nan::New((uint32_t)(ULONG_PTR)HKEY_CLASSES_ROOT));
    target->Set(Nan::New("HKEY_LOCAL_MACHINE").ToLocalChecked(), Nan::New((uint32_t)(ULONG_PTR)HKEY_LOCAL_MACHINE));
    target->Set(Nan::New("HKEY_CURRENT_USER").ToLocalChecked(), Nan::New((uint32_t)(ULONG_PTR)HKEY_CURRENT_USER));

    target->Set(Nan::New("KEY_WOW64_32KEY").ToLocalChecked(), Nan::New(KEY_WOW64_32KEY));
    target->Set(Nan::New("KEY_WOW64_64KEY").ToLocalChecked(), Nan::New(KEY_WOW64_64KEY));

    target->Set(Nan::New("KEY_QUERY_VALUE").ToLocalChecked(), Nan::New(KEY_QUERY_VALUE));
    target->Set(Nan::New("KEY_READ").ToLocalChecked(), Nan::New(KEY_READ));
    target->Set(Nan::New("KEY_SET_VALUE").ToLocalChecked(), Nan::New(KEY_SET_VALUE));
    target->Set(Nan::New("KEY_WRITE").ToLocalChecked(), Nan::New(KEY_WRITE));
}

NODE_MODULE(winreg, InitModule);