
#include <stdio.h>

#include "binding/mri/net/net_http.h"

#if defined(ENABLE_NET_SSL)
#define CPPHTTPLIB_OPENSSL_SUPPORT
#endif

#include "base/exceptions/exception.h"
#include "third_party/LUrlParser/LUrlParser.h"
#include "third_party/httplib/httplib.h"

static const char* kHttpErrorNames[] = {"Success",
                                        "Unknown",
                                        "Connection",
                                        "Bind IP Address",
                                        "Read",
                                        "Write",
                                        "Exceed Redirect Count",
                                        "Canceled",
                                        "SSLConnection",
                                        "SSL Loading Certs",
                                        "SSL Server Verification",
                                        "Unsupported Multipart Boundary Chars"};

static const char* kUrlErrorNames[] = {"OK",
                                       "Uninitialized",
                                       "No URL Character",
                                       "Invalid Scheme Name",
                                       "No Double Slash",
                                       "No At Sign",
                                       "Unexpected End-Of-Line",
                                       "No Slash"};

LUrlParser::ParseURL ReadURLInternal(const char* url) {
  LUrlParser::ParseURL p = LUrlParser::ParseURL::parseURL(std::string(url));
  if (!p.isValid() || p.errorCode_) {
    throw base::Exception(base::Exception::ContentError, "Invalid URL: %s",
                          kUrlErrorNames[p.errorCode_]);
  }

  return p;
}

std::string GetURLHost(LUrlParser::ParseURL url) {
  std::string host;
  host += url.scheme_;
  host += "://";
  host += url.host_;

  int port;
  if (!url.port_.empty() && url.getPort(&port)) {
    host += ":";
    host += std::to_string(port);
  }
  return host;
}

std::string GetQueryPath(LUrlParser::ParseURL url) {
  std::string path = "/";
  path += url.path_;

  if (!url.query_.empty()) {
    path += "?";
    path += url.query_;
  }

  return path;
}

namespace net {

HTTPResponse::HTTPResponse() : status_(0), body_(), headers_(StringMap()) {}

std::string& HTTPResponse::GetBody() {
  return body_;
}

StringMap& HTTPResponse::GetHeaders() {
  return headers_;
}

int HTTPResponse::GetStatus() {
  return status_;
}

HTTPRequest::HTTPRequest(const char* dest, bool follow_redirects)
    : destination_(dest),
      headers_(StringMap()),
      follow_location_(follow_redirects) {}

HTTPRequest::~HTTPRequest() {}

StringMap& HTTPRequest::GetHeaders() {
  return headers_;
}

HTTPResponse HTTPRequest::GET() {
  HTTPResponse ret;
  auto target = ReadURLInternal(destination_.c_str());

  httplib::Client* client = nullptr;
  try {
    client = new httplib::Client(GetURLHost(target).c_str());
  } catch (std::exception& e) {
    delete client;
    throw base::Exception(base::Exception::ContentError,
                          "Failed to create HTTP client (%s)", e.what());
  }

  httplib::Headers head;

  // Seems to need to be disabled for now, at least on macOS
#ifdef ENABLE_NET_SSL
  client->enable_server_certificate_verification(false);
#endif
  client->set_follow_location(follow_location_);

  for (auto const& h : headers_)
    head.emplace(h.first, h.second);

  if (auto result = client->Get(GetQueryPath(target).c_str(), head)) {
    auto response = result.value();
    ret.status_ = response.status;
    ret.body_ = response.body;

    for (auto const& h : response.headers)
      ret.headers_.emplace(h.first, h.second);
  } else {
    auto err = result.error();
    std::string errname = httplib::to_string(err);
    delete client;
    throw base::Exception(base::Exception::ContentError,
                          "Failed to GET %s (%i: %s)", destination_.c_str(),
                          err, errname.c_str());
  }

  delete client;
  return ret;
}

HTTPResponse HTTPRequest::POST(StringMap& postData) {
  HTTPResponse ret;
  auto target = ReadURLInternal(destination_.c_str());

  httplib::Client* client = nullptr;
  try {
    client = new httplib::Client(GetURLHost(target).c_str());
  } catch (std::exception& e) {
    delete client;
    throw base::Exception(base::Exception::ContentError,
                          "Failed to create HTTP client (%s)", e.what());
  }

  httplib::Headers head;
  httplib::Params params;

  // Seems to need to be disabled for now, at least on macOS
#ifdef ENABLE_NET_SSL
  client->enable_server_certificate_verification(false);
#endif
  client->set_follow_location(follow_location_);

  for (auto const& h : headers_)
    head.emplace(h.first, h.second);

  for (auto const& p : postData)
    params.emplace(p.first, p.second);

  if (auto result = client->Post(GetQueryPath(target).c_str(), head, params)) {
    auto response = result.value();
    ret.status_ = response.status;
    ret.body_ = response.body;

    for (auto h : response.headers)
      ret.headers_.emplace(h.first, h.second);
  } else {
    auto err = result.error();
    std::string errname = httplib::to_string(err);
    delete client;
    throw base::Exception(base::Exception::ContentError,
                          "Failed to POST %s (%i: %s)", destination_.c_str(),
                          err, errname.c_str());
  }
  delete client;
  return ret;
}

HTTPResponse HTTPRequest::POST(const char* body, const char* content_type) {
  HTTPResponse ret;
  auto target = ReadURLInternal(destination_.c_str());

  httplib::Client* client = nullptr;
  try {
    client = new httplib::Client(GetURLHost(target).c_str());
  } catch (std::exception& e) {
    delete client;
    throw base::Exception(base::Exception::ContentError,
                          "Failed to create HTTP client (%s)", e.what());
  }

  httplib::Headers head;

  // Seems to need to be disabled for now, at least on macOS
#ifdef ENABLE_NET_SSL
  client->enable_server_certificate_verification(false);
#endif
  client->set_follow_location(true);

  for (auto const& h : headers_)
    head.emplace(h.first, h.second);

  if (auto result = client->Post(GetQueryPath(target).c_str(), head, body,
                                 content_type)) {
    auto response = result.value();
    ret.status_ = response.status;
    ret.body_ = response.body;

    for (auto const& h : response.headers)
      ret.headers_.emplace(h.first, h.second);
  } else {
    auto err = result.error();
    std::string errname = httplib::to_string(err);
    delete client;
    throw base::Exception(base::Exception::ContentError,
                          "Failed to POST %s (%i: %s)", destination_.c_str(),
                          err, errname.c_str());
  }
  delete client;
  return ret;
}

}  // namespace net
