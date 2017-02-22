#include "mem/ConfigReader.h"

ConfigReader::ConfigReader(string filename)
{
    
    fh.open(filename.c_str());
    fn = filename;

    if (!fh)
    {
      printf("File %s open FAIL!\n", filename.c_str());
        std::cerr<<": ** CRITICAL ERROR: can NOT open CONFIG file!\n";
        std::terminate();
    }
}
ConfigReader::~ConfigReader()
{
    fh.close();
}

void ConfigReader::trim(string& str)
{
    int h=-1, c=-1, t=-1;
    for(uint32 i=0; i<str.length(); i++)
    {
        if(str[i] != ' '  && str[i] != '\t' && str[i] != '#' && h==-1)
        {
            h = i;
        }

        if(str[i] == '#')
        {
            c = i-1;
            break;
        }
    }

    if ( h == -1 )
    {
        str="";
        return;
    }

    for (uint32 i= ( (c==-1)?str.length()-1:c ) ; i>=0; i--)
    {
        if(str[i] != ' '  && str[i] != '\t' && str[i] !='\n' && str[i] !='\r' )
        {
            t = i;
            break;
        }
    }
    //cout<<str<<" ::: head="<<h<<", cmt="<<c<<", tail="<<t<<"\n";

    str = str.substr(h, t-h+1);
}

void ConfigReader::parse(string& buf, string& entry, string& val)
{
    //uint32 i = 0;

    size_t found = buf.find_first_of("=",0);
    if (found != string::npos)
    {
        entry = buf.substr(0, found);
        val = buf.substr(found+1);
        trim(entry);
        trim(val);

        //cout<<buf<<" ::: "<<entry<<" === "<<val<<"\n";
        return;
    }
    entry = "";
    val = "";
}

string ConfigReader::ReadString(string entry, string def)
{
    string str;
    string name,val;

    if (!fh) //very dirty part due to auto-close of fh. need to find other way.
    {
        fh.close();
        fh.open( fn.c_str() );
        if (!fh)
        {
            printf("File still closed?!\n");
        }
    }

    fh.seekg(0, std::ifstream::beg);
    while ( std::getline(fh, str, '\n') )
    {
        trim(str);
        if ( str.length() == 0 )
        {
            continue;
        }
        parse(str, name, val);
        if (name == entry)
        {
            #if DBG_PRINT_CONFIGPARSER
                cout<<"-->ReadStr: "<<name<<" = "<<val<<".\n";
            #endif
            return val;
        }
    }

    printf("** Can NOT find Entry [%s]\n", entry.c_str() );
    return def;
}

uint32 ConfigReader::ReadInt32(string entry, uint32 def)
{
    string val = ReadString(entry,"");
    //cout<<"ReadI32: "<<entry<<"="<<val<<"\n";
    if (val.length()==0)
    {
        printf("** Using default [%s]=[%u]\n", entry.c_str(), def );
        return def;
    }

    char* end;
    uint32 ret = (uint32)std::strtoll(val.c_str(),&end,10);
    if ( end != val.c_str() )
    {
        return ret;
    }
    else
    {
        //Check ADDR_XXXXX conversion
        for (int i=0; i<ADDR_NUM; i++)
        {
            string match = ADDR_STRINFO2[i];
            //printf("** m[%s] v[%s] \n", match.c_str(), val.c_str());
            if (match == val)
            {
                return i;
            }
        }

        //Check SLC/MLC/TLC conversion
        for (int i=0; i<NAND_NUM; i++)
        {
            string match = NAND_STRINFO[i];
            if (match == val)
            {
                return i;
            }
        }

    }
    printf("** ReadInt32 ERROR - %s \n", entry.c_str());
    return def;
}

long double ConfigReader::ReadFloat(string entry, long double def)
{
    string val = ReadString(entry,"");
    if (val.length()==0)
    {
        printf("** Using default [%s]=[%Lf]\n", entry.c_str(), def );
        return def;
    }

    char* end;
    long double ret = (long double)std::strtof(val.c_str(),&end);
    if ( end != val.c_str() ) return ret;
    return def;
}
