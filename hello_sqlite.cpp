#include "sqlite_wrapper.hpp"

/*
static int select_callback(void *selected_values, int argc, char **argv, char **azColName) {
   selected_values;
   std::string values;
   int i;
   for(i = 0; i<argc; i++) {
      values.append( azColName[i]);
      values.append(": ");
      values.append(argv[i]);
      values.append(", ");
   }

   std::cout << values << "\n";
   selected_values = &values;
   std::cout << (char *)selected_values << "\n";

   printf("\n");
   return 0;
}
*/

/*
int select_values(sqlite3 *db, char *zErrMsg, std::string_view table_name, void *callback_first_arg)
{
   std::string sql;
   sql.append(SELECT_ALL);
   sql.append(table_name);

   std::string_view select_operation_success_msg = "Select operation successful";
   return exec_sql(db, zErrMsg, sql, select_operation_success_msg, select_callback, callback_first_arg);
}
*/

int main(int argc, char *argv[])
{
   sqlite3 *db;

   // Open connection to the DB
   if (sqlite_wrapper::open_db("test.db", &db) == -1)
   {
      fprintf(stderr, "null\n");
      return -1;
   }

   /* Create Table */
   std::string_view table_name = "COMPANY";

   std::vector<sqlite_wrapper::table_column_info> column_info;
   // Try to populate vector using {} without insert
   column_info.insert(column_info.end(), sqlite_wrapper::table_column_info("ID", sqlite_wrapper::COLUMN_DATA_TYPE::INT, true));
   column_info.insert(column_info.end(), sqlite_wrapper::table_column_info("NAME", sqlite_wrapper::COLUMN_DATA_TYPE::TEXT));
   column_info.insert(column_info.end(), sqlite_wrapper::table_column_info("AGE", sqlite_wrapper::COLUMN_DATA_TYPE::INT));
   column_info.insert(column_info.end(), sqlite_wrapper::table_column_info("ADDRESS", sqlite_wrapper::COLUMN_DATA_TYPE::TEXT, false, true));
   column_info.insert(column_info.end(), sqlite_wrapper::table_column_info("SALARY", sqlite_wrapper::COLUMN_DATA_TYPE::REAL, false, true));

   if (sqlite_wrapper::create_table(db, table_name, column_info) == -1)
   {
      sqlite3_close(db);
      return -1;
   }
   else
      std::cout << "Table created successfully\n";

   /*Insert values to table, method 1*/
   std::string_view column_names_string = "ID,NAME,AGE,ADDRESS,SALARY";

   std::vector<std::string> value_strings;
   //verify padding is not required at end of string below
   value_strings.insert(value_strings.end(), "1, 'Paul', 32, 'California', 20000.00 ");
   value_strings.insert(value_strings.end(), "2, 'Allen', 25, 'Texas', 15000.00 ");
   value_strings.insert(value_strings.end(), "3, 'Teddy', 23, 'Norway', 20000.00 ");
   value_strings.insert(value_strings.end(), "4, 'Mark', 25, 'Rich-Mond ', 65000.00 ");

   if (sqlite_wrapper::insert_values(db, table_name, column_names_string, value_strings) == -1)
   {
      sqlite3_close(db);
      return -1;
   }
   else
      std::cout << "Records added successfully\n";

   /*Insert values to table, method 2*/
   std::vector<std::pair<std::string, sqlite_wrapper::COLUMN_DATA_TYPE>> column_name_type_map;
   column_name_type_map.insert(column_name_type_map.end(), std::pair<std::string, sqlite_wrapper::COLUMN_DATA_TYPE>("ID", sqlite_wrapper::COLUMN_DATA_TYPE::INT));
   column_name_type_map.insert(column_name_type_map.end(), std::pair<std::string, sqlite_wrapper::COLUMN_DATA_TYPE>("NAME", sqlite_wrapper::COLUMN_DATA_TYPE::TEXT));
   column_name_type_map.insert(column_name_type_map.end(), std::pair<std::string, sqlite_wrapper::COLUMN_DATA_TYPE>("AGE", sqlite_wrapper::COLUMN_DATA_TYPE::INT));
   column_name_type_map.insert(column_name_type_map.end(), std::pair<std::string, sqlite_wrapper::COLUMN_DATA_TYPE>("ADDRESS", sqlite_wrapper::COLUMN_DATA_TYPE::TEXT));
   column_name_type_map.insert(column_name_type_map.end(), std::pair<std::string, sqlite_wrapper::COLUMN_DATA_TYPE>("SALARY", sqlite_wrapper::COLUMN_DATA_TYPE::REAL));

   sqlite_wrapper::table_value_info table_value_info(column_name_type_map);

   table_value_info.add_row(std::vector<std::string>{"5", "James", "31", "London", "23000"});
   table_value_info.add_row(std::vector<std::string>{"6", "Jeremy", "33", "Westhampshire", "25000"});
   table_value_info.add_row(std::vector<std::string>{"7", "RIchard", "28", "Wales", "23000"});

   if (sqlite_wrapper::insert_values(db, table_name, table_value_info) == -1)
   {
      sqlite3_close(db);
      return -1;
   }
   else
      std::cout << "Records added successfully\n";

   /*Select values*/
   std::vector<std::map<std::string, std::string>> selected_value_map_list;

   if (sqlite_wrapper::select_values(db, table_name, selected_value_map_list) == -1)
   {
      sqlite3_close(db);
      return -1;
   }

   /*Print selected values*/
   for (const auto &selected_value_map : selected_value_map_list)
   {
      for (const auto &value : selected_value_map)
      {
         std::cout << "column name: " << value.first << ", value: " << value.second << "\n";
      }
   }

   // Close the connection.
   sqlite3_close(db);
   return 0;
}