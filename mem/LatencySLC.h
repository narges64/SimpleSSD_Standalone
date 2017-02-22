#ifndef __LatencySLC_h__
#define __LatencySLC_h__

#include "mem/Latency.h"

class LatencySLC : public Latency
{
    public:
        LatencySLC(uint32 mhz, uint32 pagesize);

        uint64 GetLatency(uint32 AddrPage, uint8 Oper, uint8 Busy);
        inline uint8  GetPageType(uint32 AddrPage);
};

#endif //__LatencyTLC_h__
