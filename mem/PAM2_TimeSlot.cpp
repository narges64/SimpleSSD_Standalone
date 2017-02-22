#include "mem/PAM2_TimeSlot.h"

TimeSlot::TimeSlot(uint64 startTick, uint64 duration)
{
    StartTick = startTick;
    EndTick = startTick + duration - 1;
    Next = NULL;
}
