#include "parser.h"

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
