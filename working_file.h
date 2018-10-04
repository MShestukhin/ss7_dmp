#ifndef WORKER_FILE_H
#define WORKER_FILE_H
#include <iostream>
#include <vector>
using namespace std;
class Working_file
{
public:
    int file_mtime;
    std::string name;
    Working_file();
};

bool contains(std::string s_cel,std::string s_find);
bool compare_str(string str_first,string str_second );
vector<Working_file> file_lookup(string absolute_path, string file_name,bool (*pt2Func)(string,string ));
void transport_file(string format,string from_pth, string to_pth, string file, bool (*pt2Func)(string,string ));

#endif // WORKER_FILE_H
