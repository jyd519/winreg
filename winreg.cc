#include <napi.h>
#include <uv.h>

#include "winreg.hpp"

#include <codecvt>
#include <locale>
#include <algorithm>

std::wstring Utf8ToUtf16(const std::string& str);
std::string Utf16ToUtf8(const std::wstring &wstr);

#define ThrowRegError(e) MakeRegError(env, e).ThrowAsJavaScriptException()

Napi::Error MakeRegError(Napi::Env env, const winreg::RegException& e);

class RegKey : public Napi::ObjectWrap<RegKey> {
 public:
  static Napi::Object Init(Napi::Env env, Napi::Object exports);

  static Napi::Value CreateKey(const Napi::CallbackInfo& info);
  static Napi::Value OpenKey(const Napi::CallbackInfo& info);

  RegKey(const Napi::CallbackInfo& info);

  Napi::Value Create(const Napi::CallbackInfo& info);
  Napi::Value Open(const Napi::CallbackInfo& info);
  Napi::Value Close(const Napi::CallbackInfo& info);
  Napi::Value GetHandle(const Napi::CallbackInfo& info);
  Napi::Value GetValueType(const Napi::CallbackInfo& info);
  Napi::Value GetString(const Napi::CallbackInfo& info);
  Napi::Value GetExpandString(const Napi::CallbackInfo& info);
  Napi::Value GetMultiString(const Napi::CallbackInfo& info);
  Napi::Value GetDword(const Napi::CallbackInfo& info);
  Napi::Value SetString(const Napi::CallbackInfo& info);
  Napi::Value SetExpandString(const Napi::CallbackInfo& info);
  Napi::Value SetDword(const Napi::CallbackInfo& info);
  Napi::Value DeleteValue(const Napi::CallbackInfo& info);
  Napi::Value DeleteKey(const Napi::CallbackInfo& info);
  Napi::Value EnumSubKeys(const Napi::CallbackInfo& info);
  Napi::Value EnumValues(const Napi::CallbackInfo& info);
  Napi::Value IsValid(const Napi::CallbackInfo& info);

  void Finalize(Napi::Env env) {
    _key.Close();
  }
 private:
  winreg::RegKey _key;
  Napi::Value createKey(const Napi::CallbackInfo& info);
  Napi::Value openKey(const Napi::CallbackInfo& info);
};

Napi::Object RegKey::Init(Napi::Env env, Napi::Object exports) {
  // This method is used to hook the accessor and method callbacks
  Napi::Function func =
      DefineClass(env, "RegKey",
                  {InstanceMethod("open", &RegKey::Open),
                   InstanceMethod("create", &RegKey::Create),
                   InstanceMethod("close", &RegKey::Close),
                   InstanceMethod("getValueType", &RegKey::GetValueType),
                   InstanceMethod("getString", &RegKey::GetString),
                   InstanceMethod("handle", &RegKey::GetHandle),
                   InstanceMethod("getExpandString", &RegKey::GetExpandString),
                   InstanceMethod("getMultiString", &RegKey::GetMultiString),
                   InstanceMethod("getDword", &RegKey::GetDword),
                   InstanceMethod("setString", &RegKey::SetString),
                   InstanceMethod("setDword", &RegKey::SetDword),
                   InstanceMethod("setExpandString", &RegKey::SetExpandString),
                   InstanceMethod("deleteValue", &RegKey::DeleteValue),
                   InstanceMethod("deleteKey", &RegKey::DeleteKey),
                   InstanceMethod("enumSubKeys", &RegKey::EnumSubKeys),
                   InstanceMethod("enumValues", &RegKey::EnumValues),
                   InstanceAccessor("isValid", &RegKey::IsValid, nullptr)});

  Napi::FunctionReference* constructor = new Napi::FunctionReference();
  *constructor = Napi::Persistent(func);
  env.SetInstanceData(constructor);

  exports.Set("RegKey", func);
  return exports;
}

void toWindowSlashStyle(std::string& path) {
  std::transform(path.cbegin(), path.cend(), path.begin(), [](char c) {
    return c == '/' ? '\\' : c;
  });
}

Napi::Value RegKey::CreateKey(const Napi::CallbackInfo& info) {
  auto env = info.Env();
  Napi::EscapableHandleScope scope(env);
  Napi::Object obj = env.GetInstanceData<Napi::FunctionReference>()->New({});
  auto pRegKey = Unwrap(obj);
  if (info.Length() > 0) {
    try {
      pRegKey->createKey(info);
    } catch (const winreg::RegException& e) {
      if (e.ErrorCode() == ERROR_FILE_NOT_FOUND) {
        return env.Null();
      }
      ThrowRegError(e);                                            
      return env.Null(); 
    }
  }
  return scope.Escape(napi_value(obj)).ToObject();
}

Napi::Value RegKey::OpenKey(const Napi::CallbackInfo& info) {
  auto env = info.Env();
  Napi::EscapableHandleScope scope(env);
  Napi::Object obj = env.GetInstanceData<Napi::FunctionReference>()->New({});
  auto pRegKey = Unwrap(obj);
  if (info.Length() > 0) {
    pRegKey->openKey(info);
  }
  return scope.Escape(napi_value(obj)).ToObject();
}

// hkey, path, value, options
Napi::Value RegQuery(const Napi::CallbackInfo& info) {
  if (info.Length() < 3) {
    Napi::Error::New(
        info.Env(), Napi::String::New(info.Env(), "invalid arguments (hkey, path, value, options?)"))
        .ThrowAsJavaScriptException();
    return info.Env().Undefined();
  }

  auto env = info.Env();
  HKEY hkey = (HKEY)info[0].As<Napi::Number>().Int64Value();
  std::string p = info[1].As<Napi::String>();
  std::string v = info[2].As<Napi::String>();
  DWORD options = 0;
  if (info.Length() > 3) {
    options = (DWORD)info[3].As<Napi::Number>().Uint32Value();
  }

  toWindowSlashStyle(p);

  winreg::RegKey key;
  try {
    key.Open(hkey, Utf8ToUtf16(p), KEY_READ | options);
    auto type = key.QueryValueType(Utf8ToUtf16(v));
    if (type == REG_DWORD) {
      return Napi::Number::New(env, key.GetDwordValue(Utf8ToUtf16(v)));
    } else if (type == REG_SZ || type == REG_EXPAND_SZ) {
      return Napi::String::New(env, Utf16ToUtf8(key.GetStringValue(Utf8ToUtf16(v))));
    } else {
      return env.Null();
    }
  } catch (const winreg::RegException& e) {
    if (e.ErrorCode() == ERROR_FILE_NOT_FOUND) {
      return env.Null();
    }
    ThrowRegError(e);                                            
    return env.Null(); 
  } catch (const std::exception& e) {
    Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
    return env.Null();
  }
}

Napi::Value RegSet(const Napi::CallbackInfo& info) {
  auto env = info.Env();
  if (info.Length() < 4) {
    Napi::Error::New(
        env, Napi::String::New(env, "invalid arguments (hkey, path, valueName, value, options?)"))
      .ThrowAsJavaScriptException();
    return env.Undefined();
  }

  HKEY hkey = (HKEY)info[0].As<Napi::Number>().Int64Value();
  std::string p = info[1].As<Napi::String>();
  std::string valName = info[2].As<Napi::String>();
  Napi::Value value = info[3];
  DWORD options = 0;
  if (info.Length() > 4) {
    options = (DWORD)info[4].As<Napi::Number>().Uint32Value();
  }
  toWindowSlashStyle(p);
  try {
    winreg::RegKey key;
    key.Create(hkey, Utf8ToUtf16(p), KEY_WRITE | options);
    if (value.IsString()) {
      key.SetStringValue(Utf8ToUtf16(valName), Utf8ToUtf16(value.ToString()));
    }  else if (value.IsNumber()) {
      key.SetDwordValue(Utf8ToUtf16(valName), value.ToNumber().Uint32Value());
    }
    return Napi::Boolean::New(env, true);
  } catch (const winreg::RegException& e) {
    ThrowRegError(e);                                            
    return env.Null(); 
  } catch (const std::exception& e) {
    Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
    return env.Null();
  }
}

Napi::Value RegDelete(const Napi::CallbackInfo& info) {
  auto env = info.Env();
  if (info.Length() < 2) {
    Napi::Error::New(
        env, Napi::String::New(env, "invalid arguments (hkey, path, value?, options?)"))
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }

  HKEY hkey = (HKEY)info[0].As<Napi::Number>().Int64Value();
  std::string p = info[1].As<Napi::String>();
  Napi::Value valueName;
  DWORD options = 0;
  if (info.Length() > 2) {
    valueName  = info[2];
  }
  if (info.Length() > 3) {
    options = (DWORD)info[3].As<Napi::Number>().Uint32Value();
  }
  toWindowSlashStyle(p);
  winreg::RegKey key;
  try {
    if (valueName.IsNull() || valueName.IsUndefined()) {
      key.Open(hkey, L"", DELETE | KEY_ENUMERATE_SUB_KEYS | KEY_QUERY_VALUE | options);
      auto status = RegDeleteTree(key.Get(), Utf8ToUtf16(p).c_str());
      if (!(status == ERROR_SUCCESS || status == ERROR_FILE_NOT_FOUND)) {
          throw winreg::RegException{"RegDeleteTree failed.", status};
      }
      return Napi::Boolean::New(env, true);
    } else {
      std::string v = valueName.As<Napi::String>();
      key.Open(hkey, Utf8ToUtf16(p), KEY_SET_VALUE | options);
      auto status = RegDeleteValue(key.Get(), Utf8ToUtf16(v).c_str());
      if (!(status == ERROR_SUCCESS || status == ERROR_FILE_NOT_FOUND)) {
          throw winreg::RegException{"RegDeleteValue failed.", status};
      }
      return Napi::Boolean::New(env, true);
    }
  } catch (const winreg::RegException& e) {
    if (e.ErrorCode() == ERROR_FILE_NOT_FOUND) {
      return Napi::Boolean::New(env, true);
    }
    ThrowRegError(e);                                            
    return env.Null(); 
  } catch (const std::exception& e) {
    Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
    return env.Null();
  }
}

RegKey::RegKey(const Napi::CallbackInfo& info)
    : Napi::ObjectWrap<RegKey>(info) {
  auto env = info.Env();
  // throw an error if constructor is called without new keyword
  if (!info.IsConstructCall()) {
    Napi::Error::New(
        env, Napi::String::New(env, "RegKey::New - called without new keyword"))
        .ThrowAsJavaScriptException();
    return;
  }

  if (info.Length() > 0) {
    this->createKey(info);
  }
}

Napi::Value RegKey::createKey(const Napi::CallbackInfo& info) {
  if (info.Length() == 2) {
    if (!info[0].IsNumber() || !info[1].IsString()) {
      Napi::Error::New(
          info.Env(),
          Napi::String::New(
              info.Env(),
              "createKey - expected arguments to be numbers and string"))
          .ThrowAsJavaScriptException();
      return info.Env().Null();
    }

    HKEY hkey = (HKEY)info[0].As<Napi::Number>().Int64Value();
    std::string p = info[1].As<Napi::String>();
    toWindowSlashStyle(p);
    this->_key.Create(hkey, Utf8ToUtf16(p));
    return info.This();
  } else if (info.Length() == 3) {
    if (!info[0].IsNumber() || !info[1].IsString() || !info[2].IsNumber()) {
      Napi::Error::New(
          info.Env(),
          Napi::String::New(info.Env(), "createKey - invalid arguments"))
          .ThrowAsJavaScriptException();
      return info.Env().Null();
    }
    HKEY hkey = (HKEY)info[0].As<Napi::Number>().Int64Value();
    std::string p = info[1].As<Napi::String>();
    toWindowSlashStyle(p);
    DWORD access = (DWORD)info[2].As<Napi::Number>().Uint32Value();
    this->_key.Create(hkey, Utf8ToUtf16(p), access);
  } else if (info.Length() == 4) {
    if (!info[0].IsNumber() || !info[1].IsString() || !info[2].IsNumber() ||
        !info[3].IsNumber()) {
      Napi::Error::New(
          info.Env(),
          Napi::String::New(info.Env(), "createkey - invalid arguments"))
          .ThrowAsJavaScriptException();
      return info.Env().Null();
    }
    HKEY hkey = (HKEY)info[0].As<Napi::Number>().Int64Value();
    std::string p = info[1].As<Napi::String>();
    toWindowSlashStyle(p);
    DWORD access = (DWORD)info[2].As<Napi::Number>().Uint32Value();
    DWORD options = (DWORD)info[3].As<Napi::Number>().Uint32Value();
    this->_key.Create(hkey, Utf8ToUtf16(p), access, options, nullptr, nullptr);
  } else {
    Napi::Error::New(
        info.Env(),
        Napi::String::New(info.Env(), "createKey - invalid arguments"))
        .ThrowAsJavaScriptException();
    return info.Env().Null();
  }

  // return the wrapped javascript instance
  return info.This();
}

Napi::Value RegKey::openKey(const Napi::CallbackInfo& info) {
  auto env = info.Env();
  if (info.Length() == 2) {
    if (!info[0].IsNumber() || !info[1].IsString()) {
      Napi::Error::New(
          env,
          Napi::String::New(
              env, "openKey - expected arguments to be numbers and string"))
          .ThrowAsJavaScriptException();
      return env.Null();
    }

    HKEY hkey = (HKEY)info[0].As<Napi::Number>().Int64Value();
    std::string p = info[1].As<Napi::String>();
    toWindowSlashStyle(p);
    this->_key.Open(hkey, Utf8ToUtf16(p));
  } else if (info.Length() == 3) {
    if (!info[0].IsNumber() || !info[1].IsString() || !info[2].IsNumber()) {
      Napi::Error::New(env,
                       Napi::String::New(env, "openKey - invalid arguments"))
          .ThrowAsJavaScriptException();
      return env.Null();
    }
    HKEY hkey = (HKEY)info[0].As<Napi::Number>().Int64Value();
    std::string p = info[1].As<Napi::String>();
    toWindowSlashStyle(p);
    DWORD access = (DWORD)info[2].As<Napi::Number>().Uint32Value();
    this->_key.Open(hkey, Utf8ToUtf16(p), access);
  } else {
    Napi::Error::New(env, Napi::String::New(env, "openKey - invalid arguments"))
        .ThrowAsJavaScriptException();
    return env.Null();
  }

  return info.This();
}

Napi::Value RegKey::Create(const Napi::CallbackInfo& info) {
  auto env = info.Env();
  try {
    return this->createKey(info);
  }
  catch (const winreg::RegException& e) {
    if (e.ErrorCode() == ERROR_FILE_NOT_FOUND) {
      return Napi::Boolean::New(env, false);
    }
    ThrowRegError(e);
    return env.Undefined();
  }
}

Napi::Value RegKey::Open(const Napi::CallbackInfo& info) {
  auto env = info.Env();
  try {
    return this->openKey(info);
  } catch (const winreg::RegException& e) {
    if (e.ErrorCode() == ERROR_FILE_NOT_FOUND) {
      return Napi::Boolean::New(env, false);
    }
    ThrowRegError(e);
    return env.Null();
  } catch (const std::exception& e) {
    Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
    return env.Null();
  }
}

Napi::Value RegKey::Close(const Napi::CallbackInfo& info) {
  auto env = info.Env();
  try {
    this->_key.Close();
    return info.This();
  } catch (const winreg::RegException& e) {
    ThrowRegError(e);
    return env.Null();
  } catch (const std::exception& e) {
    Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
  }
}

Napi::Value RegKey::IsValid(const Napi::CallbackInfo& info) {
  auto env = info.Env();
  return Napi::Boolean::New(env, this->_key.IsValid());
}

Napi::Value RegKey::GetHandle(const Napi::CallbackInfo& info) {
  auto env = info.Env();
  return Napi::Number::New(env, (int64_t)this->_key.Get());
}

Napi::Value RegKey::GetValueType(const Napi::CallbackInfo& info) {
  auto env = info.Env();
  try {
    std::string p = info[0].As<Napi::String>();
    auto dwType = this->_key.QueryValueType(Utf8ToUtf16(p));
    return Napi::Number::New(env, dwType);
  } catch (const winreg::RegException& e) {
    ThrowRegError(e);
    return env.Null();
  } catch (const std::exception& e) {
    Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
    return env.Null();
  }
}

Napi::Value RegKey::GetString(const Napi::CallbackInfo& info) {
  auto env = info.Env();
  try {
    std::string p = info[0].As<Napi::String>();
    auto v = this->_key.GetStringValue(Utf8ToUtf16(p));
    return Napi::String::New(env, Utf16ToUtf8(v));
  } catch (const winreg::RegException& e) {
    if (e.ErrorCode() == ERROR_FILE_NOT_FOUND && info.Length() > 1) {
      return info[1];
    }
    ThrowRegError(e);
    return env.Null();
  } catch (const std::exception& e) {
    Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
    return env.Null();
  }
}

Napi::Value RegKey::GetDword(const Napi::CallbackInfo& info) {
  auto env = info.Env();
  try {
    std::string p = info[0].As<Napi::String>();
    auto v = this->_key.GetDwordValue(Utf8ToUtf16(p));
    return Napi::Number::New(env, (uint32_t)v);
  } catch (const winreg::RegException& e) {
    if (e.ErrorCode() == ERROR_FILE_NOT_FOUND && info.Length() > 1) {
      return info[1];
    }

    ThrowRegError(e);
    return env.Null();
  } catch (const std::exception& e) {
    Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
  }
}

Napi::Value RegKey::GetExpandString(const Napi::CallbackInfo& info) {
  auto option = winreg::RegKey::ExpandStringOption::DontExpand;
  int defval = -1;
  if (info.Length() > 1) {
    if (info[1].IsBoolean()) {
      auto expand = info[1].As<Napi::Boolean>().Value();
      option = expand ? winreg::RegKey::ExpandStringOption::Expand
                      : winreg::RegKey::ExpandStringOption::DontExpand;
      if (info.Length() > 2 && info[2].IsString()) {
        defval = 2;
      }
    } else {
      if (info[1].IsString()) {
        defval = 1;
      }
    }
  }

  auto env = info.Env();
  try {
    std::string p = info[0].As<Napi::String>();
    auto v = this->_key.GetExpandStringValue(Utf8ToUtf16(p), option);
    return Napi::String::New(env, Utf16ToUtf8(v));
  } catch (const winreg::RegException& e) {
    if (e.ErrorCode() == ERROR_FILE_NOT_FOUND && defval > 0) {
      return info[defval];
    }
    ThrowRegError(e);
    return env.Null();
  } catch (const std::exception& e) {
    Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
    return env.Null();
  }
}

Napi::Value RegKey::GetMultiString(const Napi::CallbackInfo& info) {
  auto env = info.Env();
  try {
    std::string p = info[0].As<Napi::String>();
    auto vec = this->_key.GetMultiStringValue(Utf8ToUtf16(p));
    auto arr = Napi::Array::New(env, vec.size());
    for (size_t i = 0; i < vec.size(); ++i) {
      (arr).Set(i, Napi::String::New(env, Utf16ToUtf8(vec[i])));
    }
    return arr;
  } catch (const winreg::RegException& e) {
    if (e.ErrorCode() == ERROR_FILE_NOT_FOUND && info.Length() > 1) {
      return info[1];
    }

    ThrowRegError(e);
    return env.Null();
  } catch (const std::exception& e) {
    Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
    return env.Null();
  }
}

Napi::Value RegKey::SetString(const Napi::CallbackInfo& info) {
  auto env = info.Env();
  try {
    if (info.Length() != 2 || !info[0].IsString() || !info[1].IsString()) {
      Napi::Error::New(
          env, Napi::String::New(
                   env, "Invalid argment: two string arguments is required"))
          .ThrowAsJavaScriptException();
      return env.Null();
    }

    std::string name = info[0].As<Napi::String>();
    std::string value = info[1].As<Napi::String>();
    this->_key.SetStringValue(Utf8ToUtf16(name), Utf8ToUtf16(value));
    return Napi::Number::New(env, 0);
  } catch (const winreg::RegException& e) {
    ThrowRegError(e);
    return env.Null();
  } catch (const std::exception& e) {
    Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
    return env.Null();
  }
}

Napi::Value RegKey::SetExpandString(const Napi::CallbackInfo& info) {
  auto env = info.Env();
  try {
    if (info.Length() != 2 || !info[0].IsString() || !info[1].IsString()) {
      Napi::Error::New(
          env, Napi::String::New(
                   env, "Invalid argment: two string arguments is required"))
          .ThrowAsJavaScriptException();
      return env.Null();
    }

    auto name = info[0].As<Napi::String>();
    auto value = info[1].As<Napi::String>();
    this->_key.SetExpandStringValue(Utf8ToUtf16(name), Utf8ToUtf16(value));
    return info.This();
  } catch (const winreg::RegException& e) {
    ThrowRegError(e);
    return env.Null();
  } catch (const std::exception& e) {
    Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
    return env.Null();
  }
}

Napi::Value RegKey::SetDword(const Napi::CallbackInfo& info) {
  auto env = info.Env();
  try {
    if (info.Length() != 2 || !info[0].IsString() || !info[1].IsNumber()) {
      Napi::Error::New(
          env,
          Napi::String::New(
              env, "Invalid argment: name(string), value(number) required"))
          .ThrowAsJavaScriptException();
      return env.Null();
    }
    std::string name = info[0].As<Napi::String>();
    this->_key.SetDwordValue(Utf8ToUtf16(name),
                             info[1].As<Napi::Number>().Uint32Value());
    return info.This();
  } catch (const winreg::RegException& e) {
    ThrowRegError(e);
    return env.Null();
  } catch (const std::exception& e) {
    Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
    return env.Null();
  }
}
Napi::Value RegKey::DeleteValue(const Napi::CallbackInfo& info) {
  auto env = info.Env();
  try {
    if (info.Length() != 1 || !info[0].IsString()) {
      Napi::Error::New(
          env, Napi::String::New(
                   env, "Invalid argment: a string argument is required"))
          .ThrowAsJavaScriptException();
      return env.Null();
    }

    auto s = info[0].As<Napi::String>();
    this->_key.DeleteValue(Utf8ToUtf16(s));
    return info.This();
  } catch (const winreg::RegException& e) {
    ThrowRegError(e);
    return env.Null();
  } catch (const std::exception& e) {
    Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
    return env.Null();
  }
}

Napi::Value RegKey::DeleteKey(const Napi::CallbackInfo& info) {
  auto env = info.Env();
  try {
    if (info.Length() != 2 || !info[0].IsString() || !info[1].IsNumber()) {
      Napi::Error::New(env,
                       Napi::String::New(env, "Invalid argment: name, false"))
          .ThrowAsJavaScriptException();
      return env.Null();
    }

    std::string s = info[0].As<Napi::String>();
    this->_key.DeleteKey(Utf8ToUtf16(s),
                         info[1].As<Napi::Number>().Uint32Value());
    return info.This();
  } catch (const winreg::RegException& e) {
    ThrowRegError(e);
    return env.Null();
  } catch (const std::exception& e) {
    Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
    return env.Null();
  }
}

Napi::Value RegKey::EnumSubKeys(const Napi::CallbackInfo& info) {
  auto env = info.Env();
  try {
    auto v = this->_key.EnumSubKeys();
    auto arr = Napi::Array::New(env, v.size());
    for (size_t i = 0; i < v.size(); ++i) {
      arr.Set(i, Napi::String::New(env, Utf16ToUtf8(v[i])));
    }
    return arr;
  } catch (const winreg::RegException& e) {
    ThrowRegError(e);
    return env.Null();
  } catch (const std::exception& e) {
    Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
    return env.Null();
  }
}

Napi::Value RegKey::EnumValues(const Napi::CallbackInfo& info) {
  auto env = info.Env();
  try {
    auto v = this->_key.EnumValues();
    auto obj = Napi::Object::New(env);
    for (size_t i = 0; i < v.size(); ++i) {
      obj.Set(Napi::String::New(env, Utf16ToUtf8(v[i].first)),
              Napi::Number::New(env, (uint32_t)v[i].second));
    }
    return obj;
  } catch (const winreg::RegException& e) {
    ThrowRegError(e);
    return env.Null();
  } catch (const std::exception& e) {
    Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
    return env.Null();
  }
}

Napi::Error MakeRegError(Napi::Env env, const winreg::RegException& e) {
  auto err = Napi::Error::New(env, e.what());
  err.Set("name", "RegError");
  err.Set("code", Napi::Number::New(env, e.ErrorCode()));
  return err;
}

std::wstring Utf8ToUtf16(const std::string &str) {
    if( str.empty() ) return std::wstring();
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring wstrTo( size_needed, 0 );
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
    return wstrTo;
}

std::string Utf16ToUtf8(const std::wstring &wstr) {
    if( wstr.empty() ) return std::string();
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string strTo( size_needed, 0 );
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
    return strTo;
}

Napi::Object InitModule(Napi::Env env, Napi::Object exports) {
  RegKey::Init(env, exports);
  exports.Set("HKEY_CLASSES_ROOT",
              Napi::Number::New(env, (uint32_t)(ULONG_PTR)HKEY_CLASSES_ROOT));
  exports.Set("HKEY_LOCAL_MACHINE",
              Napi::Number::New(env, (uint32_t)(ULONG_PTR)HKEY_LOCAL_MACHINE));
  exports.Set("HKEY_CURRENT_USER",
              Napi::Number::New(env, (uint32_t)(ULONG_PTR)HKEY_CURRENT_USER));
  exports.Set("HKEY_USERS",
              Napi::Number::New(env, (uint32_t)(ULONG_PTR)HKEY_USERS));

  exports.Set("KEY_WOW64_32KEY", Napi::Number::New(env, KEY_WOW64_32KEY));
  exports.Set("KEY_WOW64_64KEY", Napi::Number::New(env, KEY_WOW64_64KEY));

  exports.Set("KEY_QUERY_VALUE", Napi::Number::New(env, KEY_QUERY_VALUE));
  exports.Set("KEY_READ", Napi::Number::New(env, KEY_READ));
  exports.Set("KEY_SET_VALUE", Napi::Number::New(env, KEY_SET_VALUE));
  exports.Set("KEY_WRITE", Napi::Number::New(env, KEY_WRITE));

  exports.Set("REG_OPTION_NON_VOLATILE",
              Napi::Number::New(env, REG_OPTION_NON_VOLATILE));
  exports.Set("REG_OPTION_VOLATILE",
              Napi::Number::New(env, REG_OPTION_VOLATILE));

  exports.Set("openKey", Napi::Function::New(env, RegKey::OpenKey));
  exports.Set("createKey", Napi::Function::New(env, RegKey::CreateKey));

  exports.Set("set", Napi::Function::New(env, RegSet));
  exports.Set("queryValue", Napi::Function::New(env, RegQuery));
  exports.Set("delete", Napi::Function::New(env, RegDelete));

  return exports;
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, InitModule);
