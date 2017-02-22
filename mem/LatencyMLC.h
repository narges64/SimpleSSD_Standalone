#ifndef __LatencyMLC_h__
#define __LatencyMLC_h__

#include "mem/Latency.h"

class LatencyMLC : public Latency
{
    public:
        LatencyMLC(uint32 mhz, uint32 pagesize);

        uint64 GetLatency(uint32 AddrPage, uint8 Oper, uint8 Busy);
        inline uint8  GetPageType(uint32 AddrPage);
};

#endif //__LatencyMLC_h__
