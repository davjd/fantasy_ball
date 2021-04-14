#include "curl_fetch.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <memory>
#include <string>

#include "util.h"

namespace fantasy_ball {

CurlFetch::CurlFetch() {}
CurlFetch::~CurlFetch() {
  if (curl_instance_) {
    curl_easy_cleanup(curl_instance_);
  }
}

void CurlFetch::Init() {
  // Initialize the Curl instance.
  curl_instance_ = curl_easy_init();
  init_curl_options(curl_instance_, &buffer_);

  // Set the api key for the MySportsFeed endpoint.
  api_config_.msf_api_key = endpoint::read_msf_api_key();

  // TODO: Decide if we want to have this here.
  endpoint::init_msf_curl_header(api_config_.msf_api_key, curl_instance_);
}

std::string CurlFetch::GetContent(const std::string &url) {
  buffer_.clear();
  curl_easy_setopt(curl_instance_, CURLOPT_URL, url.c_str());
  curl_ret_ = curl_easy_perform(curl_instance_);
  return buffer_;
}

void CurlFetch::init_curl_options(CURL *curl_instance, std::string *buffer) {
  // TODO: Add compression flag.
  curl_easy_setopt(curl_instance, CURLOPT_WRITEFUNCTION, write_callback);
  curl_easy_setopt(curl_instance, CURLOPT_WRITEDATA, buffer);
  curl_easy_setopt(curl_instance, CURLOPT_FRESH_CONNECT, 1);
}

size_t CurlFetch::write_callback(void *contents, size_t size, size_t nmemb,
                                 void *userp) {
  ((std::string *)userp)->append((char *)contents, size * nmemb);
  return size * nmemb;
}

CURL *CurlFetch::curl_instance() { return curl_instance_; }

CURLcode CurlFetch::curl_ret() { return curl_ret_; }

std::string CurlFetch::Key() { return api_config_.msf_api_key; }
} // namespace fantasy_ball