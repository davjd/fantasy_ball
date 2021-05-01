#ifndef LEAGUE_FETCHER_H_
#define LEAGUE_FETCHER_H_

#include "postgre_sql_fetch.h"

#include <string>
#include <tuple>
#include <vector>

namespace fantasy_ball {

class LeagueFetcher {
public:
  struct PlayerSlot {
    PlayerSlot() = default;
    std::string player_id;
    std::string position;
    int slot_id;
  };

  LeagueFetcher() = delete;
  ~LeagueFetcher() = default;

  // NOTE: Expects the psql_fetcher to be initialized. This class takes no
  // ownership of the object.
  LeagueFetcher(PostgreSQLFetch *psql_fetcher);

  // Creates a new user account with the provided information.
  // Returns a generated unique authentication token that should be be used for
  // all calls to update and insert.
  std::string CreateUserAccount(const std::string &username,
                                const std::string &email,
                                const std::string &password,
                                const std::string &first_name,
                                const std::string &last_name);

  // Creates a new league with the provided information.
  // Returns the league id for the new league.
  int CreateLeague(std::string token, const std::string &league_name);

  // Adds the user for the given auth_token to the given league.
  void JoinLeague(std::string token, int league_id);

  // Add the given user into the given league_id.
  // TODO: Add safety check to ensure that only the league commissioner can call
  // this.
  void AddLeagueMember(int user_account_id, int league_id);

  // Selects a player in the league's draft.
  void MakeDraftPick(std::string token, int pick_number, int player_id_selected,
                     int league_id, const std::string &positions_available);

  // Updates the user's lineup roster for the given matchup date.
  void UpdateLineup(std::string token, int lineup_slot_id,
                    const std::vector<PlayerSlot> &lineup);

  // Get an information description for the requested user.
  std::tuple<std::string, std::string, std::string, std::string>
  GetBasicUserInformation(std::string token);

  // Get the specified matchup.
  std::tuple<std::string, std::string, int, int, int, std::string, std::string,
             int>
  GetMatchup(std::string token, int week_num, int league_id);

  // Get the specified match.
  std::tuple<int, int, std::string> GetMatch(std::string token, int match_id);

  // Get the lineup slots for a given match.
  std::tuple<int, std::vector<PlayerSlot>> GetLineup(std::string token,
                                                     int match_id);

  // TODO: Complete the other helper methods for the RPC service.

private:
  // Fetcher that does contains the Postgre connection.
  // NOTE: This class has no ownership of this pointer.
  PostgreSQLFetch *psql_fetcher_;

  // Creates default league settings and variant settings, returns
  // league_settings_id.
  int init_league_settings(int commissioner_id);

  // Retrieves the user account if for the given auth token.
  int auth_token_to_account_id(std::string token);
};

} // namespace fantasy_ball

#endif // LEAGUE_FETCHER_H_