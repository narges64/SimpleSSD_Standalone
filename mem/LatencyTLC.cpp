#include "mem/LatencyTLC.h"

LatencyTLC::LatencyTLC(uint32 mhz, uint32 pagesize)
: Latency(mhz, pagesize)
{
    ;
}

inline uint8 LatencyTLC::GetPageType(uint32 AddrPage)
{
    return (AddrPage<=5)?PAGE_LSB : ( (AddrPage<=7)?PAGE_CSB : (((AddrPage-8)>>1)%3) );
}

uint64 LatencyTLC::GetLatency(uint32 AddrPage, uint8 Oper, uint8 Busy)
{
    #if 1
    //ps
  uint64 lat_tbl[3][5] =  //uint32 lat_tbl[3][5] = //Gieseo,is this right?
    {                /*  LSB           CSB         MSB         DMA0,                  DMA1*/
        /* Read  */{   58000000,   78000000,  107000000,    100000/SPDIV,         185000000/(PGDIV*SPDIV) },
        /* Write */{  558000000, 2201000000, 5001000000, 185000000/(PGDIV*SPDIV), 100000/SPDIV },
        /* Erase */{ 2274000000, 2274000000, 2274000000,   1500000/SPDIV,         100000/SPDIV }
    };
    #else
    //ns
    uint64 lat_tbl[3][5] =  //uint32 lat_tbl[3][5] = //Gieseo,is this right?
    {                /*  LSB      CSB      MSB    DMA0,  DMA1*/
        /* Read  */{   58000,   78000,  107000,    100/SPDIV, 185000/(PGDIV*SPDIV) },
        /* Write */{  558000, 2201000, 5001000, 185000/(PGDIV*SPDIV),    100/SPDIV },
        /* Erase */{ 2274000, 2274000, 2274000,   1500/SPDIV,    100/SPDIV }
    };
    #endif

    switch(Busy)
    {
        case BUSY_DMA0:
            return lat_tbl[Oper][3];
        case BUSY_DMA1:
            return lat_tbl[Oper][4];
        case BUSY_MEM:
        {
            //uint8 ptype = (AddrPage<=5)?PAGE_LSB : ( (AddrPage<=7)?PAGE_CSB : (((AddrPage-8)>>1)%3) );
            uint8 ptype = GetPageType(AddrPage);
            uint64 ret= lat_tbl[Oper][ ptype ];

            #if DBG_PRINT_TICK
            //    printf("LAT %s page_%u(%s) = %llu\n", OPER_STRINFO[Oper], AddrPage, PAGE_STRINFO[ptype], ret);
            #endif
            return ret;
        }
        default:
            break;
    }
    return 10;
}
