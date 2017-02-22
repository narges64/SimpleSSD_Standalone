#ifndef __LatencyTLC_h__
#define __LatencyTLC_h__

#include "mem/Latency.h"

class LatencyTLC : public Latency
{
    public:
        LatencyTLC(uint32 mhz, uint32 pagesize);

        uint64 GetLatency(uint32 AddrPage, uint8 Oper, uint8 Busy);
        inline uint8  GetPageType(uint32 AddrPage);
};

#endif //__LatencyTLC_h__
