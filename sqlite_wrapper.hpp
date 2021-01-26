#ifndef _SQLITE_WRAPPER_
#define _SQLITE_WRAPPER_


#include <iostream>
#include <map>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <unordered_map>
#include <vector>

namespace sqlite_wrapper
{
    /**
    * Define an enum and a string array for the column data types.
    * Any column data type that needs to be supportes should be added to both the 'COLUMN_DATA_TYPE' enum and the 'column_data_type' array in its respective order.
    */
    enum COLUMN_DATA_TYPE
    {
        INT,
        TEXT,
        REAL
    };

    /**
     * Struct of table culumn information.
     * {
     *  string name   Name of the column.
     *  column_type   Data type of the column.
     *  is_key        Whether column is a key.
     *  is_null       Whether column is nullable.
     * }
    */
    struct table_column_info
    {
        std::string name;
        COLUMN_DATA_TYPE column_type;
        bool is_key;
        bool is_null;

        table_column_info(const std::string name, const COLUMN_DATA_TYPE column_type, const bool is_key = false, const bool is_null = false)
            : name(std::move(name)), column_type(std::move(column_type)), is_key(std::move(is_key)), is_null(std::move(is_null))
        {
        }
    };

    /**
     * Struct of table data which are to be inserted.
     * {
     *  column_name_type_map    Vector of column name and data type pairs, to which needs to be inserted.
     *  value_rows              Vector of row data corresponding to each of the above columns in string format which is to be insterded.
     * }
    */
    struct table_value_info
    {
        std::vector<std::pair<std::string, COLUMN_DATA_TYPE>> column_name_type_map;
        std::vector<std::vector<std::string>> value_rows;

        table_value_info(std::vector<std::pair<std::string, COLUMN_DATA_TYPE>> column_name_type_map)
            : column_name_type_map(std::move(column_name_type_map))
        {
        }

        int add_row(const std::vector<std::string> &row_values)
        {
            if(column_name_type_map.size() == 0 || row_values.size() != column_name_type_map.size())
                return -1;
            
            value_rows.insert(value_rows.end(), row_values);

            return 0;
        }
    };

    int exec_sql(sqlite3 *db, std::string_view sql, int (*callback)(void *, int, char **, char **), void *callback_first_arg);
    int open_db(std::string_view db_name, sqlite3 **db);
    int create_table(sqlite3 *db, std::string_view table_name, const std::vector<table_column_info> &column_info);
    int insert_values(sqlite3 *db, std::string_view table_name, const table_value_info &value_info);
    int insert_values(sqlite3 *db, std::string_view table_name, std::string_view column_names_string, const std::vector<std::string> &value_strings);
    int select_values(sqlite3 *db, std::string_view table_name, std::vector<std::map<std::string, std::string>> &selected_value_map_list);
} // namespace sqlite_wrapper

#endif