#include <iostream>
#include <memory>
#include <string>

#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

#include <proto/league_service.grpc.pb.h>

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

// Logic and data behind the server's behavior.
class LeagueServiceImpl final : public leagueservice::LeagueService::Service {
public:
  // explicit LeagueServiceImpl(const std::string &db) {
  //   filename = db;
  // }

  Status
  CreateUserAccount(ServerContext *context,
                    const leagueservice::CreateUserAccountRequest *request,
                    leagueservice::AuthToken *reply) override {
    reply->set_token("100");
    return Status::OK;
  }

  Status CreateLeague(ServerContext *context,
                      const leagueservice::CreateLeagueRequest *request,
                      leagueservice::CreateLeagueResponse *reply) override {
    return Status::OK;
  }

  Status JoinLeague(ServerContext *context,
                    const leagueservice::JoinLeagueRequest *request,
                    leagueservice::DefaultResponse *reply) override {
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
    return Status::OK;
  }

  Status UpdateLineup(ServerContext *context,
                      const leagueservice::UpdateLineupRequest *request,
                      leagueservice::DefaultResponse *reply) override {
    return Status::OK;
  }

  Status
  GetBasicUserInformation(ServerContext *context,
                          const leagueservice::AuthToken *request,
                          leagueservice::BasicUserInformation *reply) override {
    return Status::OK;
  }

  Status GetMatchup(ServerContext *context,
                    const leagueservice::MatchupRequest *request,
                    leagueservice::MatchupResponse *reply) override {
    return Status::OK;
  }

  Status GetMatch(ServerContext *context,
                  const leagueservice::MatchRequest *request,
                  leagueservice::MatchResponse *reply) override {
    return Status::OK;
  }

  Status GetLineup(ServerContext *context,
                   const leagueservice::LineupRequest *request,
                   leagueservice::LineupResponse *reply) override {
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
    return Status::OK;
  }

  Status
  GetLeaguesForMember(ServerContext *context,
                      const leagueservice::LeaguesForMemberRequest *request,
                      leagueservice::LeaguesForMemberResponse *reply) override {
    return Status::OK;
  }

private:
  std::string filename;
};

int main(int argc, char *argv[]) {
  ServerBuilder builder;
  builder.AddListeningPort("0.0.0.0:50051", grpc::InsecureServerCredentials());

  LeagueServiceImpl my_service;
  builder.RegisterService(&my_service);

  std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
  std::cout << "Built server, now waiting for requests." << std::endl;
  server->Wait();
  return 0;
}