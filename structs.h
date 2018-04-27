#ifndef STRUCTS_H
#define STRUCTS_H
#include <iostream>
// структура с полями: дата, имзи, три номера, длительность звонка в сек и результат
struct line
{
    char* date;
    char* imsi;
    char* number;
    char* number_2;
    char* number_3;
    char* call_duration;
    char* res;
    char* resKey;
    line() {
        date="";
        imsi="";
        number="";
        number_2="";
        number_3="";
        call_duration="";
        res="";
        resKey="";
    }
};

struct file_data
{
    int file_mtime;
    std::string name;
    file_data() {
        file_mtime=0;
        name="";
    }
};

struct Data_from_conf_file{
    std::string str_dir;
    std::string str_trash_dir;
    std::string str_workOut_dir;
    std::string str_dbname;
    std::string str_dbhost;
    std::string str_dbuser;
    std::string str_dbpassword;
    std::string str_dbtable;
    std::string str_dbschema;
    std::string str_db_for_files_table;
    Data_from_conf_file(
              std::string str,
              std::string trash,
              std::string workOut,
              std::string dbname,
              std::string dbhost,
              std::string dbuser,
              std::string dbpassword,
              std::string dbtable,
              std::string dbschema,
              std::string dbFFtable
              )
    {
    str_dir=str;
    str_trash_dir=trash;
    str_workOut_dir=workOut;
    str_dbname=dbname;
    str_dbhost=dbhost;
    str_dbuser=dbuser;
    str_dbpassword=dbpassword;
    str_dbtable=dbtable;
    str_dbschema=dbschema;
    str_db_for_files_table=dbFFtable;
    };
};

struct bd_data
{
    std::string str_dbname;
    std::string str_dbhost;
    std::string str_dbuser;
    std::string str_dbpassword;
    std::string str_dbtable;
    std::string str_dbschema;
    bd_data(std::string dbname,
            std::string dbhost,
            std::string dbuser,
            std::string dbpassword,
            std::string dbtable,
            std::string dbschema)
    {
        str_dbname=dbname;
        str_dbhost=dbhost;
        str_dbuser=dbuser;
        str_dbpassword=dbpassword;
        str_dbtable=dbtable;
        str_dbschema=dbschema;
    }
};

struct Table_conf_data{
    std::string timestamp;
    std::string imsi;
    std::string number_1;
    std::string number_2;
    std::string number_3;
    std::string duration;
    std::string result;
    std::string result_key;
    Table_conf_data(std::string time,
                    std::string ims,
                    std::string num1,
                    std::string num2,
                    std::string num3,
                    std::string dur,
                    std::string res,
                    std::string key){
        timestamp=time;
        imsi=ims;
        number_1=num1;
        number_2=num2;
        number_3=num3;
        duration=dur;
        result=res;
        result_key=key;
    }
};

#endif // STRUCTS_H
