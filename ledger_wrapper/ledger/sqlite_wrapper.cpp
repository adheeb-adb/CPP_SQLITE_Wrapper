#include "sqlite_wrapper.hpp"

namespace ledger::sqlite_wrapper
{
    const std::string ledger_table = "ledger";
    const std::string ledger_columns = "seq_no, time, ledger_hash, prev_ledger_hash, data_hash, state_hash, patch_hash, user_hash, input_hash, output_hash";
    const std::string column_data_type[]{"INT", "TEXT"};
    const std::string CREATE_TABLE = "CREATE TABLE ";
    const std::string INSERT_INTO = "INSERT INTO ";
    const std::string PRIMARY_KEY = "PRIMARY KEY";
    const std::string NOT_NULL = "NOT NULL";
    const std::string VALUES = "VALUES";
    const std::string SELECT_ALL = "SELECT * FROM ";

    /**
     * Opens a connection to a given databse and give the db pointer.
     * @param db_name Database name to be connected.
     * @param db Pointer to the db pointer which is to be connected and ponted.
     * @returns returns 0 on success, or -1 on error.
    */
    int open_db(std::string_view db_name, sqlite3 **db)
    {
        if (sqlite3_open(db_name.data(), db) != SQLITE_OK)
        {
            std::cout << "Can't open database: " << sqlite3_errmsg(*db) << "\n";
            return -1;
        }
        return 0;
    }

    /**
     * Executes given sql query.
     * @param db Pointer to the db.
     * @param sql Sql query to be executed.
     * @param callback Callback funcion which is called for each result row.
     * @param callback_first_arg First data argumat to be parced to the callback (void pointer).
     * @returns returns 0 on success, or -1 on error.
    */
    int exec_sql(sqlite3 *db, std::string_view sql, int (*callback)(void *, int, char **, char **) = NULL, void *callback_first_arg = NULL)
    {
        char *zErrMsg;
        if (sqlite3_exec(db, sql.data(), callback, (callback != NULL ? (void *)callback_first_arg : NULL), &zErrMsg) != SQLITE_OK)
        {
            fprintf(stderr, "SQL error: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
            return -1;
        }
        return 0;
    }

    /**
     * Create a table with given table info.
     * @param db Pointer to the db.
     * @param table_name Table name to be created.
     * @param column_info Column info of the table.
     * @returns returns 0 on success, or -1 on error.
    */
    int create_table(sqlite3 *db, std::string_view table_name, const std::vector<table_column_info> &column_info)
    {
        std::string sql;
        sql.append(CREATE_TABLE);
        sql.append(table_name);
        sql.append(" (");

        for (auto itr = column_info.begin(); itr != column_info.end(); ++itr)
        {
            sql.append(itr->name + " " + column_data_type[itr->column_type]);

            if (itr->is_key)
                sql.append(" " + PRIMARY_KEY);

            if (!itr->is_null)
                sql.append(" " + NOT_NULL);

            if (itr != column_info.end() - 1)
                sql.append(",");
        }
        sql.append(")");

        /* Execute SQL statement */
        return exec_sql(db, sql);
    }

    /**
     * Insert values to a table.
     * @param db Pointer to the db.
     * @param table_name Table name to be populated.
     * @param column_names_string Comma seperated string of colums (eg: "col_1,col_2,...").
     * @param value_strings Vector of comma seperated values (wrap in single quotes for TEXT type) (eg: ["r1val1,'r1val2',...", "r2val1,'r2val2',..."]).
     * @returns returns 0 on success, or -1 on error.
    */
    int insert_values(sqlite3 *db, std::string_view table_name, std::string_view column_names_string, const std::vector<std::string> &value_strings)
    {
        std::string sql;

        sql.append(INSERT_INTO);
        sql.append(table_name);
        sql.append("(");
        sql.append(column_names_string);
        sql.append(")");
        sql.append(" " + VALUES);

        for (auto itr = value_strings.begin(); itr != value_strings.end(); ++itr)
        {
            sql.append("(");
            sql.append(*itr);
            sql.append(")");

            if (itr != value_strings.end() - 1)
                sql.append(",");
        }

        /* Execute SQL statement */
        return exec_sql(db, sql);
    }

    /**
     * Insert values to a table.
     * @param db Pointer to the db.
     * @param table_name Table name to be populated.
     * @param column_names_string Comma seperated string of colums (eg: "col_1,col_2,...").
     * @param value_strings Vector of comma seperated values (wrap in single quotes for TEXT type) (eg: ["r1val1,'r1val2',...", "r2val1,'r2val2',..."]).
     * @returns returns 0 on success, or -1 on error.
    */
    int insert_value(sqlite3 *db, std::string_view table_name, std::string_view column_names_string, std::string_view value_string)
    {
        std::string sql;
        sql.reserve(INSERT_INTO.size() + table_name.size() + column_names_string.size() + VALUES.size() + value_string.size() + 5);

        sql.append(INSERT_INTO);
        sql.append(table_name);
        sql.append("(");
        sql.append(column_names_string);
        sql.append(")");
        sql.append(" " + VALUES);
        sql.append("(");
        sql.append(value_string);
        sql.append(")");

        /* Execute SQL statement */
        return exec_sql(db, sql);
    }

    /**
     * Creates a table for ledger records.
     * @param db Pointer to the db.
     * @returns returns 0 on success, or -1 on error.
    */
    int create_ledger_table(sqlite3 *db)
    {
        std::vector<sqlite_wrapper::table_column_info> column_info{
            sqlite_wrapper::table_column_info("seq_no", sqlite_wrapper::COLUMN_DATA_TYPE::INT, true),
            sqlite_wrapper::table_column_info("time", sqlite_wrapper::COLUMN_DATA_TYPE::INT),
            sqlite_wrapper::table_column_info("ledger_hash", sqlite_wrapper::COLUMN_DATA_TYPE::TEXT),
            sqlite_wrapper::table_column_info("prev_ledger_hash", sqlite_wrapper::COLUMN_DATA_TYPE::TEXT),
            sqlite_wrapper::table_column_info("data_hash", sqlite_wrapper::COLUMN_DATA_TYPE::TEXT),
            sqlite_wrapper::table_column_info("state_hash", sqlite_wrapper::COLUMN_DATA_TYPE::TEXT),
            sqlite_wrapper::table_column_info("patch_hash", sqlite_wrapper::COLUMN_DATA_TYPE::TEXT),
            sqlite_wrapper::table_column_info("user_hash", sqlite_wrapper::COLUMN_DATA_TYPE::TEXT),
            sqlite_wrapper::table_column_info("input_hash", sqlite_wrapper::COLUMN_DATA_TYPE::TEXT),
            sqlite_wrapper::table_column_info("output_hash", sqlite_wrapper::COLUMN_DATA_TYPE::TEXT)};

        if (sqlite_wrapper::create_table(db, ledger_table, column_info) == -1)
            return -1;
        
        return 0;
    }

    /**
     * Inserts a ledger record.
     * @param db Pointer to the db.
     * @param ledger Ledger struct to beinserted.
     * @returns returns 0 on success, or -1 on error.
    */
    int insert_ledger_row(sqlite3 *db, const ledger &ledger)
    {
        std::string value_string;
        value_string.append(std::to_string(ledger.seq_no) + ",");
        value_string.append(std::to_string(ledger.time) + ",");
        value_string.append(wrap_in_single_quote(ledger.ledger_hash) + ",");
        value_string.append(wrap_in_single_quote(ledger.prev_ledger_hash) + ",");
        value_string.append(wrap_in_single_quote(ledger.data_hash) + ",");
        value_string.append(wrap_in_single_quote(ledger.state_hash) + ",");
        value_string.append(wrap_in_single_quote(ledger.patch_hash) + ",");
        value_string.append(wrap_in_single_quote(ledger.user_hash) + ",");
        value_string.append(wrap_in_single_quote(ledger.input_hash) + ",");
        value_string.append(wrap_in_single_quote(ledger.output_hash));

        // if (sqlite_wrapper::insert_values(db, ledger_table, ledger_columns, std::vector<std::string>{value_string}) == -1)
        //     return -1;
        if (sqlite_wrapper::insert_value(db, ledger_table, ledger_columns, value_string) == -1)
            return -1;
        
        return 0;
    }

    /**
     * Wraps a given string with quoted.
     * @param value String value to be quoted.
     * @returns returns quoted string.
    */
    std::string wrap_in_single_quote(std::string_view value)
    {
        std::string s;
        s.reserve(value.size() + 2);

        s.append("'");
        s.append(value);
        s.append("'");

        return s;
    }
} // namespace ledger::sqlite_wrapper