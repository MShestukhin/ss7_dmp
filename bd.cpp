#include "bd.h"
#include <boost/thread.hpp>
#include <string>

BD::BD(std::string dbname, std::string dbhost, std::string dbuser, std::string dbpassword, std::string dbtable, std::string dbschema)
{
    str_dbname=dbname;
    str_dbhost=dbhost;
    str_dbuser=dbuser;
    str_dbpassword=dbpassword;
    str_dbtable=dbtable;
    str_dbschema=dbschema;
    //connect();
}

void BD::connect()
{
    std::string str_connect_to_db="dbname="+str_dbname+" host="+str_dbhost+" user="+str_dbuser+" password="+str_dbpassword;
    conn=PQconnectdb(str_connect_to_db.c_str());
}

int BD::status(){
    if (PQstatus(conn) != CONNECTION_OK)
    {
       // std::cout<<"We were unable to connect to the database\n";
        PQfinish(conn);
        return 1;
    }
    return 0;
}

void BD::INSERT(std::string insert_cmd_str,  char* paramValues[],int num_param)
{
    PGresult* res;
    res=PQexecParams(conn,
                     insert_cmd_str.c_str(),
                     num_param,
                     NULL,
                     paramValues,
                     NULL,
                     NULL,
                     1);
    if (PQresultStatus(res) != PGRES_COMMAND_OK)
    {
        printf("INSERT failed: %s", PQerrorMessage(conn));
        std::cout <<"INSERT failed: "<<PQerrorMessage(conn);
        raise(SIGABRT);
    }
    PQclear(res);
}

void BD::prepare_query_and_insert(std::vector<std::vector<std::string> > massln, std::string table_name, std::vector<std::string>* table_field)
{
    std::string insert_begin="INSERT INTO "+str_dbschema+"."+table_name+" (";
    std::string insert_end=") VALUES(";
    for(int i=0;i<table_field->size();i++)
    {
        insert_begin=insert_begin+table_field->at(i);
        char n[10];
        sprintf(n,"%d",i+1);
        insert_end=insert_end+"$"+n;
        if((i+1)!=table_field->size())
        {
            insert_end+=",";
            insert_begin+=",";
        }
    }
    std::string insert_cmd_str=insert_begin+insert_end+")";
    std::cout<<insert_cmd_str<<"\n";
    for(int j=0;j<massln.size();j++)
    {
        char* paramValues[massln.at(j).size()];
        for(int i=0;i<massln.at(j).size();i++)
            paramValues[i]=(char*)massln.at(j).at(i).c_str();

        INSERT(insert_cmd_str,paramValues,massln.at(0).size());
    }
}

std::string BD::copy(std::vector<std::vector<std::string> > massln,std::string table_str, std::vector<std::string> *table_name)
{
    std::string buffer;
    for(int j=0; j<massln.size();j++)
    {
        std::string row;
        int i=0;
        for(i;i<massln.at(j).size()-1;i++)
            row+=massln.at(j).at(i)+",";
        row+=massln.at(j).at(i);
        buffer+=row+"\n";
    }
   // std::cout<<buffer;

    std::string query_str="copy steer."+table_str+"(";
    int i=0;
    for(i;i<table_name->size()-1;i++)
        query_str+=table_name->at(i)+",";

    query_str+=table_name->at(i)+") FROM STDIN DELIMITER ',';";

  //  std::cout<<query_str;

    PGresult* res;
    res = PQexec(conn, query_str.c_str());
    if (PQresultStatus(res) == PGRES_COPY_IN )
    {
        if(PQputCopyData(conn,(const char*)buffer.c_str(),buffer.size()) == 1)
            {
                if(PQputCopyEnd(conn,NULL) == 1)
                {
                    PGresult *res = PQgetResult(conn);
                    if(PQresultStatus(res) == PGRES_COMMAND_OK)
                        return "Successfully added to the database";
                    else
                        return PQerrorMessage(conn);
                }
                else
                    return PQerrorMessage(conn);
            }
    }
    PQclear(res);
}

void BD::finish()
{
    PQfinish(conn);
}
