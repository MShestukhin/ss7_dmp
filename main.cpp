#include <iostream>
#include <fstream>
#include "parser.h"
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include "include/rapidjson/document.h"
#include "include/rapidjson/reader.h"
#include "include/rapidjson/pointer.h"
#include <boost/thread.hpp>
#include "include/libpq-fe.h"
#include "structs.h"
#include "libconfig.h++"
#include <signal.h>
#include <time.h>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/variant.hpp>
#include "boost/signals2.hpp"
#include <ctime>
#include "CNora.h"
#include "parser.h"
#include "working_file.h"
#include "log.h"
#include "worker_cnora.h"
class log logg;
using namespace std;
using namespace rapidjson;

string coren_socket;
string cnora_name;
int num_row=0;
vector<string> data_for_search; //data from config file
vector<ignor_list> ignor_lists;
int dmp_processing_period=0;
vector<std::string> table_name;
vector<std::string> table_type;
vector<std::string> paths;
bool (*pt2Func)(string,string ) = nullptr;
std::string file_name;
int droped_by_filter;
string dbSchema;
string dbTable;

std::string GetElementValue(const Value& val)
{
  if (val.GetType() == rapidjson::kNumberType)    
    return toString(val.GetInt());
  else if (val.GetType() == rapidjson::kStringType)
    return val.GetString();
  else if (val.GetType() == rapidjson::kArrayType)
    return "Array";
  else if (val.GetType() == rapidjson::kObjectType)
    return "Object";
  return "Unknown";
}

string json_data_find(std::string json_data,std::string jstr,Document& document){
    vector<string> mass_str=split(json_data, ".");
    string str_result;
    for(Value::ValueIterator j=document.Begin(); j!=document.End();j++){
        Value::ConstMemberIterator iter=j->FindMember(mass_str.at(0).c_str());
        if(iter!=(*j).MemberEnd() && iter->value.IsString()){
            if(json_data=="PCAP")
                str_result=str_result+iter->value.GetString()+"\n";
            else
                return iter->value.GetString();
        }
        else if(iter!=(*j).MemberEnd())
        {
            return iter->value.GetObject().FindMember(mass_str.at(1).c_str())->value.GetString();
        }
    }
    return str_result;
}

vector<string> new_pars(string mass_from_js){
    vector<string> data;
    if(!contains(mass_from_js,"RULE")){
        droped_by_filter++;
        data.clear();
        return data;
    }
    for(int i=0;i<data_for_search.size();i++){
        string json_data=data_for_search.at(i);
            data.push_back("0");
    }
    Document document;
    ParseResult ok=document.Parse(mass_from_js.c_str());
    if(!ok){
        logg.error("Json format error at "+toString(num_row));
        data.clear();
        return data;
    }
    for(int i=0;i<data_for_search.size();i++){
        string json_str=data_for_search.at(i);
        string value;
        if(compare_str(json_str,"TS"))
            value=timeStampToString(json_data_find(json_str,mass_from_js, document));
        else
            value=json_data_find(json_str,mass_from_js, document);
//        std::cout<<value<<"\n";
        data.at(i)=value;
    }
    document.Clear();
    return data;
}

void init(){
    droped_by_filter=0;
    //open config file
    libconfig::Config conf;
    try
    {
        conf.readFile("/opt/svyazcom/etc/dmp2db_smsc_lv2.conf");
    }
    catch (libconfig::FileIOException e){
        std::cout<<e.what();
        exit(1);
    }
    catch (libconfig::ConfigException e){
        std::cout<<e.what();
        exit(1);
    }
    catch(libconfig::SettingException e){
        std::cout<<e.what();
        exit(1);
    }
    catch(libconfig::ParseException e)
    {
        std::cout<<e.getError();
        exit(1);
    }
    //load paths
    string str=conf.lookup("application.paths.sourceFile");
    if(contains(str,"*")){
        pt2Func = &contains;
        int begin_or_end=str.find("*");
        if(begin_or_end>0){
            file_name=str.substr(0,str.size()-1);
        }
        else{
            file_name=str.substr(1,str.size()-1);
        }
    }
    else{
        file_name=str;
         pt2Func = &compare_str;
    }

    string str_dmp_timer=conf.lookup("application.timers.lookup_files_timer");
    dmp_processing_period=toNumber(str_dmp_timer);

    paths.push_back(conf.lookup("application.paths.sourceDir"));
    paths.push_back(conf.lookup("application.paths.doneDir"));
    paths.push_back(conf.lookup("application.paths.uploadDir"));
    paths.push_back(conf.lookup("application.paths.logDir"));
    logg.init(paths.at(3));
    string schemaDb=conf.lookup("application.dataBase.schema");
    string tableDb=conf.lookup("application.dataBase.table");
    dbSchema=schemaDb;
    dbTable=tableDb;
    int number_of_table=conf.lookup("application.tableData").getLength();
    for(int i=0;i<number_of_table;i++)    
        table_name.push_back(conf.lookup("application.tableData")[i]);
    for(int i=0;i<number_of_table;i++)
        table_type.push_back(conf.lookup("application.tableTypeData")[i]);

    //load data which should search in json file
    int count_data_for_search=conf.lookup("application.data_for_search").getLength();

    for(int i=0;i<count_data_for_search;i++)    
        data_for_search.push_back(conf.lookup("application.data_for_search")[i]);
    string cor_sock=conf.lookup("application.cnora.coren_socket");
    coren_socket=cor_sock;
    string cn_name=conf.lookup("application.cnora.cnora_name");
    cnora_name=cn_name;
}

boost::asio::io_service io;

void on_signal(const boost::system::error_code& error, int signal_number)
{
    if (error) {
        logg.error("Signal with error :"+error.message());
    }
    logg.error("Got signal "+to_string(signal_number)+":"+strsignal(signal_number));
    exit(1);
}

void iosThread(){
    boost::asio::signal_set signals(io, SIGINT, SIGTERM);
    signals.async_wait(
        boost::bind(&on_signal, _1, _2)
    );
    boost::system::error_code error;
    io.run(error);
    if (error) {
        logg.error("ios.run() error: " + error.message());
    }
    logg.error("ios thread terminated");
}

int main()
{
    init();
    logg.info("Settings accepted");
    boost::thread thread(&iosThread);
    Worker_cnora cnora(io);
    while(1){
        if(io.stopped())
            break;
        sleep(10);
        string format="";
        transport_file(format,paths.at(0),paths.at(2),"dmp",contains);
        vector<Working_file> upload_file=file_lookup(paths.at(2),"dmp",contains);
        for(int i=0; i<upload_file.size();i++){
            string work_file=upload_file.at(i).name;
            logg.info("Getting started with the file "+work_file);
            string s; // сюда будем класть считанные строки
            ifstream file(work_file.c_str());
            vector<vector<std::string> > rows;
            while(getline(file, s)){ // пока не достигнут конец файла класть очередную строку в переменную (s)
                vector<string> row=new_pars(s);
                if(row.size()==table_name.size())
                    rows.push_back(row);
            }
            logg.info(toString(rows.size())+" lines to load into the database");
            cnora.multiple_insertDb(rows,dbSchema,dbTable,table_name,table_type);
            rows.clear();
            file.close();
            std::string str_rm_cmd="mv "+work_file+" "+paths.at(1)+" -f";
            logg.info(str_rm_cmd);
            system(str_rm_cmd.c_str());
        }
    }
    thread.join();
    return 0;
}

