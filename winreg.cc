#include <napi.h>
#include <uv.h>

#include "winreg.hpp"

#include <locale>
#include <codecvt>

std::wstring Utf8ToUtf16(const std::string &str);

#define ThrowRegError(e) ThrowRegErrorImpl(env, e)

NAPI_NO_RETURN void ThrowRegErrorImpl(Napi::Env env,
                                      const winreg::RegException &e);

#define REG_TRY                                                                \
  auto env = info.Env();                                                       \
  try

#define REG_CATCH                                                              \
  catch (const winreg::RegException &e) {                                      \
    if (e.ErrorCode() == ERROR_FILE_NOT_FOUND) {                               \
      return Napi::Boolean::New(env, false);                                   \
    }                                                                          \
    ThrowRegErrorImpl(env, e);                                                          \
    return env.Null();                                                         \
  }                                                                            \
  catch (const std::exception &e) {                                            \
    Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();              \
    return env.Null();                                                         \
  }

class RegKey : public Napi::ObjectWrap<RegKey> {
public:
  static Napi::Object Init(Napi::Env env, Napi::Object exports);

  RegKey(const Napi::CallbackInfo &info);

  Napi::Value Create(const Napi::CallbackInfo &info);
  Napi::Value Open(const Napi::CallbackInfo &info);
  Napi::Value Close(const Napi::CallbackInfo &info);
  Napi::Value GetHandle(const Napi::CallbackInfo &info);
  Napi::Value GetString(const Napi::CallbackInfo &info);
  Napi::Value GetExpandString(const Napi::CallbackInfo &info);
  Napi::Value GetMultiString(const Napi::CallbackInfo &info);
  Napi::Value GetDword(const Napi::CallbackInfo &info);
  Napi::Value SetString(const Napi::CallbackInfo &info);
  Napi::Value SetExpandString(const Napi::CallbackInfo &info);
  Napi::Value SetDword(const Napi::CallbackInfo &info);
  Napi::Value DeleteValue(const Napi::CallbackInfo &info);
  Napi::Value DeleteKey(const Napi::CallbackInfo &info);
  Napi::Value EnumSubKeys(const Napi::CallbackInfo &info);
  Napi::Value EnumValues(const Napi::CallbackInfo &info);
  Napi::Value IsValid(const Napi::CallbackInfo &info);
private:
  winreg::RegKey _key;
  Napi::Value createKey(const Napi::CallbackInfo &info);
  Napi::Value openKey(const Napi::CallbackInfo &info);
};

Napi::Object RegKey::Init(Napi::Env env, Napi::Object exports) {
  // This method is used to hook the accessor and method callbacks
  Napi::Function func =
      DefineClass(env, "RegKey",
                  {InstanceMethod("open", &RegKey::Open),
                   InstanceMethod("create", &RegKey::Create),
                   InstanceMethod("close", &RegKey::Close),
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
  exports.Set("RegKey", func);
  return exports;
}

RegKey::RegKey(const Napi::CallbackInfo &info)
    : Napi::ObjectWrap<RegKey>(info) {
  auto env = info.Env();
  // throw an error if constructor is called without new keyword
  if (!info.IsConstructCall()) {
    Napi::Error::New(
        env, Napi::String::New(env, "RegKey::New - called without new keyword"))
        .ThrowAsJavaScriptException();
    return;
  }

  this->createKey(info);
}

Napi::Value RegKey::createKey(const Napi::CallbackInfo &info) {
  if (info.Length() == 0) {
  } else if (info.Length() == 2) {
    // expect arguments to be numbers
    if (!info[0].IsNumber() || !info[1].IsString()) {
      Napi::Error::New(
          info.Env(),
          Napi::String::New(
              info.Env(),
              "RegKey::New - expected arguments to be numbers and string"))
          .ThrowAsJavaScriptException();
      return info.Env().Null();
    }

    HKEY hkey = (HKEY)info[0].As<Napi::Number>().Int64Value();
    std::string p = info[1].As<Napi::String>();

    REG_TRY {
      this->_key.Create(hkey, Utf8ToUtf16(p));
      return info.This();
    }
    REG_CATCH
  } else if (info.Length() == 3) {
    // expect arguments to be numbers
    if (!info[0].IsNumber() || !info[1].IsString() || !info[2].IsNumber()) {
      Napi::Error::New(
          info.Env(),
          Napi::String::New(info.Env(), "RegKey::New - invalid arguments"))
          .ThrowAsJavaScriptException();
      return info.Env().Null();
    }
    HKEY hkey = (HKEY)info[0].As<Napi::Number>().Int64Value();
    std::string p = info[1].As<Napi::String>();
    DWORD access = (DWORD)info[2].As<Napi::Number>().Uint32Value();
    REG_TRY { this->_key.Create(hkey, Utf8ToUtf16(p), access); }
    REG_CATCH
  }

  // return the wrapped javascript instance
  return info.This();
}

Napi::Value RegKey::openKey(const Napi::CallbackInfo &info) {
  auto env = info.Env();
  if (info.Length() == 0) {
    //
  } else if (info.Length() == 2) {
    // expect arguments to be numbers
    if (!info[0].IsNumber() || !info[1].IsString()) {
      Napi::Error::New(
          env,
          Napi::String::New(
              env, "RegKey::New - expected arguments to be numbers and string"))
          .ThrowAsJavaScriptException();
      return env.Null();
    }

    HKEY hkey = (HKEY)info[0].As<Napi::Number>().Int64Value();
    std::string p = info[1].As<Napi::String>();
    this->_key.Open(hkey, Utf8ToUtf16(p));
  } else if (info.Length() == 3) {
    // expect arguments to be numbers
    if (!info[0].IsNumber() || !info[1].IsString() || !info[2].IsNumber()) {
      Napi::Error::New(
          env, Napi::String::New(env, "RegKey::New - invalid arguments"))
          .ThrowAsJavaScriptException();
      return env.Null();
    }
    HKEY hkey = (HKEY)info[0].As<Napi::Number>().Int64Value();
    std::string p = info[1].As<Napi::String>();
    DWORD access = (DWORD)info[2].As<Napi::Number>().Uint32Value();
    this->_key.Open(hkey, Utf8ToUtf16(p), access);
  }

  // return the wrapped javascript instance
  return info.This();
}

Napi::Value RegKey::Create(const Napi::CallbackInfo &info) {
  return this->createKey(info);
}

Napi::Value RegKey::Open(const Napi::CallbackInfo &info) {
  auto env = info.Env();
  try {
    return this->openKey(info);
  } catch (const winreg::RegException &e) {
    if (e.ErrorCode() == ERROR_FILE_NOT_FOUND) {
      return Napi::Boolean::New(env, false);
    }
    ThrowRegError(e);
    return env.Null();
  } catch (const std::exception &e) {
    Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
    return env.Null();
  }
}

Napi::Value RegKey::Close(const Napi::CallbackInfo &info) {
  auto env = info.Env();
  try {
    this->_key.Close();
    return info.This();
  } catch (const winreg::RegException &e) {
    ThrowRegError(e);
    return env.Null();
  } catch (const std::exception &e) {
    Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
  }
}

Napi::Value RegKey::IsValid(const Napi::CallbackInfo &info) {
  auto env = info.Env();
  return Napi::Boolean::New(env, this->_key.IsValid());
}

Napi::Value RegKey::GetHandle(const Napi::CallbackInfo &info) {
  auto env = info.Env();
  return Napi::Number::New(env, (int64_t) this->_key.Get());
}

Napi::Value RegKey::GetString(const Napi::CallbackInfo &info) {
  auto env = info.Env();
  try {
    std::string p = info[0].As<Napi::String>();
    auto v = this->_key.GetStringValue(Utf8ToUtf16(p));
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> convert;
    return Napi::String::New(env, convert.to_bytes(v));
  } catch (const winreg::RegException &e) {
    if (e.ErrorCode() == ERROR_FILE_NOT_FOUND && info.Length() > 1) {
      return info[1];
    }
    ThrowRegError(e);
    return env.Null();
  } catch (const std::exception &e) {
    Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
    return env.Null();
  }
}

Napi::Value RegKey::GetDword(const Napi::CallbackInfo &info) {
  auto env = info.Env();
  try {
    std::string p = info[0].As<Napi::String>();
    auto v = this->_key.GetDwordValue(Utf8ToUtf16(p));
    return Napi::Number::New(env, (uint32_t)v);
  } catch (const winreg::RegException &e) {
    if (e.ErrorCode() == ERROR_FILE_NOT_FOUND && info.Length() > 1) {
      return info[1];
    }

    ThrowRegError(e);
    return env.Null();
  } catch (const std::exception &e) {
    Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
  }
}

Napi::Value RegKey::GetExpandString(const Napi::CallbackInfo &info) {
  auto option = winreg::RegKey::ExpandStringOption::Expand;
  int defval = -1;
  if (info.Length() > 1) {
    if (info[1].IsBoolean()) {
      auto dontExpand = info[1].As<Napi::Boolean>().Value();
      if (dontExpand) {
        option = winreg::RegKey::ExpandStringOption::DontExpand;
      }
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
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> convert;
    return Napi::String::New(env, convert.to_bytes(v));
  } catch (const winreg::RegException &e) {
    if (e.ErrorCode() == ERROR_FILE_NOT_FOUND && defval > 0) {
      return info[defval];
    }
    ThrowRegError(e);
    return env.Null();
  } catch (const std::exception &e) {
    Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
    return env.Null();
  }
}

Napi::Value RegKey::GetMultiString(const Napi::CallbackInfo &info) {
  auto env = info.Env();
  try {
    std::string p = info[0].As<Napi::String>();
    auto vec = this->_key.GetMultiStringValue(Utf8ToUtf16(p));
    auto arr = Napi::Array::New(env, vec.size());
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> convert;
    for (size_t i = 0; i < vec.size(); ++i) {
      (arr).Set(i, Napi::String::New(env, convert.to_bytes(vec[i])));
    }
    return arr;
  } catch (const winreg::RegException &e) {
    if (e.ErrorCode() == ERROR_FILE_NOT_FOUND && info.Length() > 1) {
      return info[1];
    }

    ThrowRegError(e);
    return env.Null();
  } catch (const std::exception &e) {
    Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
    return env.Null();
  }
}

Napi::Value RegKey::SetString(const Napi::CallbackInfo &info) {
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
  } catch (const winreg::RegException &e) {
    ThrowRegError(e);
    return env.Null();
  } catch (const std::exception &e) {
    Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
    return env.Null();
  }
}

Napi::Value RegKey::SetExpandString(const Napi::CallbackInfo &info) {
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
  } catch (const winreg::RegException &e) {
    ThrowRegError(e);
    return env.Null();
  } catch (const std::exception &e) {
    Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
    return env.Null();
  }
}

Napi::Value RegKey::SetDword(const Napi::CallbackInfo &info) {
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
  } catch (const winreg::RegException &e) {
    ThrowRegError(e);
    return env.Null();
  } catch (const std::exception &e) {
    Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
    return env.Null();
  }
}
Napi::Value RegKey::DeleteValue(const Napi::CallbackInfo &info) {
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
  } catch (const winreg::RegException &e) {
    ThrowRegError(e);
    return env.Null();
  } catch (const std::exception &e) {
    Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
    return env.Null();
  }
}

Napi::Value RegKey::DeleteKey(const Napi::CallbackInfo &info) {
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
  } catch (const winreg::RegException &e) {
    ThrowRegError(e);
    return env.Null();
  } catch (const std::exception &e) {
    Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
    return env.Null();
  }
}

Napi::Value RegKey::EnumSubKeys(const Napi::CallbackInfo &info) {
  auto env = info.Env();
  try {
    auto v = this->_key.EnumSubKeys();
    auto arr = Napi::Array::New(env, v.size());
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> convert;
    for (size_t i = 0; i < v.size(); ++i) {
      (arr).Set(i, Napi::String::New(env, convert.to_bytes(v[i])));
    }
    return arr;
  } catch (const winreg::RegException &e) {
    ThrowRegError(e);
    return env.Null();
  } catch (const std::exception &e) {
    Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
    return env.Null();
  }
}

Napi::Value RegKey::EnumValues(const Napi::CallbackInfo &info) {
  auto env = info.Env();
  try {
    auto v = this->_key.EnumValues();
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> convert;
    auto obj = Napi::Object::New(env);
    for (size_t i = 0; i < v.size(); ++i) {
      obj.Set(Napi::String::New(env, convert.to_bytes(v[i].first)),
              Napi::Number::New(env, (uint32_t)v[i].second));
    }
    return obj;
  } catch (const winreg::RegException &e) {
    ThrowRegError(e);
    return env.Null();
  } catch (const std::exception &e) {
    Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
    return env.Null();
  }
}

NAPI_NO_RETURN void ThrowRegErrorImpl(Napi::Env env,
                                      const winreg::RegException &e) {
  auto err = Napi::Error::New(env, e.what());
  err.Set(Napi::String::New(env, "name"), Napi::String::New(env, "RegError"));
  err.Set(Napi::String::New(env, "code"),
          Napi::Number::New(env, e.ErrorCode()));
  err.ThrowAsJavaScriptException();
}

std::wstring Utf8ToUtf16(const std::string &str) {
  std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> convert;
  return convert.from_bytes(str);
}

Napi::Object InitModule(Napi::Env env, Napi::Object exports) {
  RegKey::Init(env, exports);
  exports.Set("HKEY_CLASSES_ROOT",
              Napi::Number::New(env, (uint32_t)(ULONG_PTR)HKEY_CLASSES_ROOT));
  exports.Set("HKEY_LOCAL_MACHINE",
              Napi::Number::New(env, (uint32_t)(ULONG_PTR)HKEY_LOCAL_MACHINE));
  exports.Set("HKEY_CURRENT_USER",
              Napi::Number::New(env, (uint32_t)(ULONG_PTR)HKEY_CURRENT_USER));

  exports.Set("KEY_WOW64_32KEY", Napi::Number::New(env, KEY_WOW64_32KEY));
  exports.Set("KEY_WOW64_64KEY", Napi::Number::New(env, KEY_WOW64_64KEY));

  exports.Set("KEY_QUERY_VALUE", Napi::Number::New(env, KEY_QUERY_VALUE));
  exports.Set("KEY_READ", Napi::Number::New(env, KEY_READ));
  exports.Set("KEY_SET_VALUE", Napi::Number::New(env, KEY_SET_VALUE));
  exports.Set("KEY_WRITE", Napi::Number::New(env, KEY_WRITE));
  return exports;
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, InitModule);
