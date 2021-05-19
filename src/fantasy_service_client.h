#ifndef FANTASY_SERVICE_CLIENT_H_
#define FANTASY_SERVICE_CLIENT_H_

#include <memory>
#include <vector>

#include <grpcpp/channel.h>
#include <proto/league_service.grpc.pb.h>
#include <proto/league_service.pb.h>
#include <proto/player_team_service.grpc.pb.h>
#include <proto/player_team_service.pb.h>

#include "tournament_manager.h"

namespace fantasy_ball {

// This class is the client of all needed GRPC services.
class FantasyServiceClient {
public:
  FantasyServiceClient(std::shared_ptr<grpc::Channel> league_channel,
                       std::shared_ptr<grpc::Channel> player_channel);
  ~FantasyServiceClient() = default;

  std::string RegisterAccount(const std::string &username,
                              const std::string &email,
                              const std::string &password,
                              const std::string &first_name,
                              const std::string &last_name);

  std::string Login(const std::string &username, const std::string &password);

  std::vector<leagueservice::RosterInfo>
  GetRoster(const std::string &token, int league_id,
            const std::string &filter = "active");

  std::vector<leagueservice::LeagueDescription>
  GetLeaguesForMember(const std::string &token, const std::string &year);

  bool JoinLeague(const std::string &token, int league_id);

  fantasy_ball::TournamentManager::RosterMember
  GetPlayerDescription(const std::string &first_name,
                       const std::string &last_name);

  fantasy_ball::TournamentManager::RosterMember
  GetPlayerDescription(int player_id);

  int CreateLeague(const std::string &token, const std::string &league_name);

  void MakeDraftPick(const std::string &token, int pick_number, int player_id,
                     int league_id, const std::string &positions);

  void
  AddPlayerToFetchBatch(int player_id, const std::string &fetch_date,
                        const std::string &season_start = "2020-2021-regular");

  playerteamservice::LogsForConfigResponse
  GetLogsForConfig(const std::string &fetch_date,
                   const std::string &season_start);

private:
  std::unique_ptr<leagueservice::LeagueService::Stub> league_stub_;
  std::unique_ptr<playerteamservice::PlayerTeamService::Stub> player_stub_;
};

} // namespace fantasy_ball

#endif // FANTASY_SERVICE_CLIENT_H_