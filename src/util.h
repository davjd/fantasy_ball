#ifndef UTIL_H_
#define UTIL_H_

#include <curl/curl.h>
#include <string>
#include <vector>

namespace fantasy_ball {

// Directory containing configurations.
static const std::string config_directory = "config/";

int letter_to_int(char letter);

std::string string_to_lower(const std::string &str);

void trim_new_line(std::string *str);

bool is_number(const std::string &word);

float float_or_negative(const std::string &word);

int int_or_negative(const std::string &word);

std::vector<std::string> split(const std::string &str,
                               const std::string &delimiter = " ");

std::string replace(std::string subject, const std::string &search,
                    const std::string &replace);

std::string base64_encode(const std::string &in);

std::string base64_decode(const std::string &in);

namespace endpoint {
// File path to file that contains mysportsfeeds api key.
static const std::string msf_api_file_path = "config/mysportsfeeds_api_key.txt";

std::string read_msf_api_key();

void init_msf_curl_header(const std::string &api_key, CURL *curl_instance);

// Options for the various data fetchers.
struct Options {
  Options() = default;

  bool operator==(Options const &rhs) const {
    return (this->strict_search == rhs.strict_search) &&
           (this->date == rhs.date) && (this->version == rhs.version) &&
           (this->season_start == rhs.season_start);
  }

  // Forces player searches to require player id in order to remove
  // duplicates.
  bool strict_search;

  // Date used for the MySportsFeed daily player log endpoint.
  // Format: <4-year><2-month><2-day>, with each digit being the size of each
  // section. NOTE: Remember to update the season_start when changing the year
  // date. e.g. 20210319
  std::string date;

  // Version parameter for the MySportsFeed endpoint, should remain unchanged.
  std::string version;

  // The season year for the selected season to retrieve daily logs from.
  // e.g. 2020-2021 season.
  std::string season_start;
};
} // namespace endpoint

namespace postgre {
// Path to the config, which contains the user, password, etc.
static const std::string postgre_config_path = "config/postgre.txt";
static const std::string postgre_scripts_directory = "postgre_scripts/";
std::string read_postgre_config();
std::vector<std::string> get_commands_from_file(const std::string filename);
} // namespace postgre
} // namespace fantasy_ball
#endif // UTIL_H_