#ifndef __PAM2_h__
#define __PAM2_h__

#include "mem/SimpleSSD_types.h"

#include "mem/GlobalConfig.h"
#include "mem/Latency.h"
#include "mem/PAMStatistics.h"
//#include "RequestQueue2.h"
#include "mem/ftl.hh" //for using Command Queue in FTL
#include "mem/Simulator.h"

#include "mem/PAM2_TimeSlot.h"

#include <iostream>
#include <string>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <fstream>
using namespace std;

class PAMStatistics;
//Globals
extern GlobalConfig* gconf;
extern Latency* lat;
//extern Statstics* stats;
//extern RequestQueue2* rqueue2; //using command queue in FTL
extern Simulator* sim;

class PAM2 //let's not inherit PAM1
{
    public:
        PAM2(FTL* ftl, PAMStatistics* statistics);
        ~PAM2();

        TimeSlot** ChTimeSlots;
        TimeSlot** DieTimeSlots;
        TimeSlot** MergedTimeSlots; //for gathering busy time

        std::map<uint64, uint64> OpTimeStamp[3];

        std::map<uint64, std::map<uint64, uint64>* > * ChFreeSlots;
        uint64* ChStartPoint; //record the start point of rightmost free slot
        std::map<uint64, std::map<uint64, uint64>* > * DieFreeSlots;
        uint64* DieStartPoint;

        FTL* ftl_core;
        void FetchQueue();

        PAMStatistics* stats; //statistics of PAM2, not created by itself
        void InquireBusyTime(uint64 currentTick);
        void FlushTimeSlots(uint64 currentTick);
        void FlushOpTimeStamp();
        TimeSlot* FlushATimeSlot(TimeSlot* tgtTimeSlot, uint64 currentTick);
        TimeSlot* FlushATimeSlotBusyTime(TimeSlot* tgtTimeSlot, uint64 currentTick, uint64* TimeSum);
        //Jie: merge time slots
        void MergeATimeSlot(TimeSlot* tgtTimeSlot);
        void MergeATimeSlot(TimeSlot* startTimeSlot, TimeSlot* endTimeSlot);
        void MergeATimeSlotCH(TimeSlot* tgtTimeSlot);
        void MergeATimeSlotDIE(TimeSlot* tgtTimeSlot);
        TimeSlot* InsertAfter(TimeSlot* tgtTimeSlot, uint64 tickLen, uint64 tickFrom);

        TimeSlot* FindFreeTime(TimeSlot* tgtTimeSlot, uint64 tickLen, uint64 tickFrom); // you can insert a tickLen TimeSlot after Returned TimeSlot.

        //Jie: return: FreeSlot is found?
        bool FindFreeTime(std::map<uint64, std::map<uint64, uint64>* >& tgtFreeSlot, uint64 tickLen, uint64 & tickFrom, uint64 & startTick, bool & conflicts);
        void InsertFreeSlot(std::map<uint64, std::map<uint64, uint64>* >& tgtFreeSlot, uint64 tickLen, uint64 tickFrom, uint64 startTick, uint64 & startPoint, bool split);
        void AddFreeSlot(std::map<uint64, std::map<uint64, uint64>* >& tgtFreeSlot, uint64 tickLen, uint64 tickFrom);
        void FlushFreeSlots(uint64 currentTick);
        void FlushAFreeSlot(std::map<uint64, std::map<uint64, uint64>* >& tgtFreeSlot, uint64 currentTick);
        uint8 VerifyTimeLines(uint8 print_on);


        //PPN Conversion related //ToDo: Shifted-Mode is also required for better performance.
        uint32 RearrangedSizes[7];
        uint32 CPDPBPtoDieIdx(CPDPBP* pCPDPBP);
        void printCPDPBP(CPDPBP* pCPDPBP);
        void PPNtoCPDPBP(uint64* pPPN, CPDPBP* pCPDPBP);
        void CPDPBPtoPPN(CPDPBP* pCPDPBP, uint64* pPPN);

      #if GATHER_TIME_SERIES
        //To Gather Time Series Data
        uint64 TimeSeriesLastTick;
        uint8 SavedTimeSeries;
        uint64 SavedMinTick, SavedNumActiveCh,SavedNumActiveDie;
        void GetTimeSeries(uint64 currentTick);
        void CountActive(uint64 currentTick, uint64* activeCh, uint64* activeDie);
        void GetNextMinTick(uint64 currentMinTick, uint64* p_nextMinTick, uint8* p_runOneMore);
      #endif
};

#endif
