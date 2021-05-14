#include <iostream>
#include <memory>
#include <string>

#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

#include "curl_fetch.h"
#include "player_fetcher.h"
#include "team_fetcher.h"
#include "util.h"
#include <proto/player_team_service.grpc.pb.h>

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

void convert_logs(
    const std::vector<fantasy_ball::PlayerFetcher::DailyPlayerLog> &logs,
    playerteamservice::LogsForConfigResponse *logs_response) {
  logs_response->mutable_player_logs()->Reserve(logs.size());
  for (const auto &log : logs) {
    auto *log_response = logs_response->add_player_logs();
    log_response->mutable_player_description()->set_player_id(
        log.player_info.id);
    log_response->mutable_player_description()->set_first_name(
        log.player_info.first_name);
    log_response->mutable_player_description()->set_last_name(
        log.player_info.last_name);
    log_response->mutable_player_data()->set_points(log.player_log.points);
    log_response->mutable_player_data()->set_rebounds(
        log.player_log.total_rebounds);
    log_response->mutable_player_data()->set_assists(log.player_log.assists);
    log_response->mutable_player_data()->set_blocks(log.player_log.blocks);
    log_response->mutable_player_data()->set_steals(log.player_log.steals);
    log_response->mutable_player_data()->set_turnovers(
        log.player_log.turnovers);
    log_response->mutable_team_data()->set_home_team(log.game_info.home_team);
    log_response->mutable_team_data()->set_home_team_id(
        log.game_info.home_team_id);
    log_response->mutable_team_data()->set_away_team(log.game_info.away_team);
    log_response->mutable_team_data()->set_away_team_id(
        log.game_info.away_team_id);
    log_response->mutable_team_data()->set_home_score(log.game_info.home_score);
    log_response->mutable_team_data()->set_away_score(log.game_info.away_score);
    log_response->mutable_team_data()->set_event_id(log.game_info.event_id);
  }
}

class PlayerTeamServiceImpl final
    : public playerteamservice::PlayerTeamService::Service {
public:
  explicit PlayerTeamServiceImpl(fantasy_ball::PlayerFetcher *player_fetcher) {
    player_fetcher_ = player_fetcher;
  }

  Status AddPlayerToFetch(ServerContext *context,
                          const playerteamservice::AddPlayerRequest *request,
                          playerteamservice::DefaultResponse *reply) override {
    // TODO: For now, we'll enforce strict_search even if it's not set.
    if (request->player_description().player_id() < 0) {
      return Status::OK;
    }
    auto fetch_options =
        fantasy_ball::endpoint::Options::FromConfig(request->config());
    fantasy_ball::PlayerFetcher::PlayerInfoShort identity = {};
    identity.id = request->player_description().player_id();
    bool added = player_fetcher_->AddPlayer(identity, &fetch_options);
    if (!added) {
      return Status::CANCELLED;
    }
    return Status::OK;
  }

  Status FetchLog(ServerContext *context,
                  const playerteamservice::LogRequest *request,
                  playerteamservice::LogResponse *reply) override {
    return Status::OK;
  }

  Status
  FetchLogsForConfig(ServerContext *context,
                     const playerteamservice::LogsForConfigRequest *request,
                     playerteamservice::LogsForConfigResponse *reply) override {
    auto fetch_options =
        fantasy_ball::endpoint::Options::FromConfig(request->config());
    auto logs = player_fetcher_->GetRosterLog(&fetch_options);
    if (logs.empty()) {
      return Status::CANCELLED;
    }
    convert_logs(logs, reply);
    return Status::OK;
  }

private:
  fantasy_ball::PlayerFetcher *player_fetcher_;
};

int main(int argc, char *argv[]) {

  std::unique_ptr<CurlFetch> curl_fetch = std::make_unique<CurlFetch>();
  curl_fetch->Init();
  std::unique_ptr<TeamFetcher> team_fetcher =
      std::make_unique<TeamFetcher>(curl_fetch.get());
  std::unique_ptr<PlayerFetcher> player_fetcher =
      std::make_unique<PlayerFetcher>(curl_fetch.get(), team_fetcher.get());
  //   ServerBuilder builder;
  //   builder.AddListeningPort("0.0.0.0:50051",
  //   grpc::InsecureServerCredentials());

  //   fantasy_ball::PostgreSQLFetch psql_fetch;
  //   bool init_success = InitDB(&psql_fetch, true);
  //   if (!init_success) {
  //     return 0;
  //   }
  //   fantasy_ball::LeagueFetcher league_fetcher(&psql_fetch);
  //   LeagueServiceImpl my_service(&league_fetcher);
  //   builder.RegisterService(&my_service);

  //   std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
  //   std::cout << "Built server, now waiting for requests." << std::endl;
  //   server->Wait();
  return 0;
}