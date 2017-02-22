#ifndef __GlobalConfig_h__
#define __GlobalConfig_h__

//#include "arch/isa_traits.hh"
#include "mem/SimpleSSD_types.h"
#include "mem/ConfigReader.h"
#include "base/types.hh"
#include <iostream>
#include <string>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <fstream>
using namespace std;

class ConfigReader;
/*==============================
    GlobalConfig
==============================*/
class GlobalConfig
{
    private:
        /*
        CREATE_ACCESSOR(uint32, Num_Channel);
        CREATE_ACCESSOR(uint32, Num_Die);
        CREATE_ACCESSOR(uint32, Num_Plane);
        CREATE_ACCESSOR(uint32, Num_Block);
        CREATE_ACCESSOR(uint32, Num_Page);
        CREATE_ACCESSOR(uint32, Size_Page);
        */
    public:
        uint8  NANDType;
        uint32 NumChannel;  //in a ssd
        uint32 NumPackage;  //in a channel
        uint32 NumDie;      //in a package
        uint32 NumPlane;    //in a die
        uint32 NumBlock;    //in a plane
        uint32 NumPage;     //in a block
        uint32 SizePage;    //in a page
        uint8  AddrSeq[7];
        uint32 DMAMHz;      //DMA_MHz
	long double FTLOP;
        long double FTLGCThreshold;
        uint32 FTLMapN;
        uint32 FTLMapK;
	uint32 FTLEraseCycle; 
	int SuperblockDegree; 
	int SuperpageDegree; 
	double Warmup; 
        uint8  EnableDMAPreemption; //DMA Preemption Option

        uint32 OriginalSizes[6];


        GlobalConfig(ConfigReader*  cr );
	/*        GlobalConfig(uint8 vNANDType, uint32 vChannel, uint32 vPackage, uint32 vDie, uint32 vPlane,
		  uint32 vBlock, uint32 vPage, uint32 vPageSize, uint8* pAddrSeq, uint8 DMAPreemption);*/

        void PrintInfo();
        uint64 GetTotalSizeSSD();
        uint64 GetTotalNumPage();
        uint64 GetTotalNumBlock();
        uint64 GetTotalNumDie();
        uint64 GetTotalNumPlane();
};

#endif //__GlobalConfig_h__
