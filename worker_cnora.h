#ifndef WORKER_CNORA_H
#define WORKER_CNORA_H

#include <vector>
#include <iostream>
#include "CNora.h"
using namespace std;
class Worker_cnora
{
   CoreN::InterfacePtr I;
public:
    string db_schema;
    string db_table;
    string socket;
    vector<string> table_name;
    vector<string> table_type;
    Worker_cnora(boost::asio::io_service& io);
    void stop();
    void insertDb(vector<vector<string>> data_ln, string db_schema, string db_table, vector<string> table_name, vector<string> table_type);
    int multiple_insertDb(vector<vector<string>> data_ln, string db_schema, string db_table, vector<string> table_name, vector<string> table_type);
};

#endif // WORKER_CNORA_H
