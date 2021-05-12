#include "fantasy_service_client.h"

#include <grpcpp/client_context.h>

namespace fantasy_ball {

FantasyServiceClient::FantasyServiceClient(
    std::shared_ptr<grpc::Channel> channel)
    : league_stub_(leagueservice::LeagueService::NewStub(channel)) {}

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
} // namespace fantasy_ball