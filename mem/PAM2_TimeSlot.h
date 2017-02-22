#ifndef __PAM2_TimeSlot_h__
#define __PAM2_TimeSlot_h__

#include "mem/SimpleSSD_types.h"

#include <iostream>
#include <string>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <fstream>
using namespace std;

class TimeSlot
{
    public:
        TimeSlot(uint64 startTick, uint64 duration);
        uint64 StartTick;
        uint64 EndTick;
        TimeSlot* Next;
};

#endif
