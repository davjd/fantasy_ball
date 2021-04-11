#include "util.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <curl/curl.h>

namespace fantasy_ball {

int letter_to_int(char letter) { return std::tolower(letter) - 'a'; }

std::string string_to_lower(const std::string &str) {
  std::string result;
  std::transform(
      str.begin(), str.end(), std::back_inserter(result),
      [](unsigned char c) -> unsigned char { return std::tolower(c); });
  return result;
}

void trim_new_line(std::string *str) {
  str->erase(std::remove_if(str->begin(), str->end(),
                            [](unsigned char c) { return c == '\n'; }),
             str->end());
}

bool is_number(const std::string &word) {
  return !word.empty() && std::find_if(word.begin(), word.end(), [](char c) {
                            return !std::isdigit(c);
                          }) == word.end();
}

float float_or_negative(const std::string &word) {
  if (!word.empty() && word.find('.') != std::string::npos) {
    return std::stof(word);
  }
  return -1;
}

int int_or_negative(const std::string &word) {
  if (word.empty() || !is_number(word)) {
    return -1;
  }
  return std::stoi(word);
}

std::vector<std::string> split(const std::string &str,
                               const std::string &delimiter) {
  size_t pos_start = 0, pos_end, delim_len = delimiter.length();
  std::string token;
  std::vector<std::string> res;

  while ((pos_end = str.find(delimiter, pos_start)) != std::string::npos) {
    token = str.substr(pos_start, pos_end - pos_start);
    pos_start = pos_end + delim_len;
    res.push_back(token);
  }

  res.push_back(str.substr(pos_start));
  return res;
}

std::string replace(std::string subject, const std::string &search,
                    const std::string &replace) {
  size_t pos = 0;
  while ((pos = subject.find(search, pos)) != std::string::npos) {
    subject.replace(pos, search.length(), replace);
    pos += replace.length();
  }
  return subject;
}

std::string base64_encode(const std::string &in) {

  std::string out;

  int val = 0, valb = -6;
  for (unsigned char c : in) {
    val = (val << 8) + c;
    valb += 8;
    while (valb >= 0) {
      out.push_back(
          "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"
              [(val >> valb) & 0x3F]);
      valb -= 6;
    }
  }
  if (valb > -6)
    out.push_back(
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"
            [((val << 8) >> (valb + 8)) & 0x3F]);
  while (out.size() % 4)
    out.push_back('=');
  return out;
}

std::string base64_decode(const std::string &in) {

  std::string out;

  std::vector<int> T(256, -1);
  for (int i = 0; i < 64; i++)
    T["ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[i]] =
        i;

  int val = 0, valb = -8;
  for (unsigned char c : in) {
    if (T[c] == -1)
      break;
    val = (val << 6) + T[c];
    valb += 6;
    if (valb >= 0) {
      out.push_back(char((val >> valb) & 0xFF));
      valb -= 8;
    }
  }
  return out;
}

namespace endpoint {
std::string read_msf_api_key() {
  std::ifstream file(msf_api_file_path);
  if (!file.is_open()) {
    return "<invalid_file_path>";
  }
  std::string api_key;
  file >> api_key;
  return api_key;
}

void init_msf_curl_header(const std::string &api_key, CURL *curl_instance) {
  struct curl_slist *list = NULL;
  const std::string key_token =
      "Basic " + base64_encode(api_key + ":MYSPORTSFEEDS");
  const std::string auth_token = "Authorization: " + key_token;
  list = curl_slist_append(list, auth_token.c_str());
  curl_easy_setopt(curl_instance, CURLOPT_HTTPHEADER, list);
}
} // namespace endpoint
} // namespace fantasy_ball