#ifndef __ConfigReader_h__
#define __ConfigReader_h__

#include "mem/SimpleSSD_types.h"  /////////////////////////

#include "mem/Simulator.h"   //////////////////////

#include <iostream>
#include <string>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <fstream>
//#include <strtoll>
using namespace std;

//Globals
//extern Simulator* sim;

//Note: NO performance consideration in ConfigReader, just very easy written code

class ConfigReader
{
    private:
        std::ifstream fh;
        string fn;
    public:
        ConfigReader(string filename);
        ~ConfigReader();

        void trim(string& str);

        void parse(string& buf, string& entry, string& val);

        string ReadString(string entry, string def);

        uint32 ReadInt32(string entry, uint32 def);

        long double ReadFloat(string entry, long double def);
};

#endif //__ConfigReader_h__
