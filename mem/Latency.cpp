#include "mem/Latency.h"


/*==============================
    Latency
==============================*/ 

/*
    Latency class is inherited to:
    - TLC
    - MLC
    - SLC
*/

Latency::Latency(uint32 mhz, uint32 pagesize)
{
    switch (mhz) //base = 50mhz
    {
        case 50:  SPDIV = 1; break;
        case 100: SPDIV = 2; break;
        case 200: SPDIV = 4; break;
        case 400: SPDIV = 8; break;
        case 800: SPDIV = 16; break;
        case 1600: SPDIV = 32; break;
        default: printf("** unsupported DMA Speed\n"); std::terminate(); break;
    }
    switch (pagesize) //base = 8KB
    {
        default: printf("** unsupported page size\n"); std::terminate(); break;
        case 8192: PGDIV = 1; break;
        case 4096: PGDIV = 2; break;
        case 2048: PGDIV = 4; break;
        case 1024: PGDIV = 8; break;
    }
};
