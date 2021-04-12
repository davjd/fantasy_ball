#ifndef TEAM_FETCHER_H_
#define TEAM_FETCHER_H_

#include "curl_fetch.h"
#include "util.h"
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace fantasy_ball {

// Team (record, roster).
class TeamFetcher {
public:
  struct GameMatchup {
    GameMatchup() = default;
    std::string home_team;
    int home_team_id;
    std::string away_team;
    int away_team_id;
    std::string home_score;
    std::string away_score;
    int event_id;

    static GameMatchup deserialize_json(const nlohmann::json &json_content) {
      GameMatchup matchup = {};
      matchup.event_id = -1;
      if (!json_content.contains("schedule") ||
          !json_content.contains("score")) {
        return matchup;
      }
      const auto &schedule = json_content["schedule"];
      const auto &score = json_content["score"];
      matchup.home_team = schedule["homeTeam"]["abbreviation"];
      matchup.home_team_id = schedule["homeTeam"]["id"];
      matchup.away_team = schedule["awayTeam"]["abbreviation"];
      matchup.away_team_id = schedule["awayTeam"]["id"];
      matchup.home_score = score["homeScoreTotal"];
      matchup.away_score = score["awayScoreTotal"];
      matchup.event_id = schedule["id"];
      return matchup;
    }
  };
  TeamFetcher(CurlFetch *curl_fetch);
  ~TeamFetcher();

  std::vector<GameMatchup> GetGameReferences(endpoint::Options *options);

private:
  // Base url for MySportsFeed daily player log endpoint.
  static const std::string kBaseUrl;

  // NOTE: This class doesn't have ownership of this object.
  CurlFetch *curl_fetch_;

  std::string construct_endpoint_url(endpoint::Options *options);
};

} // namespace fantasy_ball

#endif // TEAM_FETCHER_H_