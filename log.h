#ifndef log_H
#define log_H
#include <fstream>
#include <iostream>
class log
{
public:
    log();
    std::string currentDateTime(std::string format);
    std::string path;
    void init(std::string path_log);
    void info(std::string s);
    void warning(std::string s);
    void error(std::string s);
    void msg(std::string s,std::string inf_level);
};

#endif // log_H
