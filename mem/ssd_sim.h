/*
 * ssd_sim.h
 *
 *  Created on: Jul 14, 2016
 *      Author: zhang_000
 */

#ifndef SSD_SIM_H_
#define SSD_SIM_H_

#include <iostream>
#include <string>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <fstream>
#include <list>
using namespace std;

#include "mem/ConfigReader.h"
#include "mem/GlobalConfig.h"
#include "mem/ftl.hh"
#include "mem/hil.h"
#include "mem/Latency.h"
#include "mem/LatencyMLC.h"
#include "mem/LatencyTLC.h"
#include "mem/PAM2_TimeSlot.h"
#include "mem/PAM2.h"
#include "mem/PAMStatistics.h"
#include "mem/SimpleSSD_types.h"
#include "mem/SimpleSSD.h"
#include "mem/Simulator.h"

typedef struct _Request
{
    uint64 PPN;
    uint8  Oper;
    uint32 REQ_SIZE;
    int IOGEN;
}Request;

class RequestGenerator
{
public:
	uint32 MAX_REQ;
	uint32 cur_REQ;
	uint32 cur_page;
	uint64 start_PPN;
	uint32 REQ_SIZE;
	Request curRequest;
	struct Read_Ratio{
		uint32 fraction;
		uint32 denominator;
	} readratio;
	struct Random_Ratio{
		uint32 fraction;
		uint32 denominator;
	} randomratio;
	int IOGEN;
	RequestGenerator(uint32 _MAX_REQ, uint64 _start_PPN, uint32 _REQ_SIZE, uint32 _read_fraction, uint32 _read_denominator, uint32 _random_fraction, uint32 _random_denominator, uint32 _randSeed, int _IOGEN);
	~RequestGenerator();
	bool generate_request();
};


#endif /* SSD_SIM_H_ */
