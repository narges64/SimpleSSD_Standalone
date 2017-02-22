#include "mem/Simulator.h"

Simulator::Simulator()
: TickGlobal( 0 ), TickNextEvent( 0 )
{
    printf("Init: TickGlobal = %llu, TickNextEvent = %llu\n", TickGlobal, TickNextEvent );
}

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
uint64 Simulator::GetTick()
{
    return TickGlobal;
}

void Simulator::TickGo()
{
    if (TickGlobal > TickNextEvent)
    {
        printf("TickGlobal = %llu, Requested_TickNextEvent = %llu\n", TickGlobal, TickNextEvent);
        std::cerr<<GetTick()<<": ** CRITICAL ERROR: Tick Error - Trying to go back to past !!\n";
        std::terminate();
    }

    TickGlobal = TickNextEvent;
}

uint8 Simulator::TickReserve(uint64 nextTick)
{
    if (TickNextEvent == nextTick)
    {
        return 0;
    }

    if (nextTick < TickGlobal)
    {
        return 1;
    }

    if ( TickNextEvent == TickGlobal || (TickNextEvent > nextTick && nextTick > TickGlobal) )
    {
        TickNextEvent = nextTick;
    }

    return 1;
}
