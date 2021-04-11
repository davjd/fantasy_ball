#ifndef CURL_FETCH_H_
#define CURL_FETCH_H_

#include <curl/curl.h>
#include <string>

namespace fantasy_ball {

// This class will wrap a CURL object and provide useful fetching capabilities
// to various endpoints.
class CurlFetch {
public:
  CurlFetch();
  ~CurlFetch();

  // Initializes internal objects, including api keys, curl instance, etc.
  void Init();

  // Makes a Curl call to the specified url, and returns the contents.
  std::string GetContent(const std::string &url);

  // Sets basic curl instance options, including buffer and callback, and force
  // refresh.
  static void init_curl_options(CURL *curl_instance, std::string *buffer);

  // Returns the Curl instance initialized by this class.
  CURL *curl_instance();

  // Returns the latest return code from a Curl perform call.
  CURLcode curl_ret();

  std::string Key();

private:
  struct api_config {
    std::string msf_api_key;
  } api_config_;
  std::string buffer_;
  CURL *curl_instance_;
  CURLcode curl_ret_;

  static size_t write_callback(void *contents, size_t size, size_t nmemb,
                               void *userp);
};

} // namespace fantasy_ball

#endif // CURL_FETCH_H_