#ifndef POSTGRE_SQL_FETCH
#define POSTGRE_SQL_FETCH

#include <memory>
#include <pqxx/pqxx>
#include <string>

namespace fantasy_ball {

// This class will manage a PostgreSQL instance.
class PostgreSQLFetch {
public:
  PostgreSQLFetch() = default;
  ~PostgreSQLFetch();

  // Initializes internal objects, including setting the username, dbname, etc.
  // Returns whether the initialization was successful.
  bool Init();

  // Create the necessary tables if needed.
  void CreateBaseTables(pqxx::work *work);

  // Returns the currently used postgre connection.
  pqxx::connection *GetCurrentConnection();

private:
  std::unique_ptr<pqxx::connection> connection_;

  // Execute commands found in the given filename.
  // NOTE: Call commit() to actually process the work.
  void from_file_exec0(const std::string filename, pqxx::work *work);
};

} // namespace fantasy_ball

#endif // POSTGRE_SQL_FETCH