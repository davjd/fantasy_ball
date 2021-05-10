#include <proto/league_service.grpc.pb.h>
#include <proto/league_service.pb.h>

#include <grpc/grpc.h>
#include <grpcpp/create_channel.h>

#include <iostream>

int main(int argc, char *argv[]) {
  // Setup request
  leagueservice::CreateUserAccountRequest req;
  leagueservice::AuthToken result;
  req.set_username("me");

  // Call
  auto channel = grpc::CreateChannel("localhost:50051",
                                     grpc::InsecureChannelCredentials());
  std::unique_ptr<leagueservice::LeagueService::Stub> stub =
      leagueservice::LeagueService::NewStub(channel);
  grpc::ClientContext context;
  grpc::Status status = stub->CreateUserAccount(&context, req, &result);

  // Output result
  std::cout << "I got:" << std::endl;
  std::cout << "Token: " << result.token() << std::endl;

  return 0;
}