#ifndef PLAYER_FETCHER_H_
#define PLAYER_FETCHER_H_

#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "team_fetcher.h"
#include "util.h"

namespace fantasy_ball {
class CurlFetch;

// This class retrieves player data (statistics) from various APIs (currently
// only MySportsFeed).
class PlayerFetcher {
public:
  struct PlayerIdentity {
    PlayerIdentity() = default;
    std::string first_name;
    std::string last_name;
    std::string img_url;
    std::string position;

    // An id of -1 signifies that an error occured when creating this object and
    // should be recreated.
    int id;

    // Reads a single playerReferences json item into the player_identity
    // object.
    // NOTE: Expects safety checks outside and before this function call.
    static PlayerIdentity
    deserialize_json(const nlohmann::json &player_reference) {
      PlayerIdentity player;
      if (!player_reference.contains("id")) {
        return player;
      }
      player.id = player_reference["id"];
      player.first_name = player_reference["firstName"];
      player.last_name = player_reference["lastName"];
      player.img_url = player_reference["officialImageSrc"];

      // NOTE: Right now, we can only retrieve the primary position, meaning, a
      // player might be able to play multiple positions but the API doesn't
      // return it.
      // TODO: Figure out if we can get secondary positions.
      player.position = player_reference["primaryPosition"];
      return player;
    }
  };

  struct PlayerLog {
    PlayerLog() = default;

    int seconds_played;
    int field_goals_made;
    int field_goals_attempt;
    float field_goal_percentage;
    int three_points_made;
    int three_points_attempt;
    float three_points_percentage;
    int two_points_made;
    int two_points_attempt;
    float two_points_percentage;
    int free_throws_made;
    int free_throws_attempt;
    float free_throws_percentage;
    int offensive_rebounds;
    int defensive_rebounds;
    int total_rebounds;
    int assists;
    int steals;
    int blocks;
    int turnovers;
    int personal_fouls;
    int points;
    int game_event_id;
    int player_id;

    // Reads MySportsFeed daily player log from json format to a player_log
    // object. NOTE: Expects that safety checks on jthe son_content were done
    // outside and before this function call. Also, this expects a single
    // gamelogs json item in the json_content.
    static PlayerLog deserialize_json(const nlohmann::json &game_log) {
      PlayerLog log;
      if (!game_log.contains("stats")) {
        return log;
      }
      if (game_log.contains("game")) {
        log.game_event_id = game_log["game"]["id"];
      }
      const auto &stats = game_log["stats"];
      log.seconds_played = stats["miscellaneous"]["minSeconds"];
      log.field_goals_made = stats["fieldGoals"]["fgMade"];
      log.field_goals_attempt = stats["fieldGoals"]["fgAtt"];
      log.field_goal_percentage = stats["fieldGoals"]["fgPct"];
      log.three_points_made = stats["fieldGoals"]["fg3PtMade"];
      log.three_points_attempt = stats["fieldGoals"]["fg3PtAtt"];
      log.three_points_percentage = stats["fieldGoals"]["fg3PtPct"];
      log.two_points_made = stats["fieldGoals"]["fg2PtMade"];
      log.two_points_attempt = stats["fieldGoals"]["fg2PtAtt"];
      log.two_points_percentage = stats["fieldGoals"]["fg2PtPct"];
      log.free_throws_made = stats["freeThrows"]["ftMade"];
      log.free_throws_attempt = stats["freeThrows"]["ftAtt"];
      log.free_throws_percentage = stats["freeThrows"]["ftPct"];
      log.offensive_rebounds = stats["rebounds"]["offReb"];
      log.defensive_rebounds = stats["rebounds"]["defReb"];
      log.total_rebounds = stats["rebounds"]["reb"];
      log.assists = stats["offense"]["ast"];
      log.steals = stats["defense"]["stl"];
      log.blocks = stats["defense"]["blk"];
      log.turnovers = stats["defense"]["tov"];
      log.personal_fouls = stats["miscellaneous"]["fouls"];
      log.points = stats["offense"]["pts"];
      log.player_id = game_log["player"]["id"];
      return log;
    }
  };

  struct DailyPlayerLog {
    DailyPlayerLog() = default;
    PlayerIdentity player_info;
    TeamFetcher::GameMatchup game_info;
    PlayerLog player_log;
  };

  PlayerFetcher(CurlFetch *curl_fetch, TeamFetcher *team_fetcher,
                endpoint::Options *options = nullptr);
  ~PlayerFetcher();

  // Add a player to the roster to do batch fetches. Returns true if player was
  // added successfully, otherwise false.
  bool AddPlayer(const std::string &fname = "", const std::string &lname = "",
                 const int &id = -1, endpoint::Options *options = nullptr);

  // Add to a roster with the given fetch options to do a batch retrieval.
  void AddToRoster(std::vector<int> roster,
                   endpoint::Options *options = nullptr);

  // Return the daily log for the given player. May utilize a cached copy or do
  // API call.
  DailyPlayerLog GetPlayerLog(PlayerIdentity player,
                              endpoint::Options *options = nullptr);

  // Retrieve all the daily logs for the given roster with the specified
  // options.
  std::unordered_map<int, DailyPlayerLog>
  GetRosterLog(endpoint::Options *options = nullptr);

  // Updates the date parameter for the daily player log endpoint.
  void SetDateForEndpoint(const std::string &date);

  // Returns the default options that are used when adding players to the
  // fetched rosters.
  endpoint::Options GetDefaultOptions();

  // Gets the game log for the specified player, which constructs the struct
  // from an endpoint call. NOTE: Since this is a static function, it will force
  // an API call instead of checking the cache.
  static DailyPlayerLog RetrivePlayerLog(const int &id);
  static DailyPlayerLog RetrivePlayerLog(const std::string &fname,
                                         const std::string &lname = "");

private:
  // Represents a single fetch request from the user to the daily player log
  // endpoint. This usually will be used to split requests based on different
  // endpoint parameters, say different requests for different dates.
  struct PlayerLogFetch {
    PlayerLogFetch() = default;

    // Options that contain parameters for the daily player log endpoint.
    endpoint::Options fetch_options;

    // The list of players that we want to retrieve their daily logs for.
    std::vector<PlayerIdentity> roster;
  };

  // List of fetches for daily player logs requests to the endpoint to process.
  std::vector<PlayerLogFetch> player_log_fetches_;

  // Cache copy of the retrieved daily player logs.
  std::unordered_map<int,
                     std::vector<std::pair<endpoint::Options, DailyPlayerLog>>>
      cache_;

  // Current and default options for the daily player log endpoint.
  endpoint::Options options_;

  // NOTE: This class doesn't have ownership of this object.
  CurlFetch *curl_fetch_;

  // NOTE: This class doesn't have ownership of this object.
  TeamFetcher *team_fetcher_;

  // Constructs a string with the player list section of the MySportsFeed daily
  // log endpoint. e.g. player=lebron-james,kyrie-irving
  std::string make_player_list_url();

  // TODO: Add comments for the following functions.
  std::string make_player_list_url(PlayerIdentity player);
  void add_player_to_list_url(PlayerIdentity player,
                              std::string *player_list_url);

  // Constructs a string with the base daily log endpoint url.
  std::string make_base_daily_log_url(endpoint::Options *options = nullptr);

  // Creates the player log object from a JSON string.
  // NOTE: The string parameter should be of JSON format and should've been fed
  // from a Curl perform call to the daily player log MySportsFeed endpoint. We
  // also do various safety checks on the json data.
  std::vector<DailyPlayerLog>
  construct_player_logs(const std::string &curl_response,
                        endpoint::Options *options);

  // Retrieves all game logs found in the daily player log endpoint call json
  // object.
  nlohmann::json get_game_logs(const nlohmann::json &json_daily_log);

  // Retrieves all player references found in the daily player log endpoint call
  // json object.
  nlohmann::json get_player_references(const nlohmann::json &json_daily_log);

  // Retrieves the game log for the specified player from a daily player log
  // endcall json object.
  nlohmann::json get_game_log(const nlohmann::json &json_daily_log,
                              int player_id);

  // Retrieves the player reference for the specified player from a daily player
  // log endcall json object.
  nlohmann::json get_player_reference(const nlohmann::json &json_daily_log,
                                      int player_id);

  // Finds the game log for the given player id, from a list of game logs.
  nlohmann::json find_game_log(const nlohmann::json &game_logs, int player_id);

  // Find the player reference for the given player id, from a list of player
  // references.
  nlohmann::json find_player_reference(const nlohmann::json &player_refs,
                                       int player_id);

  // Retrieves the daily player log from the MySportsFeed endpoint.
  DailyPlayerLog
  retrieve_daily_player_log(PlayerIdentity player,
                            endpoint::Options *options = nullptr);

  // Default parameters to the daily player log endpoint.
  static const std::string kDefaultVersion;
  static const std::string kDefaultSeasonStart;
  static const std::string kDefaultDate;
  static const bool kDefaultStrictSearch;

  // Base url for MySportsFeed daily player log endpoint.
  static const std::string base_url_;
};

} // namespace fantasy_ball

#endif // PLAYER_FETCHER_H_