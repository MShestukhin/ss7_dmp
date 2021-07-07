#include "working_file.h"
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <algorithm>
#include "log.h"
extern class log logg;
Working_file::Working_file()
{
    file_mtime=0;
    name="";
}

vector<Working_file> file_lookup(string absolute_path, string file_name,bool (*pt2Func)(string,string )){
    DIR* dir;
    struct dirent* entry;
    struct stat sb;
    vector<Working_file> vdata_file;
    dir=opendir(absolute_path.c_str());
    if(dir==nullptr)
    {
        logg.error("Can not open folder ");
        return vdata_file;
    }
    while ((entry=readdir(dir))!=nullptr)
    {
        std::string str_file=entry->d_name;
        std::string str_dir_file=absolute_path+"/"+str_file;
        //lookup some file in dir
        if(str_file!="."&&str_file!="..")
        {
            Working_file time_name_file;
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
    std::sort (vdata_file.begin(), vdata_file.end(), [](Working_file i,Working_file j){return (i.file_mtime>j.file_mtime);});
    return vdata_file;
}
void transport_file(string format,string from_pth, string to_pth, string file, bool (*pt2Func)(string,string )){
    vector<Working_file> dmp=file_lookup(from_pth,file,pt2Func);
    for(int i=0;i<dmp.size();i++)
    {
        string absolute_path_to_file=dmp.at(i).name;
        logg.info("File detected "+absolute_path_to_file);
        char buffer[80];
        time_t seconds = time(nullptr);
        tm* timeinfo = localtime(&seconds);
        strftime(buffer, 80, format.c_str(), timeinfo);
        string work_file=to_pth+"/"+std::string(buffer)+absolute_path_to_file.substr(absolute_path_to_file.rfind("/")+1,string::npos);
        std::string str_copy_result_cmd="mv "+absolute_path_to_file+" "+work_file+" -f";
        logg.info(str_copy_result_cmd);
        system(str_copy_result_cmd.c_str());
    }
}
