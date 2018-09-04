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
#include <libconfig.h++>
#include <signal.h>
#include "bd.h"
#include <time.h>
#include <boost/asio.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/keywords/format.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/thread.hpp>
#include <boost/variant.hpp>
#include "boost/signals2.hpp"
#include <ctime>
#include "CNora.h"
namespace logging = boost::log;
namespace sinks = boost::log::sinks;
namespace src = boost::log::sources;
namespace expr = boost::log::expressions;
namespace attrs = boost::log::attributes;
namespace keywords = boost::log::keywords;
using namespace logging::trivial;
using namespace std;
using namespace rapidjson;
int num_row=0;
PGconn* conn;
bd_data* conf_data; //in which database should connect
vector<string>* data_for_search; //data from config file
BD* bd; //data base worker
vector<ignor_list> ignor_lists;
int dmp_processing_period;
int db_reconnect_period;
src::severity_logger< severity_level > lg;
vector<std::string>* table_name;
vector<std::string>* paths;
bool (*pt2Func)(string,string ) = NULL;
std::string file_name;
int droped_by_filter;
string dbSchema;
string dbTable;
CoreN::InterfacePtr getCoreN(boost::asio::io_service& ios)
{
    const std::string& unix_socket_name = "/opt/svyazcom/var/run/coren.sock";
    unsigned int reconnect_tmo = 5;
    return CoreN::Interface::Create(ios, unix_socket_name, reconnect_tmo);
}

CoreN::CNoraPtr getCNora(const CoreN::InterfacePtr& I)
{
    const CoreN::Service::Address remote {
        "CNORA", "", 0
    };
    const boost::chrono::milliseconds timeout {
        1000
    };
    CoreN::CNoraPtr svc = boost::make_shared<CoreN::CNora>(I, remote, timeout);
    I->AddService(CoreN::cnora_label, boost::dynamic_pointer_cast<CoreN::Service::Base>(svc));
    return svc;
}

boost::asio::io_service io;
const CoreN::InterfacePtr& I = getCoreN(io);
const CoreN::CNoraPtr& cnora=getCNora(I);
int toNumber(std::string str){
    return atoi(str.c_str());
}

void rebind(boost::function<void()>& f, const boost::function<void(const boost::function<void()>& handler)>& new_one)
{
    boost::function<void()> origin;
    boost::swap(origin, f);
    f = [new_one, origin]() {
        new_one(origin ? origin : [](){});
    };
}

std::string toString(int number)
{
    char str[11];
    sprintf(str, "%d", number);
    return std::string(str);
}

bool contains(std::string s_cel,std::string s_find){
    if (s_cel.find(s_find) != std::string::npos) {
        return true;
    }
    return false;
}

bool compare_str(string str_first,string str_second ){
    if(str_first==str_second)
        return true;
    else
        return false;
}

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

std::string timeStampToString(string ts)
{
    int n=atoi(ts.c_str());
    time_t t = n;
    struct tm * ptm = localtime(&t);
    char buf[30];
    //Format: 2007-10-23 10:45:51
    strftime (buf, 30, "%F %H:%M:%S",  ptm);
    std::string result(buf);
    return result;
}

vector<string> split(string str,const char * delimitr)
{
    //usleep(100);
    std::vector<string> v;
    char* chars_array = strtok((char*)str.c_str(), delimitr);
    while(chars_array)
    {
        v.push_back(chars_array);
        chars_array = strtok(NULL, delimitr);
    }
    return v;
//    vector<string> mass;
//    int pos=0;
//    int end=0;
//    if(str.find(delimitr)!=pos!=string::npos)
//        while(pos!=string::npos)
//        {
//            pos=str.find(delimitr);
//            mass.push_back(str.substr(end,pos));
//            str.erase(0,pos+1);
//        }
//    else
//        mass.push_back(str);
//    return mass;
}

string json_data_find(std::string json_data,std::string jstr)
{
    vector<string> mass_str=split(json_data, ".");
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

int processing_ignor_list(string jstr)
{
    Document document;
    document.Parse(jstr.c_str());
    for(int i=0;i<ignor_lists.size();i++)
    {
        vector<string> mass_str=split(ignor_lists.at(i).name, ".");
        Value::ConstMemberIterator iter=document.FindMember(mass_str.at(0).c_str()); //is object from conf file object which i find from json file
        if(iter!=document.MemberEnd())
        {
            Value& val=document[mass_str.at(0).c_str()]; //try find value of parametr which i need
            for(int t=1; t<mass_str.size();t++)
            {
                Value& obj=val;
                val=obj[mass_str.at(t).c_str()];
            }
            for(int j=0;j<ignor_lists.at(i).values.size();j++)
            {
                string v=ignor_lists.at(i).values.at(j);
                std::string vall=GetElementValue(val);
                if(v==vall)
                    return 1;
            }
        }
    }
    return 0;
}

vector<string> pars(string mass_from_js)
{
    num_row++;
    vector<string> data;
    if(!contains(mass_from_js,"RULE")){
        droped_by_filter++;
        data.clear();
        return data;
    }
    for(int i=0;i<data_for_search->size();i++){
        string json_data=data_for_search->at(i);
        if(contains(json_data,"{:")){
            int str_begin=json_data.find(":");
            int str_end=json_data.find("}");
            data.push_back(json_data.substr(str_begin+1,str_end-str_begin-1));
        }
        else{
            data.push_back("0");
        }
    }
    int data_for_search_iter=0;
    Document document;
    ParseResult ok=document.Parse(mass_from_js.c_str());
    if(!ok){
        BOOST_LOG_SEV(lg, error) <<"Json format error at "<<num_row;
        data.clear();
        return data;
    }
    while(data_for_search_iter<data_for_search->size()){
        std::string default_value=data.at(data_for_search_iter);
        if(contains(data_for_search->at(data_for_search_iter),"MAP.VLR")){
            document.Clear();
            document.Parse(mass_from_js.c_str());
        }
        string pcap;
        for(Value::ValueIterator iter=document.Begin(); iter!=document.End();iter++){
            string json_str=data_for_search->at(data_for_search_iter); //object from conf file which should i find
            vector<string> mass_or_str=split(json_str,"/");
            vector<string> obj_str;
            string json_data;
            for(int i=0;i<mass_or_str.size();i++){
                json_data=mass_or_str.at(i);
                obj_str=split(json_data, "."); //split by point
                Value::ConstMemberIterator j=(*iter).FindMember(obj_str.at(0).c_str());
                if(j!=(*iter).MemberEnd()){
                    Value& val=(*iter)[obj_str.at(0).c_str()];
                    for(int j=1;j<obj_str.size();j++){
                        Value& obj=val;
                        Value::ConstMemberIterator jtr=val.FindMember(obj_str.at(j).c_str());
                        if(jtr!=val.MemberEnd())
                            val=obj[obj_str.at(j).c_str()];
                        else
                            break;
                    }
                    if(obj_str.at(0)=="TS")
                    {
                        string TS=val.GetString();
                        TS=timeStampToString(TS);
                        data.at(data_for_search_iter)=TS;
                        break;
                    }
                    if(obj_str.at(0)!="PCAP") //if object is pcap i should sum str_pcap
                    {
                        std::string val_str1=val.GetString();
                        if(val_str1.size()!=0)
                            data.at(data_for_search_iter)=val_str1; //if its not pcap i find object once
                        break;
                    }
                    else
                    {
                        string new_pcap=val.GetString();
                        pcap=pcap+new_pcap+"\\n";
                        data.at(data_for_search_iter)=pcap;
                    }
                }
             }
            if(data.at(data_for_search_iter)!=default_value&&obj_str.at(0)!="PCAP")
                break;
            }
        data_for_search_iter++;
        }
    return data;
}

bool myfunction (file_data i,file_data j)
{
    return (i.file_mtime>j.file_mtime);
}

vector<file_data> dmpfile_lookup(string absolute_path,bool (*pt2Func)(string,string )){
    DIR* dir;
    struct dirent* entry;
    struct stat sb;
    vector<file_data> vdata_file;
    dir=opendir(absolute_path.c_str());
    if(dir==NULL)
    {
        BOOST_LOG_SEV(lg, error) <<"Can not open folder ";
        return vdata_file;
    }
    while ((entry=readdir(dir))!=NULL)
    {
        std::string str_file=entry->d_name;
        std::string str_dir_file=absolute_path+"/"+str_file;
        //lookup some file in dir
        if(str_file!="."&&str_file!="..")
        {
            file_data time_name_file;
            stat((char*)str_dir_file.c_str(),&sb);
            //if there are files dmp
            if(S_ISREG(sb.st_mode)&&(*pt2Func)(str_file,file_name))
            {
                time_name_file.file_mtime=sb.st_mtim.tv_sec;
                time_name_file.name=str_dir_file;
                vdata_file.push_back(time_name_file);
            }
        }
    }
    closedir(dir);
    //сортируем в порядке времени изменения файла
    std::sort (vdata_file.begin(), vdata_file.end(), myfunction);
    return vdata_file;
}

vector<file_data> uploadfile_lookup(string absolute_path){
    DIR* dir;
    struct dirent* entry;
    struct stat sb;
    vector<file_data> vdata_file;
    dir=opendir(absolute_path.c_str());
    if(dir==NULL)
    {
        BOOST_LOG_SEV(lg, error) <<"Can not open folder ";
        return vdata_file;
    }
    while ((entry=readdir(dir))!=NULL)
    {        
        std::string str_file=entry->d_name;
        std::string str_dir_file=absolute_path+"/"+str_file;
        //lookup some file in dir
        if(str_file!="."&&str_file!="..")
        {
            file_data time_name_file;
            stat((char*)str_dir_file.c_str(),&sb);
            //if there are files dmp
            if(contains(str_file,file_name))
            {
                time_name_file.file_mtime=sb.st_mtim.tv_sec;
                time_name_file.name=str_dir_file;
                vdata_file.push_back(time_name_file);
            }
        }
    }
    closedir(dir);
    //сортируем в порядке времени изменения файла
    std::sort (vdata_file.begin(), vdata_file.end(), myfunction);
    return vdata_file;
}


void finish_prog_func(int sig){
    bd->finish();
    exit(0);
}

void sig_abort_func(int sig){
    BOOST_LOG_SEV(lg, error) <<"Abnormal shutdown";
    bd->finish();
    exit(1);
}

void init()
{
    droped_by_filter=0;
    //open config file
    libconfig::Config conf;
    try
    {
        //conf.readFile("/opt/svyazcom/etc/dmp2db_smsc_lv2.conf"); //opt/svyazcom/etc/dmp_sca.conf
        conf.readFile("./dmp2db_smsc_lv2.conf");
    }
    catch (libconfig::FileIOException e){
        BOOST_LOG_SEV(lg, error)<<e.what();
        exit(1);
    }
    catch (libconfig::ConfigException e){
        BOOST_LOG_SEV(lg, error)<<e.what();
        exit(1);
    }
    catch(libconfig::SettingException e){
        BOOST_LOG_SEV(lg, error)<<e.what();
        exit(1);
    }
    catch(libconfig::ParseException e)
    {
        BOOST_LOG_SEV(lg, error)<<e.getError();
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
    string str_db_reconect_period=conf.lookup("application.timers.reconect_db_timer");
    db_reconnect_period=toNumber(str_db_reconect_period);
    paths=new vector<string>;
    paths->push_back(conf.lookup("application.paths.sourceDir"));
    paths->push_back(conf.lookup("application.paths.doneDir"));
    paths->push_back(conf.lookup("application.paths.uploadDir"));
    paths->push_back(conf.lookup("application.paths.logDir"));
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
        table_name->push_back(conf.lookup("application.tableData")[i]);

    //load data which should search in json file
    data_for_search=new vector<string>;
    int count_data_for_search=conf.lookup("application.data_for_search").getLength();

    for(int i=0;i<count_data_for_search;i++)    
        data_for_search->push_back(conf.lookup("application.data_for_search")[i]);

    int ignor_list_size=conf.lookup("application.ignor_list").getLength();

    for (int i=0;i<ignor_list_size;i++)
    {
        ignor_list list;
        string conf_str="application.ignor_list.obj"+toString(i)+".name";
        string str_name=conf.lookup(conf_str);
        list.name=str_name;
        conf_str="application.ignor_list.obj"+toString(i)+".values";
        int ignor_list_num_values=conf.lookup(conf_str).getLength();
        for(int j=0;j<ignor_list_num_values;j++)
        {
            string str_v=conf.lookup(conf_str)[j];
            list.values.push_back(str_v);
        }
        ignor_lists.push_back(list);
    }
//        logging::add_file_log
//        (
//            keywords::file_name =paths->at(3)+"/%Y-%m-%d.log",
//                    keywords::auto_flush = true ,
//            keywords::time_based_rotation = sinks::file::rotation_at_time_point(0, 0, 0),
//            keywords::format =
//            (
//                expr::stream
//                << expr::format_date_time< boost::posix_time::ptime >("TimeStamp", "%Y-%m-%d %H:%M:%S")
//                << "\t: <" << logging::trivial::severity
//                << "> \t" << expr::smessage
//            )
//        );
}

void transport_dmp_to_upload(){
    vector<file_data> dmp=dmpfile_lookup(paths->at(0),pt2Func);
    for(int i=0;i<dmp.size();i++)
    {
        string absolute_path_to_file=dmp.at(i).name;
        BOOST_LOG_SEV(lg, info) <<"File detected "<<absolute_path_to_file;
        char buffer[80];
        time_t seconds = time(NULL);
        tm* timeinfo = localtime(&seconds);
        const char* format = "%B_%d_%Y_%I:%M:%S";
        strftime(buffer, 80, format, timeinfo);
        string work_file=paths->at(2)+"/"+std::string(buffer)+absolute_path_to_file.substr(absolute_path_to_file.rfind("/")+1,string::npos);
        std::string str_copy_result_cmd="mv "+absolute_path_to_file+" "+work_file+" -f";
        BOOST_LOG_SEV(lg, info) <<str_copy_result_cmd;
        system(str_copy_result_cmd.c_str());
    }
}

struct output : public boost::static_visitor<>
{
  template <typename T>
  void operator()(T t) const { std::cout << t << '\n'; }
};

class my_visitor : public boost::static_visitor<int>
{
    public:
    int operator()(int i) const
    {
        return i;
    }

    int operator()(const std::string & str) const
    {
        return str.length();
    }
};


void mainThread(){

    while(1){
        sleep(dmp_processing_period);
        transport_dmp_to_upload();
        vector<file_data> upload_file=uploadfile_lookup(paths->at(2));
        for(int i=0; i<upload_file.size();i++){
            string work_file=upload_file.at(i).name;
            BOOST_LOG_SEV(lg, info) <<"Getting started with the file "<<work_file;
            FILE * file;
            file = fopen(work_file.c_str(),"r");
            vector<string> rows;
            while( !feof(file) ){
                char buf[100000];
                fgets(buf,100000,file);
                if(strlen(buf)==0)
                    break;
                string str(buf);
                memset(buf,0,100000);
                rows.push_back(str);
            }
            int n=rows.size();
            droped_by_filter=0;
            vector<vector<std::string> > data_ln;
            num_row=0;
            for(int i=0;i<n;i++){
                vector<std::string> ml_rows=pars(rows.at(i));
                if(ml_rows.size()>0)
                    data_ln.push_back(ml_rows);
            }
            BOOST_LOG_SEV(lg, info)<<data_ln.size()<<" lines to load into the database";
            BOOST_LOG_SEV(lg, warning)<<droped_by_filter<<" lines droped by filter";
            fclose(file);
            //load tables data
            const CoreN::CNoraPtr& cnora=getCNora(I);
            CoreN::CNora::Values v;
            string steerLog="steer_log";
            string ss7_log="ss7_log";
            std::string insert_begin="INSERT INTO "+steerLog+"."+ss7_log+" (";
            std::string insert_end=") VALUES(";
            vector<string> types={"::timestamp", "::varchar", "::int8", "::varchar", "::varchar","::text"};
            for(int i=0;i<table_name->size();i++)
            {
                insert_begin=insert_begin+table_name->at(i);
                char n[10];
                sprintf(n,"%d",i+1);
                insert_end=insert_end+"$"+n+types.at(i);
                if((i+1)!=table_name->size())
                {
                    insert_end+=",";
                    insert_begin+=",";
                }
            }
            std::string insert_cmd_str=insert_begin+insert_end+")";
            std::cout<<"\n"<<insert_cmd_str<<"\n";
            for(int i=0;i<data_ln.size();i++){
                for(int j=0;j<data_ln.at(i).size();j++){
                    v.emplace_back(CoreN::CNora::Value(data_ln.at(i).at(j)));
                }
                std::cout<<data_ln.at(i).at(0)<<"\t";
                cnora->Request(
                    insert_cmd_str, // SQL
                    v, // Binded params (now empty)
                    CoreN::CNora::Commit | CoreN::CNora::StopSession, // Some flags for request
                        [](const CoreN::CNora::Rows& ) {},
                        [](const CoreN::Error& error){
                            std::cout << "Request status: " << error << std::endl;
                        } // Method on request finish
                );
                v.clear();
            };
        }
    }
}

int main()
{
    init();
    signal(SIGINT, finish_prog_func);
    signal(SIGABRT,sig_abort_func);
    logging::add_common_attributes();
    BOOST_LOG_SEV(lg, info) << "Settings accepted";
    boost::thread thread(&mainThread);
    rebind(I->on_connect, [](const boost::function<void()>& handler){
        std::cout<<"\n" <<"Ready to work"<<"\n";
        handler();
    });
//  OnPressed.connect(boost::bind(&on_signal, _1, _2, boost::ref(I)));
//  boost::asio::signal_set signals(io, SIGINT, SIGTERM);
//      signals.async_wait(
//          boost::bind(&on_signal, _1, _2, boost::ref(I))
//      );
    boost::system::error_code error;
    io.run(error);
    thread.join();
    if (error) {
        log_error("ios.run() error: " << error.message());
    }
    log_debug("ios thread terminated");
    return 0;
}

