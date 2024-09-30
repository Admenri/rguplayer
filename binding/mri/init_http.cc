// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#include "binding/mri/init_http.h"

#include "net/net_http.h"
#include "third_party/json5pp/json5pp.hpp"

VALUE stringMap2hash(net::StringMap& map) {
  VALUE ret = rb_hash_new();
  for (auto const& item : map) {
    VALUE key = rb_utf8_str_new_cstr(item.first.c_str());
    VALUE val = rb_utf8_str_new_cstr(item.second.c_str());
    rb_hash_aset(ret, key, val);
  }
  return ret;
}

net::StringMap hash2StringMap(VALUE hash) {
  net::StringMap ret;
  Check_Type(hash, T_HASH);

  VALUE keys = rb_funcall(hash, rb_intern("keys"), 0);
  for (int i = 0; i < RARRAY_LEN(keys); i++) {
    VALUE key = rb_ary_entry(keys, i);
    VALUE val = rb_hash_aref(hash, key);
    SafeStringValue(key);
    SafeStringValue(val);

    ret.emplace(RSTRING_PTR(key), RSTRING_PTR(val));
  }
  return ret;
}

bool strContainsStr(std::string& first, std::string second) {
  return first.find(second) != std::string::npos;
}

VALUE getResponseBody(net::HTTPResponse& res) {
#if RAPI_FULL >= 190
  auto it = res.GetHeaders().find("Content-Type");
  if (it == res.GetHeaders().end())
    return rb_str_new(res.GetBody().c_str(), res.GetBody().length());

  std::string& ctype = it->second;

  if (strContainsStr(ctype, "text/plain") ||
      strContainsStr(ctype, "application/json") ||
      strContainsStr(ctype, "application/xml") ||
      strContainsStr(ctype, "text/html") || strContainsStr(ctype, "text/css") ||
      strContainsStr(ctype, "text/javascript") ||
      strContainsStr(ctype, "application/x-sh") ||
      strContainsStr(ctype, "image/svg+xml") ||
      strContainsStr(ctype, "application/x-httpd-php"))
    return rb_utf8_str_new(res.GetBody().c_str(), res.GetBody().length());

#endif
  return rb_str_new(res.GetBody().c_str(), res.GetBody().length());
}

VALUE formResponse(net::HTTPResponse& res) {
  VALUE ret = rb_hash_new();

  rb_hash_aset(ret, ID2SYM(rb_intern("status")), INT2NUM(res.GetStatus()));
  rb_hash_aset(ret, ID2SYM(rb_intern("body")), getResponseBody(res));
  rb_hash_aset(ret, ID2SYM(rb_intern("headers")),
               stringMap2hash(res.GetHeaders()));
  return ret;
}

#if RAPI_MAJOR >= 2
void* httpGetInternal(void* req) {
  VALUE ret;

  net::HTTPResponse res = ((net::HTTPRequest*)req)->GET();
  ret = formResponse(res);

  return (void*)ret;
}
#endif

namespace binding {

MRI_METHOD(http_get) {
  MRI_GUARD_BEGIN
  VALUE path, rheaders, redirect;
  rb_scan_args(argc, argv, "12", &path, &rheaders, &redirect);
  SafeStringValue(path);

  bool rd = ((redirect == Qtrue) ? true : false);

  net::HTTPRequest req(RSTRING_PTR(path), rd);
  if (rheaders != Qnil) {
    auto headers = hash2StringMap(rheaders);
    req.GetHeaders().insert(headers.begin(), headers.end());
  }

#if RAPI_MAJOR >= 2
  return (VALUE)drop_gvl_guard(httpGetInternal, &req, 0, 0);
#else
  return (VALUE)httpGetInternal(&req);
#endif

  MRI_GUARD_END
  return Qnil;
}

#if RAPI_MAJOR >= 2

typedef struct {
  net::HTTPRequest* req;
  net::StringMap* postData;
} httpPostInternalArgs;

void* httpPostInternal(void* args) {
  VALUE ret;

  net::HTTPRequest* req = ((httpPostInternalArgs*)args)->req;
  net::StringMap* postData = ((httpPostInternalArgs*)args)->postData;

  net::HTTPResponse res = req->POST(*postData);
  ret = formResponse(res);

  return (void*)ret;
}
#endif

MRI_METHOD(http_post) {
  MRI_GUARD_BEGIN
  VALUE path, postDataHash, rheaders, redirect;
  rb_scan_args(argc, argv, "22", &path, &postDataHash, &rheaders, &redirect);
  SafeStringValue(path);

  bool rd = ((redirect == Qtrue) ? true : false);

  net::HTTPRequest req(RSTRING_PTR(path), rd);
  if (rheaders != Qnil) {
    auto headers = hash2StringMap(rheaders);
    req.GetHeaders().insert(headers.begin(), headers.end());
  }

  net::StringMap postData = hash2StringMap(postDataHash);
  httpPostInternalArgs args{&req, &postData};

#if RAPI_MAJOR >= 2
  return (VALUE)drop_gvl_guard(httpPostInternal, &args, 0, 0);
#else
  return httpPostInternal(&args);
#endif

  MRI_GUARD_END
  return Qnil;
}

#if RAPI_MAJOR >= 2
typedef struct {
  net::HTTPRequest* req;
  const char* body;
  const char* ctype;
} httpPostBodyInternalArgs;

void* httpPostBodyInternal(void* args) {
  VALUE ret;

  net::HTTPRequest* req = ((httpPostBodyInternalArgs*)args)->req;
  const char* reqbody = ((httpPostBodyInternalArgs*)args)->body;
  const char* reqctype = ((httpPostBodyInternalArgs*)args)->ctype;

  net::HTTPResponse res = req->POST(reqbody, reqctype);
  ret = formResponse(res);

  return (void*)ret;
}
#endif

MRI_METHOD(http_post_body) {
  MRI_GUARD_BEGIN
  VALUE path, body, ctype, rheaders;
  rb_scan_args(argc, argv, "31", &path, &body, &ctype, &rheaders);
  SafeStringValue(path);
  SafeStringValue(body);
  SafeStringValue(ctype);

  net::HTTPRequest req(RSTRING_PTR(path));
  if (rheaders != Qnil) {
    auto headers = hash2StringMap(rheaders);
    req.GetHeaders().insert(headers.begin(), headers.end());
  }

  httpPostBodyInternalArgs args{&req, RSTRING_PTR(body), RSTRING_PTR(ctype)};
#if RAPI_MAJOR >= 2
  return (VALUE)drop_gvl_guard(httpPostBodyInternal, &args, 0, 0);
#else
  return httpPostBodyInternal(&args);
#endif

  MRI_GUARD_END
  return Qnil;
}

VALUE json2rb(json5pp::value const& v) {
  if (v.is_null())
    return Qnil;

  if (v.is_number())
    return rb_float_new(v.as_number());

  if (v.is_string())
    return rb_utf8_str_new_cstr(v.as_string().c_str());

  if (v.is_boolean())
    return MRI_BOOL_NEW(v.as_boolean());

  if (v.is_integer())
    return LL2NUM(v.as_integer());

  if (v.is_array()) {
    auto& a = v.as_array();
    VALUE ret = rb_ary_new();
    for (auto item : a) {
      rb_ary_push(ret, json2rb(item));
    }
    return ret;
  }

  if (v.is_object()) {
    auto& o = v.as_object();
    VALUE ret = rb_hash_new();
    for (auto const& pair : o) {
      rb_hash_aset(ret, rb_utf8_str_new_cstr(pair.first.c_str()),
                   json2rb(pair.second));
    }
    return ret;
  }

  // This should be unreachable
  return Qnil;
}

json5pp::value rb2json(VALUE v) {
  if (v == Qnil)
    return json5pp::value(nullptr);

  if (RB_TYPE_P(v, RUBY_T_FLOAT))
    return json5pp::value(RFLOAT_VALUE(v));

  if (RB_TYPE_P(v, RUBY_T_STRING))
    return json5pp::value(RSTRING_PTR(v));

  if (v == Qtrue || v == Qfalse)
    return json5pp::value(RTEST(v));

  if (RB_TYPE_P(v, RUBY_T_FIXNUM))
    return json5pp::value(NUM2DBL(v));

  if (RB_TYPE_P(v, RUBY_T_ARRAY)) {
    json5pp::value ret_value = json5pp::array({});
    auto& ret = ret_value.as_array();
    for (int i = 0; i < RARRAY_LEN(v); i++) {
      ret.push_back(rb2json(rb_ary_entry(v, i)));
    }
    return ret_value;
  }

  if (RTEST(rb_funcall(v, rb_intern("is_a?"), 1, rb_cHash))) {
    json5pp::value ret_value = json5pp::object({});
    auto& ret = ret_value.as_object();

    VALUE keys = rb_funcall(v, rb_intern("keys"), 0);

    for (int i = 0; i < RARRAY_LEN(keys); i++) {
      VALUE key = rb_ary_entry(keys, i);
      SafeStringValue(key);
      VALUE val = rb_hash_aref(v, key);
      ret.emplace(RSTRING_PTR(key), rb2json(val));
    }

    return ret_value;
  }

  throw base::Exception(base::Exception::ContentError,
                        "Invalid value for JSON: %s",
                        RSTRING_PTR(rb_inspect(v)));

  // This should be unreachable
  return json5pp::value(0);
}

MRI_METHOD(json_parse) {
  MRI_GUARD_BEGIN
  VALUE jsonv;
  rb_scan_args(argc, argv, "1", &jsonv);
  SafeStringValue(jsonv);

  json5pp::value v;
  try {
    v = json5pp::parse5(RSTRING_PTR(jsonv));
  } catch (const std::exception& e) {
    throw base::Exception(base::Exception::ContentError,
                          "Failed to parse JSON: %s", e.what());
  }

  return json2rb(v);
  MRI_GUARD_END
  return Qnil;
}

MRI_METHOD(json_stringify) {
  MRI_GUARD_BEGIN
  VALUE obj;
  rb_scan_args(argc, argv, "1", &obj);

  json5pp::value v = rb2json(obj);
  return rb_utf8_str_new_cstr(
      v.stringify5(json5pp::rule::space_indent<>()).c_str());
  MRI_GUARD_END
  return Qnil;
}

void InitHTTPBinding() {
  VALUE mNet = rb_define_module("HTTPLite");
  MriDefineModuleFunction(mNet, "get", http_get);
  MriDefineModuleFunction(mNet, "post", http_post);
  MriDefineModuleFunction(mNet, "post_body", http_post_body);

  VALUE mNetJSON = rb_define_module_under(mNet, "JSON");
  MriDefineModuleFunction(mNetJSON, "stringify", json_stringify);
  MriDefineModuleFunction(mNetJSON, "parse", json_parse);
}

}  // namespace binding
