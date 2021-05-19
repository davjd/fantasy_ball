#include "fantasy_service_client.h"

#include <grpcpp/client_context.h>

namespace fantasy_ball {

FantasyServiceClient::FantasyServiceClient(
    std::shared_ptr<grpc::Channel> league_channel,
    std::shared_ptr<grpc::Channel> player_channel)
    : league_stub_(leagueservice::LeagueService::NewStub(league_channel)),
      player_stub_(
          playerteamservice::PlayerTeamService::NewStub(player_channel)) {}

std::string FantasyServiceClient::RegisterAccount(
    const std::string &username, const std::string &email,
    const std::string &password, const std::string &first_name,
    const std::string &last_name) {
  leagueservice::CreateUserAccountRequest req;
  leagueservice::AuthToken result;
  grpc::ClientContext context;

  req.set_username(username);
  req.set_email(email);
  req.set_password(password);
  req.set_first_name(first_name);
  req.set_last_name(last_name);
  grpc::Status status = league_stub_->CreateUserAccount(&context, req, &result);
  if (!status.ok()) {
    return status.error_message();
  }
  return result.token();
}

std::string FantasyServiceClient::Login(const std::string &username,
                                        const std::string &password) {
  leagueservice::LoginUserAccountRequest req;
  leagueservice::AuthToken result;
  grpc::ClientContext context;

  req.set_username(username);
  req.set_password(password);
  grpc::Status status = league_stub_->LoginUserAccount(&context, req, &result);
  if (!status.ok()) {
    return status.error_message();
  }
  return result.token();
}

std::vector<leagueservice::RosterInfo>
FantasyServiceClient::GetRoster(const std::string &token, int league_id,
                                const std::string &filter) {
  leagueservice::RosterRequest req;
  leagueservice::RosterResponse result;
  grpc::ClientContext context;

  req.mutable_auth_token()->set_token(token);
  req.set_status_filter(filter);
  req.set_league_id(league_id);
  grpc::Status status = league_stub_->GetRoster(&context, req, &result);
  return std::vector<leagueservice::RosterInfo>(result.roster().begin(),
                                                result.roster().end());
}

std::vector<leagueservice::LeagueDescription>
FantasyServiceClient::GetLeaguesForMember(const std::string &token,
                                          const std::string &year) {
  leagueservice::LeaguesForMemberRequest req;
  leagueservice::LeaguesForMemberResponse result;
  grpc::ClientContext context;

  req.mutable_auth_token()->set_token(token);
  req.set_season_year(year);
  grpc::Status status =
      league_stub_->GetLeaguesForMember(&context, req, &result);
  return std::vector<leagueservice::LeagueDescription>(
      result.league_descriptions().begin(), result.league_descriptions().end());
}

bool FantasyServiceClient::JoinLeague(const std::string &token, int league_id) {
  leagueservice::JoinLeagueRequest req;
  leagueservice::DefaultResponse result;
  grpc::ClientContext context;

  req.mutable_auth_token()->set_token(token);
  req.set_league_id(league_id);
  grpc::Status status = league_stub_->JoinLeague(&context, req, &result);
  // TODO: Need to do an overall better job of communicating/propagating errors.
  if (result.message().find("ERROR") != std::string::npos) {
    return false;
  }
  return status.ok();
}

fantasy_ball::TournamentManager::RosterMember
FantasyServiceClient::GetPlayerDescription(const std::string &first_name,
                                           const std::string &last_name) {
  playerteamservice::MinimalPlayerDescription req;
  playerteamservice::PlayerDescription result;
  grpc::ClientContext context;

  if (first_name.empty() || last_name.empty()) {
    return {};
  }
  req.set_first_name(first_name);
  req.set_last_name(last_name);
  grpc::Status status =
      player_stub_->GetPlayerDescription(&context, req, &result);

  fantasy_ball::TournamentManager::RosterMember member = {};
  if (!status.ok()) {
    return member;
  }
  member.first_name = result.first_name();
  member.last_name = result.last_name();
  member.player_id = result.player_id();
  member.team = result.team();
  member.team_id = result.team_id();
  return member;
}

fantasy_ball::TournamentManager::RosterMember
FantasyServiceClient::GetPlayerDescription(int player_id) {
  playerteamservice::PlayerId req;
  playerteamservice::PlayerDescription result;
  grpc::ClientContext context;

  if (player_id < 1) {
    return {};
  }
  req.set_id(player_id);
  grpc::Status status =
      player_stub_->GetPlayerDescriptionForId(&context, req, &result);

  fantasy_ball::TournamentManager::RosterMember member = {};
  if (!status.ok()) {
    return member;
  }
  member.first_name = result.first_name();
  member.last_name = result.last_name();
  member.player_id = result.player_id();
  member.team = result.team();
  member.team_id = result.team_id();
  member.positions = result.positions();
  return member;
}

int FantasyServiceClient::CreateLeague(const std::string &token,
                                       const std::string &league_name) {
  leagueservice::CreateLeagueRequest req;
  leagueservice::CreateLeagueResponse result;
  grpc::ClientContext context;

  req.mutable_auth_token()->set_token(token);
  req.set_league_name(league_name);
  grpc::Status status = league_stub_->CreateLeague(&context, req, &result);
  // TODO: Need to do an overall better job of communicating/propagating errors.
  if (!status.ok()) {
    return 0;
  }
  return result.league_id();
}

void FantasyServiceClient::MakeDraftPick(const std::string &token,
                                         int pick_number, int player_id,
                                         int league_id,
                                         const std::string &positions) {
  leagueservice::DraftPickRequest req;
  leagueservice::DefaultResponse result;
  grpc::ClientContext context;

  req.mutable_auth_token()->set_token(token);
  req.set_league_id(league_id);
  req.set_pick_number(pick_number);
  req.set_player_selected_id(player_id);
  req.set_league_id(league_id);
  req.set_positions(positions);

  grpc::Status status = league_stub_->MakeDraftPick(&context, req, &result);
  if (!status.ok()) {
    return;
  }
}

void FantasyServiceClient::AddPlayerToFetchBatch(
    int player_id, const std::string &fetch_date,
    const std::string &season_start) {
  playerteamservice::AddPlayerRequest req;
  playerteamservice::DefaultResponse result;
  grpc::ClientContext context;

  if (player_id == 0 || fetch_date.empty()) {
    return;
  }
  req.mutable_player_description()->set_player_id(player_id);
  req.mutable_config()->set_date(fetch_date);
  req.mutable_config()->set_season_start(season_start);
  grpc::Status status = player_stub_->AddPlayerToFetch(&context, req, &result);
}

playerteamservice::LogsForConfigResponse
FantasyServiceClient::GetLogsForConfig(const std::string &fetch_date,
                                       const std::string &season_start) {
  if (fetch_date.empty()) {
    return {};
  }

  playerteamservice::LogsForConfigRequest req;
  playerteamservice::LogsForConfigResponse result;
  grpc::ClientContext context;

  req.mutable_config()->set_date(fetch_date);
  req.mutable_config()->set_season_start(season_start);
  grpc::Status status =
      player_stub_->FetchLogsForConfig(&context, req, &result);

  if (!status.ok()) {
    return {};
  }
  return result;
}
} // namespace fantasy_ball