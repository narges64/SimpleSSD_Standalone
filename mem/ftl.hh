//
//  FTL.h
//  FTL-3
//
//  Created by Narges on 7/3/15.
//  Copyright (c) 2015 narges shahidi. All rights reserved.
//

#ifndef __FTL_3__FTL__
#define __FTL_3__FTL__

#include <iostream>


#include "mem/ftl_hybridmapping.hh"
#include "mem/ftl_mappingtable.hh"
#include "mem/ftl_commandqueue.hh"
#include "mem/ftl_request.hh"
#include "mem/ftl_defs.hh"
#include "mem/hil.h"
#include "mem/ftl_statistics.hh"

#include "mem/GlobalConfig.h"
#include <cfloat>
#include <time.h>

class HIL;
class CommandQueue;
class FTL{

    friend class CommandQueue; 
protected:
	Parameter * param;
	HIL * hil; 
	std::map<long int, OutputRequest *> PAMMap; 
	void read(Addr lpn, InputRequest * ireq, const Mask & mask, bool synced, bool init = false);
	void write(Addr lpn, InputRequest * ireq, const Mask & mask,  bool synced, bool init = false );
public:
    FTL(int disk); 
    FTL(Parameter *p, int disk);
    FTL(GlobalConfig *p, int disk);
    ~FTL();
	
	FTLStats ftl_statistics;  
	int disk_number;  
    CommandQueue * que;
	MappingTable * map;
    Tick current_time;   

	// Output Requests Sequence number (for requests we send to the PAM) 
	long int seq_num; 
 
	void setHIL(HIL * h){hil = h; }
	void initialize(); 
    Parameter * getParameter(){return param;}
	void PAM_SendRequest(long int seq_num, Tick time, Addr ppn, int operation, const Mask & superpage_mask, bool synced, Addr lpn = -1, InputRequest * ireq = NULL, bool init = false);  
	void PAM_SetLatency(long int seq_num, Tick latency); 
	void readTransaction(Addr lpn, int size, Tick TransTick, bool synced, bool init = false);
	void writeTransaction(Addr lpn, int size, Tick TransTick, bool synced, bool init= false);
	void PrintStats(Tick sim_time, bool final_call);
};

#endif /* defined(__FTL_3__FTL__) */
