#include <iostream>
#include <memory>
#include <string>

#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

#include <proto/league_service.grpc.pb.h>

#include "league_fetcher.h"
#include "postgre_sql_fetch.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

// Logic and data behind the server's behavior.
class LeagueServiceImpl final : public leagueservice::LeagueService::Service {
public:
  explicit LeagueServiceImpl(fantasy_ball::LeagueFetcher *league_fetcher) {
    league_fetcher_ = league_fetcher;
  }

  Status
  CreateUserAccount(ServerContext *context,
                    const leagueservice::CreateUserAccountRequest *request,
                    leagueservice::AuthToken *reply) override {
    league_fetcher_->CreateUserAccount(request, reply);
    return Status::OK;
  }

  Status LoginUserAccount(ServerContext *context,
                          const leagueservice::LoginUserAccountRequest *request,
                          leagueservice::AuthToken *reply) override {
    league_fetcher_->LoginUserAccount(request, reply);
    return Status::OK;
  }

  Status CreateLeague(ServerContext *context,
                      const leagueservice::CreateLeagueRequest *request,
                      leagueservice::CreateLeagueResponse *reply) override {
    league_fetcher_->CreateLeague(request, reply);
    return Status::OK;
  }

  Status JoinLeague(ServerContext *context,
                    const leagueservice::JoinLeagueRequest *request,
                    leagueservice::DefaultResponse *reply) override {
    league_fetcher_->JoinLeague(request, reply);
    return Status::OK;
  }

  Status
  UpdateLeagueBasicSettings(ServerContext *context,
                            const leagueservice::LeagueBasicSettings *request,
                            leagueservice::DefaultResponse *reply) override {
    return Status::OK;
  }

  Status
  UpdateTransactionSettings(ServerContext *context,
                            const leagueservice::TransactionSettings *request,
                            leagueservice::DefaultResponse *reply) override {
    return Status::OK;
  }

  Status UpdateWaiverSettings(ServerContext *context,
                              const leagueservice::WaiverSettings *request,
                              leagueservice::DefaultResponse *reply) override {
    return Status::OK;
  }

  Status MakeDraftPick(ServerContext *context,
                       const leagueservice::DraftPickRequest *request,
                       leagueservice::DefaultResponse *reply) override {
    league_fetcher_->MakeDraftPick(request, reply);
    return Status::OK;
  }

  Status UpdateLineup(ServerContext *context,
                      const leagueservice::UpdateLineupRequest *request,
                      leagueservice::DefaultResponse *reply) override {
    league_fetcher_->UpdateLineup(request, reply);
    return Status::OK;
  }

  Status
  GetBasicUserInformation(ServerContext *context,
                          const leagueservice::AuthToken *request,
                          leagueservice::BasicUserInformation *reply) override {
    league_fetcher_->GetBasicUserInformation(request, reply);
    return Status::OK;
  }

  Status GetMatchup(ServerContext *context,
                    const leagueservice::MatchupRequest *request,
                    leagueservice::MatchupResponse *reply) override {
    league_fetcher_->GetMatchup(request, reply);
    return Status::OK;
  }

  Status GetMatch(ServerContext *context,
                  const leagueservice::MatchRequest *request,
                  leagueservice::MatchResponse *reply) override {
    league_fetcher_->GetMatch(request, reply);
    return Status::OK;
  }

  Status GetLineup(ServerContext *context,
                   const leagueservice::LineupRequest *request,
                   leagueservice::LineupResponse *reply) override {
    league_fetcher_->GetLineup(request, reply);
    return Status::OK;
  }

  Status
  GetLeagueSettings(ServerContext *context,
                    const leagueservice::LeagueSettingsRequest *request,
                    leagueservice::LeagueSettingsResponse *reply) override {
    return Status::OK;
  }

  Status
  GetLeagueStandings(ServerContext *context,
                     const leagueservice::LeagueStandingsRequest *request,
                     leagueservice::LeagueStandingsResponse *reply) override {
    return Status::OK;
  }

  Status GetRoster(ServerContext *context,
                   const leagueservice::RosterRequest *request,
                   leagueservice::RosterResponse *reply) override {
    league_fetcher_->GetRoster(request, reply);
    return Status::OK;
  }

  Status
  GetLeaguesForMember(ServerContext *context,
                      const leagueservice::LeaguesForMemberRequest *request,
                      leagueservice::LeaguesForMemberResponse *reply) override {
    league_fetcher_->GetLeaguesForMember(request, reply);
    return Status::OK;
  }

private:
  fantasy_ball::LeagueFetcher *league_fetcher_;
};

// Helper to initialize the Postgre database.
bool InitDB(fantasy_ball::PostgreSQLFetch *psql_fetch,
            bool delete_tables = false) {
  bool db_init_success = psql_fetch->Init();
  if (!db_init_success) {
    std::cout << "Couldn't initialize PostgreSQL database." << std::endl;
    return false;
  }
  auto *connection = psql_fetch->GetCurrentConnection();
  pqxx::work W{*connection};
  if (delete_tables) {
    psql_fetch->DeleteBaseTables(&W);
  }
  psql_fetch->CreateBaseTables(&W);
  W.commit();
  return true;
}

int main(int argc, char *argv[]) {
  ServerBuilder builder;
  builder.AddListeningPort("0.0.0.0:50051", grpc::InsecureServerCredentials());

  fantasy_ball::PostgreSQLFetch psql_fetch;
  bool init_success = InitDB(&psql_fetch, true);
  if (!init_success) {
    return 0;
  }
  fantasy_ball::LeagueFetcher league_fetcher(&psql_fetch);
  LeagueServiceImpl my_service(&league_fetcher);
  builder.RegisterService(&my_service);

  std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
  std::cout << "Built server, now waiting for requests." << std::endl;
  server->Wait();
  return 0;
}