#include "postgre_sql_fetch.h"

#include <fstream>
#include <iostream>
#include <memory>
#include <pqxx/pqxx>
#include <string>

#include "util.h"

namespace fantasy_ball {

PostgreSQLFetch::~PostgreSQLFetch() {}

bool PostgreSQLFetch::Init() {
  try {
    const std::string config = postgre::read_postgre_config();
    connection_ = std::make_unique<pqxx::connection>(config);
    return true;
  } catch (const std::exception &e) {
    return false;
  }
}

void PostgreSQLFetch::CreateBaseTables(pqxx::work *work) {
  try {
    // Since the tables may reference one another with their foreign keys, we
    // need to create the tables in a specific order.
    // Create the user profile and user account tables.
    from_file_exec0("create_profile_description.sql", work);
    from_file_exec0("create_user_account.sql", work);
    from_file_exec0("create_user_auth_token.sql", work);
    // Create the draft tables.
    from_file_exec0("create_draft.sql", work);
    from_file_exec0("create_draft_selection.sql", work);
    // Create the settings tables.
    from_file_exec0("create_waiver_settings.sql", work);
    from_file_exec0("create_transaction_settings.sql", work);
    from_file_exec0("create_league_settings.sql", work);
    // Create the league table and standing table.
    from_file_exec0("create_league.sql", work);
    from_file_exec0("create_league_membership.sql", work);
    from_file_exec0("create_roster_member.sql", work);
    from_file_exec0("create_standing.sql", work);
    // Create the matchup table along with the match and lineup tables.
    from_file_exec0("create_matchup.sql", work);
    from_file_exec0("create_match.sql", work);
    from_file_exec0("create_lineup_slot.sql", work);
  } catch (std::exception const &e) {
    std::cerr << e.what() << std::endl;
  }
}

void PostgreSQLFetch::DeleteBaseTables(pqxx::work *work) {
  try {
    from_file_exec0("delete_base_tables.sql", work);
  } catch (std::exception const &e) {
    std::cerr << e.what() << std::endl;
  }
}

pqxx::connection *PostgreSQLFetch::GetCurrentConnection() {
  return connection_.get();
}

void PostgreSQLFetch::from_file_exec0(const std::string filename,
                                      pqxx::work *work) {
  const auto &sql_commands = postgre::get_commands_from_file(filename);
  for (const std::string &command : sql_commands) {
    work->exec0(command);
  }
}
} // namespace fantasy_ball