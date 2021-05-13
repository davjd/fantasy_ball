#ifndef FANTASY_SERVICE_CLIENT_H_
#define FANTASY_SERVICE_CLIENT_H_

#include <memory>
#include <vector>

#include <grpcpp/channel.h>
#include <proto/league_service.grpc.pb.h>
#include <proto/league_service.pb.h>

namespace fantasy_ball {

// This class is the client of all needed GRPC services.
class FantasyServiceClient {
public:
  FantasyServiceClient(std::shared_ptr<grpc::Channel> channel);
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

  bool JoinLeague(const std::string& token, int league_id);

private:
  std::unique_ptr<leagueservice::LeagueService::Stub> league_stub_;
};

} // namespace fantasy_ball

#endif // FANTASY_SERVICE_CLIENT_H_