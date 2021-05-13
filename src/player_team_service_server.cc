#include <iostream>
#include <memory>
#include <string>

#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

#include <proto/player_team_service.grpc.pb.h>

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

class PlayerTeamServiceImpl final
    : public playerteamservice::PlayerTeamService::Service {
public:
  //   explicit PlayerTeamServiceImpl(fantasy_ball::LeagueFetcher
  //   *league_fetcher) {
  //     league_fetcher_ = league_fetcher;
  //   }

  Status AddPlayerToFetch(ServerContext *context,
                          const playerteamservice::AddPlayerRequest *request,
                          playerteamservice::DefaultResponse *reply) override {
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
    return Status::OK;
  }

private:
  //   fantasy_ball::LeagueFetcher *league_fetcher_;
};

// Helper to initialize the Postgre database.
// bool InitDB(fantasy_ball::PostgreSQLFetch *psql_fetch,
//             bool delete_tables = false) {
//   bool db_init_success = psql_fetch->Init();
//   if (!db_init_success) {
//     std::cout << "Couldn't initialize PostgreSQL database." << std::endl;
//     return false;
//   }
//   auto *connection = psql_fetch->GetCurrentConnection();
//   pqxx::work W{*connection};
//   if (delete_tables) {
//     psql_fetch->DeleteBaseTables(&W);
//   }
//   psql_fetch->CreateBaseTables(&W, true);
//   W.commit();
//   return true;
// }

int main(int argc, char *argv[]) {
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