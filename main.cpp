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
#include <time.h>
#include <boost/log/trivial.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/keywords/format.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/thread.hpp>
namespace logging = boost::log;
namespace sinks = boost::log::sinks;
namespace src = boost::log::sources;
namespace expr = boost::log::expressions;
namespace attrs = boost::log::attributes;
namespace keywords = boost::log::keywords;
using namespace logging::trivial;
src::severity_logger< severity_level > lg;
vector<std::string>* table_name;
vector<std::string>* paths;
using namespace std;
using namespace rapidjson;
PGconn* conn;
bd_data* conf_data; //in which database should connect
vector<string>* data_for_search; //data from config file
BD* bd; //data base worker
vector<ignor_list> ignor_lists;
 //data which should insert to database

std::string to_string(int number)
{
    char str[11];
    sprintf(str, "%d", number);
    return std::string(str);
}

std::string GetElementValue(const Value& val)
{
  if (val.GetType() == rapidjson::kNumberType)
    return to_string(val.GetInt());
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

vector<string> split(string str,char delimitr)
{
    vector<string> mass;
    int pos=0;
    if(str.find(delimitr)!=pos!=string::npos)
        while(pos!=string::npos)
        {
            pos=str.find(delimitr);
            mass.push_back(str.substr(0,pos));
            str.erase(0,pos+1);
        }
    else
        mass.push_back(str);
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

int processing_ignor_list(string jstr)
{
    Document document;
    document.Parse(jstr.c_str());
    for(int i=0;i<ignor_lists.size();i++)
    {
        vector<string> mass_str=split(ignor_lists.at(i).name, '.');
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
    int end_jstr;
    int i=0;
    int data_for_search_iter=0;
    vector<string> data;
    //before start pars put all data in vector 0
    for(int i=0;i<data_for_search->size();i++)    
        data.push_back("0");    
    //find data in json file
    string pcap; //special streeng in which will develop all pcap
    while(data_for_search_iter<data_for_search->size())
    {
        string testStr=mass_from_js; //string like [{some data,data},{some data:{data,data}},{}]
        vector<string> mass_str;
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
            string json_str=data_for_search->at(data_for_search_iter); //object from conf file which should i find
            vector<string> mass_or_str=split(json_str, '/');
            for(int i=0;i<mass_or_str.size();i++)
            {
                string json_data=mass_or_str.at(i);
                mass_str=split(json_data, '.'); //split by point
                if(processing_ignor_list(jstr)==1)
                {
                    data.clear();
                    return data;
                }
                Document document;
                document.Parse(jstr.c_str()); //pars json str
                Value::ConstMemberIterator iter=document.FindMember(mass_str.at(0).c_str()); //is object from conf file object which i find from json file
                if(iter!=document.MemberEnd())
                {
                    Value& val=document[mass_str.at(0).c_str()]; //try find value of parametr which i need
                    for(int i=1; i<mass_str.size();i++)
                    {
                        Value& obj=val;
                        Value::ConstMemberIterator j=val.FindMember(mass_str.at(i).c_str());
                        if(j!=val.MemberEnd())
                            val=obj[mass_str.at(i).c_str()];
                        else
                            break;
                    }
                    //std::cout<<mass_str.at(0)<<" - "<<GetElementValue(val)<<"\n";
                    if(mass_str.at(0)=="TS") //if object is pcap i should sum str_pcap
                    {
                        string TS=val.GetString();
                        TS=timeStampToString(TS);
                        data.at(data_for_search_iter)=TS;
                        break;
                    }
                    if(mass_str.at(0)!="PCAP") //if object is pcap i should sum str_pcap
                    {
                        std::string val_str=val.GetString();
                        if(val_str.size()!=0)
                            data.at(data_for_search_iter)=val_str; //if its not pcap i find object once
                        break;
                    }
                    else
                    {
                        string new_pcap=val.GetString();
                        new_pcap+="\\n";
                        pcap=pcap+new_pcap;
                        data.at(data_for_search_iter)=pcap;
                    }
                }
            }
            if(data.at(data_for_search_iter)!="0"&&mass_str.at(0)!="PCAP")
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

vector<file_data> dmpfile_lookup(string absolute_path)
{
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
                if(S_ISREG(sb.st_mode)&&str_file=="dmp")
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
    bd->finish();
    exit(0);
}

void init()
{
    //open config file
    libconfig::Config conf;
    try
    {
        conf.readFile("/opt/svyazcom/etc/dmp2db_smsc_lv2.conf"); //opt/svyazcom/etc/dmp_sca.conf
    }
    catch(libconfig::ParseException e)
    {
        BOOST_LOG_SEV(lg, error)<<"Can not open file";
    }
    //load paths
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
        string conf_str="application.ignor_list.obj"+to_string(i)+".name";
        string str_name=conf.lookup(conf_str);
        list.name=str_name;
        conf_str="application.ignor_list.obj"+to_string(i)+".values";
        //std::cout<<conf_str<<"\n";
        int ignor_list_num_values=conf.lookup(conf_str).getLength();
        for(int j=0;j<ignor_list_num_values;j++)
        {
            string str_v=conf.lookup(conf_str)[j];
            list.values.push_back(str_v);
        }
        ignor_lists.push_back(list);
    }

    //std::string log_path_str= conf.lookup("application.paths.logDir");

        logging::add_file_log
        (
            keywords::file_name =paths->at(3)+"/%Y-%m-%d.log",
                    keywords::auto_flush = true ,
            keywords::time_based_rotation = sinks::file::rotation_at_time_point(0, 0, 0),
            keywords::format =
            (
                expr::stream
                << expr::format_date_time< boost::posix_time::ptime >("TimeStamp", "%Y-%m-%d %H:%M:%S")
                << "\t: <" << logging::trivial::severity
                << "> \t" << expr::smessage
            )
        );
}

int main()
{
    init();
    signal(SIGINT, finish_prog_func);
    signal(SIGABRT,sig_abort_func);
    logging::add_common_attributes();
    BOOST_LOG_SEV(lg, info) << "Settings accepted";
    while(1)
    {
        sleep(60);
        vector<file_data> dmp=dmpfile_lookup(paths->at(0));
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
            std::string str_copy_result_cmd="cp "+absolute_path_to_file+" "+work_file+" -f";
            BOOST_LOG_SEV(lg, info) <<str_copy_result_cmd;
            system(str_copy_result_cmd.c_str());
            BOOST_LOG_SEV(lg, info) <<"Getting started with the file "<<work_file;
            FILE * file;
            file = fopen(work_file.c_str(),"r");
            vector<string> rows;
            while( !feof(file) )
            {
                char buf[100000];
                fgets(buf,100000,file);
                if(strlen(buf)==0)
                    break;
                string str(buf);
                memset(buf,0,100000);
                rows.push_back(str);
            }
            int n=rows.size();
            vector<vector<std::string> > data_ln;
            for(int i=0;i<n;i++)
            {
                vector<std::string> ml_rows=pars(rows.at(i));
                if(ml_rows.size()>0)
                    data_ln.push_back(ml_rows);
            }
            BOOST_LOG_SEV(lg, info) <<data_ln.size()<<" lines to load into the database";
            int start_time=clock();
            std::string copy_res=bd->copy(data_ln,table_name);
            int finish_time=clock();
            if(copy_res=="Successfully added to the database")  BOOST_LOG_SEV(lg, info) <<"Successfully added to the database";
            else BOOST_LOG_SEV(lg, error)<<copy_res;
            fclose(file);
            std::string str_rm_cmd="mv "+work_file+" "+paths->at(1)+" -f";
            system(str_rm_cmd.c_str());
            BOOST_LOG_SEV(lg, info) <<str_rm_cmd;
            std::string str_result_cmd="mv "+absolute_path_to_file+" "+paths->at(1)+" -f";
            BOOST_LOG_SEV(lg, info) <<str_result_cmd;
            system(str_result_cmd.c_str());
        }
    }
    bd->finish();
    return 0;
}
