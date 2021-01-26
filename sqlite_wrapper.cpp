#include "sqlite_wrapper.hpp"

namespace sqlite_wrapper
{
    const std::string column_data_type[]{"INT", "TEXT", "REAL"};
    const std::string CREATE_TABLE = "CREATE TABLE ";
    const std::string INSERT_INTO = "INSERT INTO ";
    const std::string PRIMARY_KEY = "PRIMARY KEY";
    const std::string NOT_NULL = "NOT NULL";
    const std::string VALUES = "VALUES";
    const std::string SELECT_ALL = "SELECT * FROM ";

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
     * @param value_info Struct which contains colums and data types, and the vector of data rows to be inserted.
     * @returns returns 0 on success, or -1 on error.
    */
    int insert_values(sqlite3 *db, std::string_view table_name, const table_value_info &value_info)
    {
        std::string column_names_string, values_string;

        //construct values tring
        for (auto itr = value_info.value_rows.begin(); itr != value_info.value_rows.end(); ++itr)
        {

            values_string.append("(");
            int column_index = 0;
            for (const auto &column : value_info.column_name_type_map)
            {
                 //construct column names string during the first iteration.
                if (itr == value_info.value_rows.begin()) 
                {
                    column_names_string.append(column.first);
                    if(column_index != value_info.column_name_type_map.size() - 1)
                        column_names_string.append(",");
                }

                if (column.second == COLUMN_DATA_TYPE::TEXT)
                    values_string.append("'");

                values_string.append(itr->at(column_index));

                if (column.second == COLUMN_DATA_TYPE::TEXT)
                    values_string.append("'");

                if (column_index != value_info.column_name_type_map.size() - 1)
                    values_string.append(",");
                
                ++column_index;
            }
            values_string.append(")");

            if (itr != value_info.value_rows.end() - 1)
                values_string.append(",");
        }
        std::string sql;
        sql.append(INSERT_INTO);
        sql.append(table_name);
        sql.append(" (");
        sql.append(column_names_string);
        sql.append(")");
        sql.append(" " + VALUES);
        sql.append(values_string);

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
        sql.append(" (");
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
     * Select values from a table.
     * @param db Pointer to the db.
     * @param table_name Table name to be selected.
     * @param selected_value_map_list Vector of maps which will be populated from the table data.
     * @returns returns 0 on success, or -1 on error.
    */
    int select_values(sqlite3 *db, std::string_view table_name, std::vector<std::map<std::string, std::string>> &selected_value_map_list)
    {
        std::string sql;
        sql.append(SELECT_ALL);
        sql.append(table_name);

        sqlite3_stmt *stmt;

        if (sqlite3_prepare_v2(db, sql.data(), -1, &stmt, NULL) != SQLITE_OK)
            return -1;

        int column_count = sqlite3_column_count(stmt);

        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            std::map<std::string, std::string> selected_value_map;
            for (int i = 0; i < column_count; ++i)
            {
                selected_value_map.emplace(std::string(sqlite3_column_name(stmt, i)), std::string((char *)sqlite3_column_text(stmt, i)));
            }

            selected_value_map_list.insert(selected_value_map_list.end(), selected_value_map);
        }

        return 0;
    }
} // namespace sqlite_wrapper
