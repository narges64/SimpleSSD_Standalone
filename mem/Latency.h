#ifndef __Latency_h__
#define __Latency_h__

#include "mem/SimpleSSD_types.h"

#include <iostream>
#include <string>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <fstream>
using namespace std;

/*==============================
    Latency
==============================*/ 
class Latency
{
    public:
        uint32 SPDIV; //50 to 100mhz
        uint32 PGDIV;

        //Get Latency for PageAddress(L/C/MSBpage), Operation(RWE), BusyFor(Ch.DMA/Mem.Work)
        virtual uint64 GetLatency(uint32 AddrPage, uint8 Oper, uint8 BusyFor){ return 0; };
        virtual inline uint8  GetPageType(uint32 AddrPage) { return PAGE_NUM; };

        //Setup DMA speed and pagesize
        Latency(uint32 mhz, uint32 pagesize);
};


#endif //__Latency_h__
