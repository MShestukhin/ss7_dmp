#include <iostream>
#include <libconfig.h++>
#include <fstream>
#include "parser.h"
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include "include/rapidjson/document.h"
#include "include/rapidjson/writer.h"
#include "include/rapidjson/stringbuffer.h"
#include "include/rapidjson/istreamwrapper.h"
#include "include/rapidjson/reader.h"
#include <boost/thread.hpp>
#include </usr/local/pgsql/include/libpq-fe.h>
#include "structs.h"
#include <libconfig.h++>
#include <signal.h>
#include "bd.h"
vector<std::string>* table_name;
using namespace std;
using namespace rapidjson;
PGconn* conn;
bd_data* conf_data; //in which database should connect
vector<string>* data_for_search; //data from config file
BD* bd; //data base worker
vector<vector<std::string> > data_ln; //data which should insert to database

std::string GetElementValue(const Value& val)
{
  if (val.GetType() == rapidjson::kNumberType)
  {
      char str[11];
      int number = val.GetInt();
      sprintf(str, "%d", number);
      return std::string(str);
  }
  else if (val.GetType() == rapidjson::kStringType)
    return val.GetString();
  else if (val.GetType() == rapidjson::kArrayType)
    return "Array";
  else if (val.GetType() == rapidjson::kObjectType)
    return "Object";
  return "Unknown";
}

std::string timeStampToString(string ts) {
      int n=atoi(ts.c_str());
      time_t t = n;
      struct tm * ptm = localtime(&t);
      char buf[30];
      // Format: 2007-10-23 10:45:51
      strftime (buf, 30, "%F %H:%M:%S",  ptm);
      std::string result(buf);
      return result;
}

vector<string> split(string str,char delimitr)
{
    vector<string> mass;
    int pos=0;
    if(str.find(delimitr)!=pos!=string::npos)
    {
        while(pos!=string::npos)
        {
        pos=str.find(delimitr);
        mass.push_back(str.substr(0,pos));
        str.erase(0,pos+1);
        }
    }
    else
    {
        mass.push_back(str);
    }
    return mass;
}

string json_data_find(std::string json_data,std::string jstr)
{
    vector<string> mass_str=split(json_data, '.');
    Document document;
    document.Parse(jstr.c_str());
    Value::ConstMemberIterator iter=document.FindMember(mass_str.at(0).c_str());
    string str_result;
    if(iter!=document.MemberEnd())
    {
        Value& val=document[mass_str.at(0).c_str()];
        for(int i=1; i<mass_str.size();i++)
        {
            Value& obj=val;
            val=obj[mass_str.at(i).c_str()];
        }
        str_result=val.GetString();
    }
    return str_result;
}

void pars(string mass_from_js)
{
    int end_jstr;
    int i=0;
    int data_for_search_iter=0;
    vector<string> data;
    //before bigin pars put all data in vector 0
    for(int i=0;i<data_for_search->size();i++)
    {
        data.push_back("0");
    }
    //find data in json file
    string pcap; //special streeng in which will develop all pcap
    while(data_for_search_iter<data_for_search->size()){
        string testStr=mass_from_js; //string like [{some data,data},{some data:{data,data}},{}]
        while((end_jstr=testStr.find('}'))!=string::npos)
        {
            unsigned int begin_jstr=testStr.find('{'); //begin build object when find {
            i=0;
            while(testStr.at(end_jstr+i)!=',')         //if i find somthing like }, then continue work whie object
            {
            if(testStr.at(end_jstr+i)==']')
                break;
            i++;
            }
            end_jstr=testStr.find('}')+i;
            string jstr=testStr.substr(begin_jstr,end_jstr-1); //this is object somthing like {SCCP:{A:7983...,B:7950...}}
            testStr.erase(begin_jstr,end_jstr);                //delete find object in all string
            string json_data=data_for_search->at(data_for_search_iter); //object from conf file which should i find
            vector<string> mass_str=split(json_data, '.'); //split by point
            Document document;
            document.Parse(jstr.c_str()); //pars json str
            Value::ConstMemberIterator iter=document.FindMember(mass_str.at(0).c_str()); //is object from conf file object which i find from json file
            if(iter!=document.MemberEnd())
            {
                Value& val=document[mass_str.at(0).c_str()]; //try find value of parametr which i need
                for(int i=1; i<mass_str.size();i++)
                {
                    Value& obj=val;
                    val=obj[mass_str.at(i).c_str()];
                }
               // std::cout<<mass_str.at(0);
                if(mass_str.at(0)=="TS") //if object is pcap i should sum str_pcap
                {
                    string TS=val.GetString();
                    TS=timeStampToString(TS);
                    data.at(data_for_search_iter)=TS;
                    break;
                }
                else if(mass_str.at(0)!="PCAP") //if object is pcap i should sum str_pcap
                {
                    data.at(data_for_search_iter)=val.GetString(); //if its not pcap i find object once
                    break;
                }
                else
                {
                    string new_pcap=val.GetString();
                    pcap=pcap+new_pcap.substr(36,string::npos);
                    data.at(data_for_search_iter)=pcap;
                }
            }
        }
        data_for_search_iter++;
    }
    data_ln.push_back(data);
}

void init()
{
    //open config file
    libconfig::Config conf;
    try
    {
        conf.readFile("./json_parser.conf");
    }
    catch(libconfig::ParseException e)
    {
        std::cout<<"Can not open file";
    }
    //load database data from config file
    bd=new BD(
              conf.lookup("application.dataBase.dbname"),
              conf.lookup("application.dataBase.host"),
              conf.lookup("application.dataBase.user"),
              conf.lookup("application.dataBase.password"),
              conf.lookup("application.dataBase.table"),
              conf.lookup("application.dataBase.schema"));
    //load tables data
    table_name=new vector<std::string>;
    int number_of_table=conf.lookup("application.tableData").getLength();
    for(int i=0;i<number_of_table;i++)
    {
        table_name->push_back(conf.lookup("application.tableData")[i]);
    }
    //load data which should search in json file
    data_for_search=new vector<string>;
    int count_data_for_search=conf.lookup("application.data_for_search").getLength();
    for(int i=0;i<count_data_for_search;i++)
    {
        data_for_search->push_back(conf.lookup("application.data_for_search")[i]);
    }
}

void finish_prog_func(int sig){
    bd->finish();
    exit(0);
}

void sig_abort_func(int sig){
    bd->finish();
    exit(0);
}

bool myfunction (file_data i,file_data j) {
    return (i.file_mtime>j.file_mtime);
}

vector<file_data> dmpfile_lookup(string absolute_path)
{
    DIR* dir;
    struct dirent* entry;
    struct stat sb;
    vector<file_data> vdata_file;
    dir=opendir(absolute_path.c_str());
        if(dir==NULL){
            cout<<"Can not open folder ";
            return vdata_file;
        }
        while ((entry=readdir(dir))!=NULL)
        {
            std::string str_file=entry->d_name;
            std::string str_dir_file=absolute_path+"/"+str_file;
            //если в папке есть файлы cмотрим какие
            if(str_file!="."&&str_file!=".."){
                file_data time_name_file;
                stat((char*)str_dir_file.c_str(),&sb);
                //если в папке есть файлы расширения .cdr заносим в буфер
                if(S_ISREG(sb.st_mode)){
                    size_t pos=str_file.rfind(".");
                    std::string ext=str_file.substr(pos, string::npos);
                    if(ext==".cdr"){
                    time_name_file.file_mtime=sb.st_mtim.tv_sec;
                    time_name_file.name=str_dir_file;
                    vdata_file.push_back(time_name_file);
                    }
                }
            }
        }
    //сортируем в порядке времени изменения файла
    std::sort (vdata_file.begin(), vdata_file.end(), myfunction);
    return vdata_file;
}

int main()
{
    init();
    signal(SIGINT, finish_prog_func);
    signal(SIGABRT,sig_abort_func);
    vector<file_data> dmp=dmpfile_lookup("./DMP");
    for(int i=0;i<dmp.size();i++)
    {
        std::cout<<dmp.at(i).name;
    }
    /*FILE * file;
    file = fopen("1.json","r");
    int n;
    vector<string> rows;
    while( !feof(file) )
    {
        char buf[100000];
        fgets(buf,100000,file);
        if(strlen(buf)==0)
        {
            break;
        }
        string str(buf);
        memset(buf,0,100000);
        rows.push_back(str);
    }
    for(int i=0;i<rows.size();i++)
    {
        pars(rows.at(i));
    }

    //bd->prepare_query_and_insert(data_ln,bd->str_dbtable,table_name);
    bd->copy(data_ln,table_name);
    fclose(file);
    bd->finish();*/
    return 0;
}

