#ifndef LEAGUE_FETCHER_H_
#define LEAGUE_FETCHER_H_

#include "postgre_sql_fetch.h"

#include <proto/league_service.pb.h>
#include <string>

namespace fantasy_ball {

class LeagueFetcher {
public:
  LeagueFetcher() = delete;
  ~LeagueFetcher() = default;

  // NOTE: Expects the psql_fetcher to be initialized. This class takes no
  // ownership of the object.
  LeagueFetcher(PostgreSQLFetch *psql_fetcher);

  // Creates a new user account with the provided information.
  // Returns a generated unique authentication token that should be be used for
  // all calls to update and insert.
  void CreateUserAccount(const leagueservice::CreateUserAccountRequest *request,
                         leagueservice::AuthToken *reply);

  // Login using provide user account info. Returns an authentication code.
  void LoginUserAccount(const leagueservice::LoginUserAccountRequest *request,
                        leagueservice::AuthToken *reply);

  // Creates a new league with the provided information.
  // Returns the league id for the new league.
  void CreateLeague(const leagueservice::CreateLeagueRequest *request,
                    leagueservice::CreateLeagueResponse *reply);

  // Adds the user for the given auth_token to the given league.
  void JoinLeague(const leagueservice::JoinLeagueRequest *request,
                  leagueservice::DefaultResponse *reply);

  // Add the given user into the given league_id.
  // TODO: Add safety check to ensure that only the league commissioner can call
  // this.
  bool AddLeagueMember(int user_account_id, int league_id);

  // Selects a player in the league's draft.
  void MakeDraftPick(const leagueservice::DraftPickRequest *request,
                     leagueservice::DefaultResponse *reply);

  // Updates the user's lineup roster for the given matchup date.
  void UpdateLineup(const leagueservice::UpdateLineupRequest *request,
                    leagueservice::DefaultResponse *reply);

  // Get an information description for the requested user.
  void GetBasicUserInformation(const leagueservice::AuthToken *request,
                               leagueservice::BasicUserInformation *reply);

  // Get the specified matchup.
  void GetMatchup(const leagueservice::MatchupRequest *request,
                  leagueservice::MatchupResponse *reply);

  // Get the specified match.
  void GetMatch(const leagueservice::MatchRequest *request,
                leagueservice::MatchResponse *reply);

  // Get the lineup slots for a given match.
  void GetLineup(const leagueservice::LineupRequest *request,
                 leagueservice::LineupResponse *reply);

  // rpc GetRoster(RosterRequest) returns (RosterResponse)
  void GetRoster(const leagueservice::RosterRequest *request,
                 leagueservice::RosterResponse *reply);

  void
  GetLeaguesForMember(const leagueservice::LeaguesForMemberRequest *request,
                      leagueservice::LeaguesForMemberResponse *reply);

  // TODO: Complete the other helper methods for the RPC service.
private:
  // Fetcher that does contains the Postgre connection.
  // NOTE: This class has no ownership of this pointer.
  PostgreSQLFetch *psql_fetcher_;

  // Creates default league settings and variant settings, returns
  // league_settings_id.
  //
  // TODO: Much of this could be just setting up default values inside the
  // create table script file.
  int init_league_settings(int commissioner_id);

  // Retrieves the user account if for the given auth token.
  int auth_token_to_account_id(std::string token);
};

} // namespace fantasy_ball

#endif // LEAGUE_FETCHER_H_