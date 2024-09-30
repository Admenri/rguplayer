
#ifndef net_h
#define net_h

#include <string>
#include <unordered_map>

namespace net {

typedef std::unordered_map<std::string, std::string> StringMap;

class HTTPResponse {
 public:
  int GetStatus();
  std::string& GetBody();
  StringMap& GetHeaders();

 private:
  friend class HTTPRequest;
  HTTPResponse();

  int status_;
  std::string body_;
  StringMap headers_;
};

class HTTPRequest {
 public:
  HTTPRequest(const char* dest, bool follow_redirects = true);
  ~HTTPRequest();

  HTTPRequest(const HTTPRequest&) = delete;
  HTTPRequest& operator=(const HTTPRequest&) = delete;

  StringMap& GetHeaders();

  HTTPResponse GET();
  HTTPResponse POST(StringMap& postData);
  HTTPResponse POST(const char* body, const char* content_type);

 private:
  std::string destination_;
  StringMap headers_;
  bool follow_location_;
};

}  // namespace net

#endif /* net_h */
