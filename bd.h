#ifndef BD_H
#define BD_H
#include <string>
#include <vector>
#include </usr/local/pgsql/include/libpq-fe.h>
#include <signal.h>
#include <iostream>
class BD
{
public:
    std::string str_dbname;
    std::string str_dbhost;
    std::string str_dbuser;
    std::string str_dbpassword;
    std::string str_dbtable;
    std::string str_dbschema;
    PGconn* conn;
    BD(std::string dbname,
       std::string dbhost,
       std::string dbuser,
       std::string dbpassword,
       std::string dbtable,
       std::string dbschema);
    void connect();
    void INSERT(std::string insert_cmd_str,  char* paramValues[],int num_param);
    void prepare_query_and_insert(std::vector<std::vector<std::string> > massln, std::string table_name, std::vector<std::string>* table_field);
    void finish();
    void copy(std::vector<std::vector<std::string> > massln,std::vector<std::string>* table_name);
};

#endif // BD_H
