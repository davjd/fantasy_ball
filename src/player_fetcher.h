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
    // TODO: Temporarily, we are using this struct to also handle fetch
    // requests. We should create a separate struct just for this use case
    // without the unused fields.
    PlayerIdentity() = default;
    std::string first_name;
    std::string last_name;
    std::string img_url;
    std::string position;

    // A negative id signifies that an error occured when creating this object
    // and should be recreated.
    int id = -1;

    // Reads a single playerReferences json item into the player_identity
    // object.
    // NOTE: Expects safety checks outside and before this function call.
    static PlayerIdentity
    deserialize_json(const nlohmann::json &player_reference) {
      PlayerIdentity player;
      if (!player_reference.contains("id")) {
        return player;
      }
      player.id = player_reference["id"].get<int>();
      player.first_name = player_reference["firstName"].get<std::string>();
      player.last_name = player_reference["lastName"].get<std::string>();
      // Some players do not have an image source (may be because they're new
      // players). TODO: Insert a default image source for those that don't have
      // an image assigned them.
      if (!player_reference["officialImageSrc"].is_null()) {
        player.img_url =
            player_reference["officialImageSrc"].get<std::string>();
      }

      // NOTE: Right now, we can only retrieve the primary position, meaning, a
      // player might be able to play multiple positions but the API doesn't
      // return it.
      // TODO: Figure out if we can get secondary positions.
      player.position = player_reference["primaryPosition"].get<std::string>();
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
      log.seconds_played = stats["miscellaneous"]["minSeconds"].get<int>();
      log.field_goals_made = stats["fieldGoals"]["fgMade"].get<int>();
      log.field_goals_attempt = stats["fieldGoals"]["fgAtt"].get<int>();
      log.field_goal_percentage = stats["fieldGoals"]["fgPct"].get<float>();
      log.three_points_made = stats["fieldGoals"]["fg3PtMade"].get<int>();
      log.three_points_attempt = stats["fieldGoals"]["fg3PtAtt"].get<int>();
      log.three_points_percentage =
          stats["fieldGoals"]["fg3PtPct"].get<float>();
      log.two_points_made = stats["fieldGoals"]["fg2PtMade"].get<int>();
      log.two_points_attempt = stats["fieldGoals"]["fg2PtAtt"].get<int>();
      log.two_points_percentage = stats["fieldGoals"]["fg2PtPct"].get<float>();
      log.free_throws_made = stats["freeThrows"]["ftMade"].get<int>();
      log.free_throws_attempt = stats["freeThrows"]["ftAtt"].get<int>();
      log.free_throws_percentage = stats["freeThrows"]["ftPct"].get<float>();
      log.offensive_rebounds = stats["rebounds"]["offReb"].get<int>();
      log.defensive_rebounds = stats["rebounds"]["defReb"].get<int>();
      log.total_rebounds = stats["rebounds"]["reb"].get<int>();
      log.assists = stats["offense"]["ast"].get<int>();
      log.steals = stats["defense"]["stl"].get<int>();
      log.blocks = stats["defense"]["blk"].get<int>();
      log.turnovers = stats["defense"]["tov"].get<int>();
      log.personal_fouls = stats["miscellaneous"]["fouls"].get<int>();
      log.points = stats["offense"]["pts"].get<int>();
      log.player_id = game_log["player"]["id"].get<int>();
      return log;
    }
  };

  struct DailyPlayerLog {
    DailyPlayerLog() = default;
    PlayerIdentity player_info;
    TeamFetcher::GameMatchup game_info;
    PlayerLog player_log;

    // Temporary way to handle errors in creating a daily log. Pass in a
    // negative error code to signifiy that there was an error creating this
    // object.
    // TODO: Update this later when we find a way to handle or log errors.
    static DailyPlayerLog MakeFaultyLog(int error_code) {
      DailyPlayerLog faulty_log = {};
      if (error_code > 0) {
        error_code *= -1;
      }
      faulty_log.player_info.id = error_code;
      return faulty_log;
    }
  };

  // Contains a short description of a player.
  struct PlayerInfoShort {
    PlayerInfoShort() = default;
    std::string first_name;
    std::string last_name;
    // A negative id signifies that an error occured when creating this object
    // and should be recreated.
    int id = -1;
    std::string team;
    int team_id;

    void read_json(const nlohmann::json &player_reference) {
      if (!player_reference.contains("player") ||
          !player_reference.contains("teamAsOfDate")) {
        id = -1;
        return;
      }
      const auto &player_map = player_reference["player"];
      const auto &team_map = player_reference["teamAsOfDate"];
      id = player_map["id"].get<int>();
      first_name = player_map["firstName"].get<std::string>();
      last_name = player_map["lastName"].get<std::string>();
      team = team_map["abbreviation"].get<std::string>();
      team_id = team_map["id"].get<int>();
    }

    static PlayerInfoShort
    deserialize_json(const nlohmann::json &player_reference) {
      PlayerInfoShort player;
      if (!player_reference.contains("player") ||
          !player_reference.contains("teamAsOfDate")) {
        return player;
      }
      const auto &player_map = player_reference["player"];
      const auto &team_map = player_reference["teamAsOfDate"];
      player.id = player_map["id"].get<int>();
      player.first_name = player_map["firstName"].get<std::string>();
      player.last_name = player_map["lastName"].get<std::string>();
      player.team = team_map["abbreviation"].get<std::string>();
      player.team_id = team_map["id"].get<int>();
      return player;
    }

    bool is_empty() const {
      return id == -1 && (first_name.empty() && last_name.empty());
    }
  };

  PlayerFetcher(CurlFetch *curl_fetch, TeamFetcher *team_fetcher,
                endpoint::Options *options = nullptr);
  ~PlayerFetcher();

  // Add a player to the roster to do batch fetches. Returns true if player was
  // added successfully, otherwise false.
  bool AddPlayer(const PlayerInfoShort &player_info,
                 endpoint::Options *options = nullptr);

  // Add to a roster with the given fetch options to do a batch retrieval.
  void AddToRoster(const std::vector<PlayerInfoShort> &roster,
                   endpoint::Options *options = nullptr);

  // Return the daily log for the given player. May utilize a cached copy or do
  // API call.
  DailyPlayerLog GetPlayerLog(const PlayerInfoShort &player,
                              endpoint::Options *options = nullptr);

  // Retrieve all the daily logs for the given roster with the specified
  // options.
  std::vector<DailyPlayerLog>
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

  // Retrieves the player id for the given names. Inserts the information
  void GetPlayerInfoShort(PlayerInfoShort *player_info,
                          endpoint::Options *options = nullptr);

private:
  // Represents a single fetch request from the user to the daily player log
  // endpoint. This usually will be used to split requests based on different
  // endpoint parameters, say different requests for different dates.
  struct PlayerLogFetch {
    PlayerLogFetch() = default;

    // Options that contain parameters for the daily player log endpoint.
    endpoint::Options fetch_options;

    // The list of players that we want to retrieve their daily logs for.
    std::vector<PlayerInfoShort> roster;
  };

  // List of fetches for daily player logs requests to the endpoint to process.
  std::vector<PlayerLogFetch> player_log_fetches_;

  // Cache copy of the retrieved daily player logs.
  // TODO: This can get complicated and might neeed its own class.
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
  std::string make_player_list_url(const std::vector<PlayerInfoShort> &roster);

  // TODO: Add comments for the following functions.
  std::string make_player_list_url(const PlayerInfoShort &player);
  void add_player_to_list_url(const PlayerInfoShort &player,
                              std::string *player_list_url);

  // Constructs a string with the base daily log endpoint url.
  std::string make_base_daily_log_url(endpoint::Options *options = nullptr);

  std::string make_base_player_info_url(endpoint::Options *options);

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
  retrieve_daily_player_log(const PlayerInfoShort &player,
                            endpoint::Options *options = nullptr);

  // Retrieves multiple player logs using the players in the roster.
  std::vector<DailyPlayerLog>
  retrieve_daily_player_logs(const std::vector<PlayerInfoShort> &roster,
                             endpoint::Options *options);

  // Default parameters to the daily player log endpoint.
  static const std::string kDefaultVersion;
  static const std::string kDefaultSeasonStart;
  static const std::string kDefaultDate;
  static const bool kDefaultStrictSearch;

  // Base url for MySportsFeed daily player log endpoint.
  static const std::string kDailyPlayerLogUrl;
  static const std::string kPlayerInfoUrl;
};

} // namespace fantasy_ball

#endif // PLAYER_FETCHER_H_