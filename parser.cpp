#include "parser.h"
#include <iostream>
#include <regex>
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
    std::vector<string> v;
    char* chars_array = strtok((char*)str.c_str(), delimitr);
    while(chars_array)
    {
        v.push_back(chars_array);
        chars_array = strtok(nullptr, delimitr);
    }
    return v;
}

parser::parser()
{
}

void parser::transform_to_timestamp_promat(vector<vector<std::string> >* mass_ln_to_transform, int num_row){
    for(int i=0;i<mass_ln_to_transform->size();i++){
        std::string str_tr=mass_ln_to_transform->at(i).at(num_row);
        std::string timestamp=str_tr.substr(0,4)+"-"+str_tr.substr(4,2)+"-"+str_tr.substr(6,2)+" "+str_tr.substr(8,2)+":"+str_tr.substr(10,2)+":"+str_tr.substr(12,2);
        mass_ln_to_transform->at(i).at(num_row)=timestamp;
    }
}

std::string parser::Finding_All_Regex_Matches(std::string text, std::string regx){
    string pcap;
    try {
        std::regex re(regx);
      std::sregex_iterator next(text.begin(), text.end(), re);
      std::sregex_iterator end;
      while (next != end) {
        std::smatch match = *next;
        pcap+= match[2].str()+"\n";
        next++;
      }
    } catch (std::regex_error& e) {
      // Syntax error in the regular expression
    }
    return pcap;
}
std::string parser::Finding_Regex_Match(std::string text, std::string regx, int element){
    std::string result;
    try {
      std::regex re(regx);
      std::smatch match;
      if (std::regex_search(text, match, re) && match.size() > 1) {
        result = match.str(element);
      } else {
        result = std::string("Can not find regular expression");
      }
    } catch (std::regex_error& e) {
      std::cout<<e.what();
    }
    return result;
}

vector<vector<std::string> > parser::pars_file(std::string fileName,char delimiter, int data_num, int start_reading_line){
    vector<vector<std::string> > rows;
    vector<std::string> row;
    FILE* file;
    char* nameFile=(char*)fileName.c_str();
    file=fopen(nameFile,"r");
    if(file == NULL)    
        return rows;    
    for(int i=1; i<start_reading_line;i++)
    {
        char buf[100];
        fgets(buf,100,file);
    }

    while(!feof(file))
    {
        char buf[100];
        fgets(buf,100,file);
        if(strlen(buf)==0)
            break;

        std::string s1(buf);
        memset(buf,0,100);
        if(delimiter==NULL)
        {
            std::string::size_type n = s1.find(",");
            if ( n != std::string::npos )
                delimiter=',';

            else
                delimiter=';';
        }
        int j=0;
        int i=0;
        int end=0;
        while(i<s1.size())
        {
            if(s1.at(i)==delimiter)
            {
                std::string str=s1.substr(end,i-end);
                row.push_back(str);
                end=i+1;
                j++;
            }
            if(s1.at(i)=='\x0D') {};
            if(s1.at(i)!='\x0A') i=i+1;
            else break;
        }
        std::cout<<i<<"\n";
        std::cout<<end<<"\n";
        s1=s1.substr(end,i-end);
        row.push_back(s1);
        if(data_num!=0)
        {
            if(j!=data_num-1)
                mass_broken_ln.push_back(s1);            
            else
               rows.push_back(row);
        }
    }
    fclose(file);
    return rows;
}

