#ifndef PARSER_H
#define PARSER_H
#include <vector>
#include <string.h>
#include <iostream>
#include <stdio.h>
#include <dirent.h>
using namespace std;
class parser
{
private :
public:
    parser();
    void transform_to_timestamp_promat(vector<vector<std::string> >* mass_ln_to_transform, int num_table);
    vector<vector<string> > pars_file(std::string fileName, char delimiter, int data_num, int start_reading_line);
    vector<std::string> mass_broken_ln;
    std::string Finding_All_Regex_Matches(std::string text, std::string regx);
    std::string Finding_Regex_Match(std::string text, std::string regx, int element);
};
bool contains(std::string s_cel,std::string s_find);
bool compare_str(string str_first,string str_second);
std::string timeStampToString(string ts);
vector<string> split(string str,const char * delimitr);
std::string toString(int number);
int toNumber(std::string str);

#endif // PARSER_H
