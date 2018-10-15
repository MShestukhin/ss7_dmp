#include "log.h"

log::log()
{
}

void log::init(std::string path_log){
    path=path_log+"/";
}

std::string log::currentDateTime(std::string format) {
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), format.c_str(), &tstruct);
    return buf;
}
void log::info(std::string s){
    msg(s,"INFO");
}
void log::warning(std::string s){
    msg(s,"WARNING");
}
void log::error(std::string s){
    msg(s,"ERROR");
}

void log::msg(std::string s, std::string inf_level){
    std::string file_name=path+currentDateTime("%d.%m.%Y")+".log";
    std::ofstream ifs(file_name.c_str(), std::ios_base::in | std::ios_base::app);
     if (ifs.is_open())
     {
         ifs <<currentDateTime("%d.%m.%Y %H:%M:%S")<<"\t"+inf_level+"\t"<< s+"\n";
         ifs.close();
     }
     else
     {
         std::cout << "Error opening log file\n";
         exit(3);
     }

}



