#ifndef __Simulator_h__
#define __Simulator_h__

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
    Simulator
==============================*/
class Simulator
{
    public:
        uint64 TickGlobal;  //system time
        uint64 TickNextEvent;  //Next Event Time

        Simulator();

        /* Simulator Seq:
           0) TickNextEvent => TickGlobal
           1) Unset Busy Resource
           2) Get Possible Request
           3) Set Busy Resource
           4) Set TickNextEvent
         
           Event Data:
           Reserver  | Action | CurStatus       | EndTime
           <Address> | <RWE>  | <DMA0/Mem/DMA1> | TickEnd
        */
        uint64 GetTick();

        void TickGo();

        uint8 TickReserve(uint64 nextTick);
};

#endif //__Simulator_h__
