#include "mem/PAMStatistics.h"
#include "mem/SimpleSSD_types.h"

//#include "debug/PAM2.hh"

#if 0 //ch-die io count (legacy)
PAMStatistics::mini_cnt_page::mini_cnt_page()
{
    memset(c,0,sizeof(uint32)*OPER_NUM*PAGE_NUM);
}
#endif

#if GATHER_RESOURCE_CONFLICT && 0 //conflict gather (legacy)
PAMStatistics::mini_cnt_conflict::mini_cnt_conflict()
{
    memset(c,0,sizeof(uint32)*OPER_NUM*CONFLICT_NUM);
}
#endif

#if 1// Polished stats - Improved instrumentation
PAMStatistics::Counter::Counter() { init(); }

void PAMStatistics::Counter::init()
{
    cnt = 0;
}

void PAMStatistics::Counter::add()
{
    cnt++;
}

PAMStatistics::CounterOper::CounterOper() { init(); }

void PAMStatistics::CounterOper::init()
{
    for (int i=0; i<OPER_ALL; i++) cnts[i].init();
}

void PAMStatistics::CounterOper::add(uint32 oper)
{
    cnts[oper].add();
    cnts[OPER_NUM].add();
}

void PAMStatistics::CounterOper::printstat(const char* namestr)
{
    char OPER_STR[OPER_ALL][8] = {"Read ", "Write", "Erase", "Total"};
    printf( "[ %s ]:\n", namestr);
    printf( "OPER, COUNT\n");
    for (int i=0;i<OPER_ALL;i++)
    {
    	printf( "%s, %llu\n",
                OPER_STR[i],
                cnts[i].cnt);
    }
}



PAMStatistics::Value::Value() { init(); }

void PAMStatistics::Value::init()
{
    sum = 0;
    cnt = 0;
    sampled_sum = 0;
    sampled_cnt = 0;
    minval = MAX64;
    maxval = 0;
    legacy_sum = 0;
    legacy_cnt = 0;
    legacy_minval = MAX64;
    legacy_maxval = 0;
}
void PAMStatistics::Value::backup(){
	sampled_sum = sum;
	sampled_cnt = cnt;
}

void PAMStatistics::Value::update(){
	legacy_sum = sum;
	legacy_cnt = cnt;
	legacy_minval = minval;
	legacy_maxval = maxval;
}

void PAMStatistics::Value::add(double val)
{
    #define MIN(a,b) (((a)<(b))?(a):(b))
    #define MAX(a,b) (((a)>(b))?(a):(b))
    sum += val;
    cnt += 1;
    minval = MIN(val, minval);
    maxval = MAX(val, maxval);
}

double PAMStatistics::Value::avg()
{
    return SAFEDIV(sum, cnt);
}

double PAMStatistics::Value::legacy_avg()
{
    return SAFEDIV(legacy_sum, legacy_cnt);
}

PAMStatistics::ValueOper::ValueOper() { init(); }
PAMStatistics::ValueOper::ValueOper(ValueOper *_ValueOper){
	for (int i=0; i<OPER_ALL; i++){
		vals[i].sum = _ValueOper->vals[i].sum;
		vals[i].cnt = _ValueOper->vals[i].cnt;
		vals[i].sampled_sum = _ValueOper->vals[i].sampled_sum;
		vals[i].sampled_cnt = _ValueOper->vals[i].sampled_cnt;
		vals[i].minval = _ValueOper->vals[i].minval;
		vals[i].maxval = _ValueOper->vals[i].maxval;
		vals[i].legacy_sum = _ValueOper->vals[i].legacy_sum;
		vals[i].legacy_cnt = _ValueOper->vals[i].legacy_cnt;
		vals[i].legacy_minval = _ValueOper->vals[i].legacy_minval;
		vals[i].legacy_maxval = _ValueOper->vals[i].legacy_maxval;
	}
}

void PAMStatistics::ValueOper::init() 
{
    for (int i=0; i<OPER_ALL; i++) vals[i].init();
}

void PAMStatistics::ValueOper::update()
{
    for (int i=0; i<OPER_ALL; i++) vals[i].update();
}

void PAMStatistics::ValueOper::add(uint32 oper, double val)
{
    vals[oper].add(val);
    vals[OPER_NUM].add(val);
}

void PAMStatistics::ValueOper::exclusive_add(uint32 oper, double val){
	vals[oper].add(val);
}

void PAMStatistics::ValueOper::printstat(const char* namestr) //This is only used by access capacity
{
    char OPER_STR[OPER_ALL][8] = {"Read", "Write", "Erase", "Total"};
    printf( "[ %s ]:\n", namestr);
    printf( "PAM: OPER, AVERAGE (B), COUNT, TOTAL (B), MIN (B), MAX(B)\n");
    for (int i=0;i<OPER_ALL;i++)
    {

        if(vals[i].cnt == 0)
        {
        	printf( "PAM: %s, ( NO DATA )\n", OPER_STR[i]);
        }
        else
        {
        	printf( "PAM: %s, %llu, %llu, %llu, %llu, %llu\n",
                   OPER_STR[i],
                   (uint64)vals[i].avg(),
				   (uint64)vals[i].cnt, (uint64)vals[i].sum,
				   (uint64)vals[i].minval, (uint64)vals[i].maxval);
        }
    }
}

void PAMStatistics::ValueOper::printstat_bandwidth(class ValueOper* Access_Capacity, uint64 ExactBusyTime, uint64 LastExactBusyTime)
{
    char OPER_STR[OPER_ALL][8] = {"Read", "Write", "Erase", "Total"};
    for (int i=0;i<OPER_ALL;i++)
    {
    	if (ExactBusyTime > LastExactBusyTime){
    		//printf("sum=%f\tsampled_sum=%f\n",Access_Capacity->vals[i].sum,Access_Capacity->vals[i].sampled_sum);
    		this->exclusive_add(i,(Access_Capacity->vals[i].sum - Access_Capacity->vals[i].sampled_sum)*1.0/MBYTE/((ExactBusyTime - LastExactBusyTime)*1.0/PSEC));
    	}
        if(vals[i].cnt == 0)
        {
        	printf( "PAM: %s bandwidth excluding idle time (min, max, average): ( NO DATA )\n", OPER_STR[i]);
        }
        else
        {
        	printf( "PAM: %s bandwidth excluding idle time (min, max, average): %.6lf MB/s, %.6lf MB/s, %.6lf MB/s\n",
                   OPER_STR[i],
				   vals[i].minval,
                   vals[i].maxval,
				   (Access_Capacity->vals[i].sum)*1.0/MBYTE/((ExactBusyTime)*1.0/PSEC));
        }
    }
}

void PAMStatistics::ValueOper::printstat_bandwidth_widle(class ValueOper* Access_Capacity, uint64 ExecutionTime, uint64 LastExecutionTime)
{
    char OPER_STR[OPER_ALL][8] = {"Read", "Write", "Erase", "Total"};
    for (int i=0;i<OPER_ALL;i++)
    {
    	assert(ExecutionTime > LastExecutionTime);
		this->exclusive_add(i,(Access_Capacity->vals[i].sum - Access_Capacity->vals[i].sampled_sum)*1.0/MBYTE/((ExecutionTime - LastExecutionTime)*1.0/PSEC));

        if(vals[i].cnt == 0)
        {
        	printf( "PAM: %s bandwidth including idle time (min, max, average): ( NO DATA )\n", OPER_STR[i]);
        }
        else
        {
        	printf( "PAM: %s bandwidth including idle time (min, max, average): %.6lf MB/s, %.6lf MB/s, %.6lf MB/s\n",
                   OPER_STR[i],
				   vals[i].minval,
                   vals[i].maxval,
				   (Access_Capacity->vals[i].sum)*1.0/MBYTE/((ExecutionTime)*1.0/PSEC));
        }
    }
}

void PAMStatistics::ValueOper::printstat_oper_bandwidth(class ValueOper* Access_Capacity, uint64* OpBusyTime, uint64* LastOpBusyTime)
{
    char OPER_STR[OPER_ALL][8] = {"Read", "Write", "Erase", "Total"};
    for (int i=0;i<3;i++) //only read/write/erase
    {
    	if (OpBusyTime[i] > LastOpBusyTime[i])
    		this->exclusive_add(i,(Access_Capacity->vals[i].sum - Access_Capacity->vals[i].sampled_sum)*1.0/MBYTE/((OpBusyTime[i] - LastOpBusyTime[i])*1.0/PSEC));

        if(vals[i].cnt == 0)
        {
        	printf( "PAM: %s-only bandwidth (min, max, average): ( NO DATA )\n", OPER_STR[i]);
        }
        else
        {
        	printf( "PAM: %s-only bandwidth (min, max, average): %.6lf MB/s, %.6lf MB/s, %.6lf MB/s\n",
                   OPER_STR[i],
				   vals[i].minval,
                   vals[i].maxval,
				   (Access_Capacity->vals[i].sum)*1.0/MBYTE/((OpBusyTime[i])*1.0/PSEC));
        }
    }
}

void PAMStatistics::ValueOper::printstat_iops(class ValueOper* Access_Capacity, uint64 ExactBusyTime, uint64 LastExactBusyTime)
{
    char OPER_STR[OPER_ALL][8] = {"Read", "Write", "Erase", "Total"};
    for (int i=0;i<OPER_ALL;i++)
    {
    	if (ExactBusyTime > LastExactBusyTime)
    		this->exclusive_add(i,(Access_Capacity->vals[i].cnt - Access_Capacity->vals[i].sampled_cnt)*1.0/((ExactBusyTime - LastExactBusyTime)*1.0/PSEC));
        if(vals[i].cnt == 0)
        {
        	printf( "PAM: %s IOPS excluding idle time (min, max, average): ( NO DATA )\n", OPER_STR[i]);
        }
        else
        {
        	printf( "PAM: %s IOPS excluding idle time (min, max, average): %.6lf, %.6lf, %.6lf\n",
                   OPER_STR[i],
				   vals[i].minval,
                   vals[i].maxval,
				   (Access_Capacity->vals[i].cnt)*1.0/((ExactBusyTime)*1.0/PSEC));
        }
    }
}

void PAMStatistics::ValueOper::printstat_iops_widle(class ValueOper* Access_Capacity, uint64 ExecutionTime, uint64 LastExecutionTime)
{
    char OPER_STR[OPER_ALL][8] = {"Read", "Write", "Erase", "Total"};
    for (int i=0;i<OPER_ALL;i++)
    {

    	this->exclusive_add(i,(Access_Capacity->vals[i].cnt - Access_Capacity->vals[i].sampled_cnt)*1.0/((ExecutionTime - LastExecutionTime)*1.0/PSEC));
        if(vals[i].cnt == 0)
        {
        	printf( "PAM: %s IOPS including idle time (min, max, average): ( NO DATA )\n", OPER_STR[i]);
        }
        else
        {
        	printf( "PAM: %s IOPS including idle time (min, max, average): %.6lf, %.6lf, %.6lf\n",
                   OPER_STR[i],
				   vals[i].minval,
                   vals[i].maxval,
				   (Access_Capacity->vals[i].cnt)*1.0/((ExecutionTime)*1.0/PSEC));
        }
    }
}

void PAMStatistics::ValueOper::printstat_oper_iops(class ValueOper* Access_Capacity, uint64* OpBusyTime, uint64* LastOpBusyTime)
{
    char OPER_STR[OPER_ALL][8] = {"Read", "Write", "Erase", "Total"};
    for (int i=0;i<3;i++) //only read/write/erase
    {
    	if (OpBusyTime[i] > LastOpBusyTime[i])
    		this->exclusive_add(i,(Access_Capacity->vals[i].cnt - Access_Capacity->vals[i].sampled_cnt)*1.0/((OpBusyTime[i] - LastOpBusyTime[i])*1.0/PSEC));
        if(vals[i].cnt == 0)
        {
        	printf( "PAM: %s-only IOPS (min, max, average): ( NO DATA )\n", OPER_STR[i]);
        }
        else
        {
        	printf( "PAM: %s-only IOPS (min, max, average): %.6lf, %.6lf, %.6lf\n",
                   OPER_STR[i],
				   vals[i].minval,
                   vals[i].maxval,
				   (Access_Capacity->vals[i].cnt)*1.0/((OpBusyTime[i])*1.0/PSEC));
        }
    }
}

void PAMStatistics::ValueOper::printstat_latency(const char* namestr)
{
    char OPER_STR[OPER_ALL][8] = {"Read", "Write", "Erase", "Total"};
    //DPRINTF(PAM2, "[ %s ]:\n", namestr);
    //DPRINTF(PAM2, "PAM: OPER, AVERAGE (B), COUNT, TOTAL (B), MIN (B), MAX(B)\n");
    for (int i=0;i<OPER_ALL;i++)
    {
        if(vals[i].cnt == 0)
        {
        	printf( "PAM: %s latency (min, max, average): ( NO DATA )\n", OPER_STR[i]);
        }
        else
        {
        	printf( "PAM: %s latency (min, max, average): %llu us, %llu us, %llu us\n",
                   OPER_STR[i],
				   (uint64)(vals[i].minval*1.0/1000000),
				   (uint64)(vals[i].maxval*1.0/1000000),
				   (uint64)(vals[i].avg()*1.0/1000000));
        }
    }
}
#endif // Polished stats


PAMStatistics::PAMStatistics()
{
    LastTick = 0;

    InitStats();

    SampledTick = 0;
    skip = true;
}

PAMStatistics::~PAMStatistics()
{
    PrintStats( sim->GetTick() );
    ClearStats();
}

void PAMStatistics::ResetStats()
{
    ClearStats();
    InitStats();
}


void PAMStatistics::InitStats()
{
    sim_start_time_ps = sim->GetTick();


    #if 0 //ch-die io count (legacy)
    ch_io_cnt = new mini_cnt_page[gconf->NumChannel];
    die_io_cnt = new mini_cnt_page[gconf->GetTotalNumDie()];
    #endif

    #if 0 //latency (legacy method)
    memset(latency_cnt,0,sizeof(latency_cnt));
    memset(latency_sum,0,sizeof(latency_sum));
    #endif

    #if 0 //stall
    memset(stall_sum,0,sizeof(stall_sum));
    memset(stall_cnt,0,sizeof(stall_cnt));
    #endif

    #if 0 //util time (legacy method)
    channel_util_time_sum = new uint64[gconf->NumChannel];
    memset(channel_util_time_sum, 0, sizeof(uint64)*gconf->NumChannel);

    die_util_time_sum_mem = new uint64[gconf->GetTotalNumDie()];
    memset(die_util_time_sum_mem, 0, sizeof(uint64)*gconf->GetTotalNumDie());

    die_util_time_sum_optimum = new uint64[gconf->GetTotalNumDie()];
    memset(die_util_time_sum_optimum, 0, sizeof(uint64)*gconf->GetTotalNumDie());

    die_util_time_sum_all = new uint64[gconf->GetTotalNumDie()];
    memset(die_util_time_sum_all, 0, sizeof(uint64)*gconf->GetTotalNumDie());
    #endif

    #if GATHER_RESOURCE_CONFLICT && 0 //conflict gather (legacy)
    ch_conflict_cnt = new mini_cnt_conflict[gconf->NumChannel];
    ch_conflict_length = new mini_cnt_conflict[gconf->NumChannel];
    die_conflict_cnt = new mini_cnt_conflict[gconf->GetTotalNumDie()];
    die_conflict_length = new mini_cnt_conflict[gconf->GetTotalNumDie()];
    #endif

    ExactBusyTime = 0;
    LastExactBusyTime = 0;
    LastExecutionTime = 0;
    OpBusyTime[0] =  OpBusyTime[1] = OpBusyTime[2] = 0;
    LastOpBusyTime[0] = LastOpBusyTime[1] =LastOpBusyTime[2] = 0;


  #if 1 // Polished stats - Improved instrumentation
    PPN_requested_ch  = new CounterOper[gconf->NumChannel];
    PPN_requested_die = new CounterOper[gconf->GetTotalNumDie()];
    Ticks_Active_ch   = new ValueOper[gconf->NumChannel];
    Ticks_Active_die  = new ValueOper[gconf->GetTotalNumDie()];

    PPN_requested_rwe.init();
    for (int j=0;j<PAGE_ALL;j++) { PPN_requested_pagetype[j].init(); };
    //PPN_requested_ch
    //PPN_requested_die
    CF_DMA0_dma.init();
    CF_DMA0_mem.init();
    CF_DMA0_none.init();
    CF_DMA1_dma.init();
    CF_DMA1_none.init();

    Ticks_DMA0WAIT.init();
    Ticks_DMA0.init();
    Ticks_MEM.init();
    Ticks_DMA1WAIT.init();
    Ticks_DMA1.init();
    Ticks_Total.init();
    Ticks_TotalOpti.init();
    //Ticks_Active_ch
    //Ticks_Active_die
    Access_Capacity.init();
    Access_Bandwidth.init();
    Access_Bandwidth_widle.init();
    Access_Oper_Bandwidth.init();
    Access_Iops.init();
    Access_Iops_widle.init();
    Access_Oper_Iops.init();
   #endif // Polished stats
}

void PAMStatistics::ClearStats()
{
    #if 0 //util time (legacy method)
    delete channel_util_time_sum;
    delete die_util_time_sum_mem;
    delete die_util_time_sum_optimum;
    delete die_util_time_sum_all;
    #endif

    #if 0 //ch-die io count (legacy)
    delete ch_io_cnt;
    delete die_io_cnt;
    #endif

    #if GATHER_RESOURCE_CONFLICT && 0 //conflict gather (legacy)
    delete ch_conflict_cnt;
    delete ch_conflict_length;
    delete die_conflict_cnt;
    delete die_conflict_length;
    #endif

  #if 1 // Polished stats - Improved instrumentation
    delete PPN_requested_ch;
    delete PPN_requested_die;
    delete Ticks_Active_ch;
    delete Ticks_Active_die;
  #endif // Polished stats
}


#if 0 //latency (legacy method)
/*
void PAMStatistics::AddLatency(uint32 lat, uint8 oper, uint8 busyfor)
{
    latency_sum[oper][busyfor] += lat;
    latency_cnt[oper][busyfor] += 1;
}
*/
#endif

void PAMStatistics::UpdateLastTick(uint64 tick)
{
    if (LastTick<tick) LastTick=tick;
}

uint64 PAMStatistics::GetLastTick()
{
    return LastTick;
}

void PAMStatistics::MergeSnapshot()
{
	if (Ticks_Total_snapshot.size() != 0){
		std::map<uint64, ValueOper*>::iterator e  = Ticks_Total_snapshot.end();
		e--;
		for (std::map<uint64, ValueOper*>::iterator f = Ticks_Total_snapshot.begin(); f != e;){
			delete f->second;
			Ticks_Total_snapshot.erase(f++);
		}

	}
	if (Access_Capacity_snapshot.size() != 0){
		std::map<uint64, ValueOper*>::iterator e  = Access_Capacity_snapshot.end();
		e--;
		for (std::map<uint64, ValueOper*>::iterator f = Access_Capacity_snapshot.begin(); f != e;){
			delete f->second;
			Access_Capacity_snapshot.erase(f++);
		}
	}
}

#if GATHER_RESOURCE_CONFLICT
void PAMStatistics::AddLatency(Command* CMD, CPDPBP* CPD, uint32 dieIdx, TimeSlot* DMA0, TimeSlot* MEM, TimeSlot* DMA1, uint8 confType, uint64 confLength)
#else
void PAMStatistics::AddLatency(Command* CMD, CPDPBP* CPD, uint32 dieIdx, TimeSlot* DMA0, TimeSlot* MEM, TimeSlot* DMA1)
#endif
{
    uint32 oper = CMD->operation;
    uint32 chIdx = CPD->Channel;
    uint64 time_all[TICK_STAT_NUM];
    uint8 pageType = lat->GetPageType(CPD->Page);
    memset(time_all,0,sizeof(time_all));

    /*
        TICK_IOREQUESTED, CMD->arrived_time
        TICK_DMA0WAIT, let it 0
        TICK_DMA0, DMA0->StartTick
        TICK_MEM,  MEM->StartTick
        TICK_DMA1WAIT, 
        TICK_DMA1, DMA1->StartTick
        TICK_IOEND, DMA1->TickEnd
    */

    time_all[TICK_DMA0WAIT]     = DMA0->StartTick - CMD->arrived_time;                     // FETCH_WAIT --> when DMA0 couldn't start immediatly
    time_all[TICK_DMA0]         = lat->GetLatency(CPD->Page, CMD->operation, BUSY_DMA0);
    time_all[TICK_DMA0_SUSPEND] = 0;                                                       //no suspend in new design
    time_all[TICK_MEM]          = lat->GetLatency(CPD->Page, CMD->operation, BUSY_MEM);
    time_all[TICK_DMA1WAIT]     = ( MEM->EndTick - MEM->StartTick + 1 )
                                - (  lat->GetLatency(CPD->Page, CMD->operation, BUSY_DMA0)
                                   + lat->GetLatency(CPD->Page, CMD->operation, BUSY_MEM)
                                   + lat->GetLatency(CPD->Page, CMD->operation, BUSY_DMA1) ); // --> when DMA1 didn't start immediatly.
    time_all[TICK_DMA1]         = lat->GetLatency(CPD->Page, CMD->operation, BUSY_DMA1);
    time_all[TICK_DMA1_SUSPEND] = 0;                                                       //no suspend in new design
    time_all[TICK_FULL]         = DMA1->EndTick - CMD->arrived_time +1;  //D0W+D0+M+D1W+D1 full latency
    time_all[TICK_PROC]         = time_all[TICK_DMA0] + time_all[TICK_MEM] + time_all[TICK_DMA1];  // OPTIMUM_TIME

  #if 1 // Polished stats - Improved instrumentation
    PPN_requested_rwe.add(oper);  // { PPN_requested_rwe[oper].add(); PPN_requested_rwe[OPER_NUM].add(); }
    PPN_requested_pagetype[pageType].add(oper);    //PPN_requested_pagetype[PAGE_TYPE][i];
    PPN_requested_ch[chIdx].add(oper);  //PPN_requested_ch[ch#]
    PPN_requested_die[dieIdx].add(oper); //PPN_requested_die[die#]

    if (confType & CONFLICT_DMA0)
        CF_DMA0_dma.add(oper);   //{ CF_DMA0_dma[oper].add(); CF_DMA0_dma[OPER_NUM].add(); }
    if (confType & CONFLICT_MEM)
        CF_DMA0_mem.add(oper);   //{ CF_DMA0_mem[oper].add(); CF_DMA0_mem[OPER_NUM].add(); }
    if ( !(confType & (CONFLICT_DMA0|CONFLICT_MEM)) )
        CF_DMA0_none.add(oper);  //{ CF_DMA0_none[oper].add(); CF_DMA0_none[OPER_NUM].add(); }

    if ( confType & CONFLICT_DMA1 )
        CF_DMA1_dma.add(oper);   //{ CF_DMA1_dma[oper].add(); CF_DMA1_dma[OPER_NUM].add(); }
    if ( !(confType & CONFLICT_DMA1) )
        CF_DMA1_none.add(oper);  //{ CF_DMA1_none[oper].add(); CF_DMA1_none[OPER_NUM].add(); }

    Ticks_DMA0WAIT.add(oper, time_all[TICK_DMA0WAIT]);  //Ticks_DMA0WAIT[i].init();
    Ticks_DMA0.add(oper, time_all[TICK_DMA0]);
    Ticks_MEM.add(oper, time_all[TICK_MEM]);
    Ticks_DMA1WAIT.add(oper, time_all[TICK_DMA1WAIT]);
    Ticks_DMA1.add(oper, time_all[TICK_DMA1]);
    Ticks_Total.add(oper, time_all[TICK_FULL]);
    //***********************************************
    uint64 finished_time = CMD->finished_time;
    uint64 update_point = finished_time / EPOCH_INTERVAL;
    //printf("finished_time= %llu\n", finished_time / 100000000000);
    std::map<uint64, ValueOper*>::iterator e  = Ticks_Total_snapshot.find(update_point);
    if (e != Ticks_Total_snapshot.end()) e->second->add(oper, time_all[TICK_FULL]);
    else{
    	/*
    	for (uint64 regress_point = 0; regress_point <= update_point; regress_point++){
    		e  = Ticks_Total_snapshot.find(regress_point);
    		if (e == Ticks_Total_snapshot.end()){
    			if (regress_point == 0) Ticks_Total_snapshot[regress_point] = new ValueOper();
    			else Ticks_Total_snapshot[regress_point] = new ValueOper(Ticks_Total_snapshot[regress_point-1]);
    		}
    	}

    	*/

/*    	if(update_point == 0) Ticks_Total_snapshot[update_point] = new ValueOper();
    	else{
    		e = Ticks_Total_snapshot.end();
    		e--;
    		uint64 regress_point = e->first + 1;
    		while (regress_point <= update_point){
    			//printf("++++++++++Ticks_Total_snapshot=%llu\n",regress_point);
    			Ticks_Total_snapshot[regress_point] = new ValueOper(Ticks_Total_snapshot[regress_point-1]);
    			regress_point++;
    		}
    	}
    	*/

    	e = Ticks_Total_snapshot.upper_bound(update_point);
    	if (e != Ticks_Total_snapshot.end()){
    		if (e != Ticks_Total_snapshot.begin()){
    			e--;
    			Ticks_Total_snapshot[update_point] = new ValueOper(e->second);
    		}
    		else{
    			Ticks_Total_snapshot[update_point] = new ValueOper();
    		}

    	}
    	else{
    		if (Ticks_Total_snapshot.size() == 0)Ticks_Total_snapshot[update_point] = new ValueOper();
    		else {
    			e--;
    			Ticks_Total_snapshot[update_point] = new ValueOper(e->second);
    		}
    	}
    	Ticks_Total_snapshot[update_point]->add(oper, time_all[TICK_FULL]);
    }
    //update_point++;
    e  = Ticks_Total_snapshot.upper_bound(update_point);
    while (e != Ticks_Total_snapshot.end()){
    	e->second->add(oper, time_all[TICK_FULL]);
    	//update_point++;
    	e  = Ticks_Total_snapshot.upper_bound(e->first);
    }
    //***********************************************
    Ticks_TotalOpti.add(oper, time_all[TICK_PROC]);
    Ticks_Active_ch[chIdx].add( oper,time_all[TICK_DMA0]+time_all[TICK_DMA1] );
    Ticks_Active_die[dieIdx].add(oper, (time_all[TICK_DMA0] + time_all[TICK_MEM] + time_all[TICK_DMA1WAIT] + time_all[TICK_DMA1]) );
    if (oper == OPER_ERASE) Access_Capacity.add(oper, gconf->SizePage*gconf->NumPage); // ERASE
    else                    Access_Capacity.add(oper, gconf->SizePage);                // READ,WRITE
    //************************************************
    update_point = finished_time / EPOCH_INTERVAL;
    std::map<uint64, ValueOper*>::iterator f  = Access_Capacity_snapshot.find(update_point);
    if (f != Access_Capacity_snapshot.end()){
    	if (oper == OPER_ERASE) f->second->add(oper, gconf->SizePage*gconf->NumPage); // ERASE
    	else                    f->second->add(oper, gconf->SizePage);                // READ,WRITE
    }
    else{
    	/*
    	for (uint64 regress_point = 0; regress_point <= update_point; regress_point++){
    		e  = Access_Capacity_snapshot.find(regress_point);
    		if (e == Access_Capacity_snapshot.end()){
    			if (regress_point == 0) Access_Capacity_snapshot[regress_point] = new ValueOper();
    			else Access_Capacity_snapshot[regress_point] = new ValueOper(Access_Capacity_snapshot[regress_point-1]);
    		}

    	}
		*/

    	/*
    	if(update_point == 0) Access_Capacity_snapshot[update_point] = new ValueOper();
    	else{
    		e = Access_Capacity_snapshot.end();
    		e--;
    		uint64 regress_point = e->first + 1;
    		while (regress_point <= update_point){
    			//printf("------------Ticks_Total_snapshot=%llu\n",regress_point);
    			Access_Capacity_snapshot[regress_point] = new ValueOper(Access_Capacity_snapshot[regress_point-1]);
    			regress_point++;
    		}

    	}
    	*/

    	f = Access_Capacity_snapshot.upper_bound(update_point);
    	if (f != Access_Capacity_snapshot.end()){
    		if (f != Access_Capacity_snapshot.begin()){
    			f--;
    			Access_Capacity_snapshot[update_point] = new ValueOper(f->second);
    		}
    		else{
    			Access_Capacity_snapshot[update_point] = new ValueOper();
    		}

    	}
    	else{
    		if (Access_Capacity_snapshot.size() == 0)Access_Capacity_snapshot[update_point] = new ValueOper();
    		else {
    			f--;
    			Access_Capacity_snapshot[update_point] = new ValueOper(f->second);
    		}
    	}

    	if (oper == OPER_ERASE) Access_Capacity_snapshot[update_point]->add(oper, gconf->SizePage*gconf->NumPage); // ERASE
    	else                    Access_Capacity_snapshot[update_point]->add(oper, gconf->SizePage);                // READ,WRITE
    }
    //update_point++;
    f  = Access_Capacity_snapshot.upper_bound(update_point);
    while (f != Access_Capacity_snapshot.end()){
    	if (oper == OPER_ERASE) f->second->add(oper, gconf->SizePage*gconf->NumPage); // ERASE
    	else                    f->second->add(oper, gconf->SizePage);                // READ,WRITE
    	//update_point++;
    	f  = Access_Capacity_snapshot.upper_bound(f->first);
    }
//    for (f=Access_Capacity_snapshot.begin(); f!=Access_Capacity_snapshot.end(); f++)
//    	printf("count=%f\n",Access_Capacity_snapshot[0]->vals[3].cnt);
    //************************************************
  #endif  // Polished stats


    #if 0 //latency (legacy method)
    for (uint32 i=0;i<TICK_STAT_NUM; i++)
    {
        latency_sum[CMD->operation][i] += time_all[i];
        latency_cnt[CMD->operation][i] += 1;
    }
    #endif

    #if 0 //ch-die io count (legacy)
    ch_io_cnt [CPD->Channel].c[CMD->operation][pageType]++;
    die_io_cnt[dieIdx      ].c[CMD->operation][pageType]++;
    #endif

    #if GATHER_RESOURCE_CONFLICT && 0 //conflict gather (legacy)
    if  (confType == CONFLICT_NONE )
    {
        ch_conflict_cnt    [CPD->Channel].c[CMD->operation][CONFLICT_NONE]++;
        ch_conflict_length [CPD->Channel].c[CMD->operation][CONFLICT_NONE]+=(confLength);
        die_conflict_cnt   [dieIdx      ].c[CMD->operation][CONFLICT_NONE]++;
        die_conflict_length[dieIdx      ].c[CMD->operation][CONFLICT_NONE]+=(confLength);
    }
    else
    {
        if (confType & CONFLICT_DMA0)
        {
            ch_conflict_cnt    [CPD->Channel].c[CMD->operation][1]++;
            ch_conflict_length [CPD->Channel].c[CMD->operation][1]+=(confLength);
            die_conflict_cnt   [dieIdx      ].c[CMD->operation][1]++;
            die_conflict_length[dieIdx      ].c[CMD->operation][1]+=(confLength);
        }

        if (confType & CONFLICT_MEM)
        {
            ch_conflict_cnt    [CPD->Channel].c[CMD->operation][2]++;
            ch_conflict_length [CPD->Channel].c[CMD->operation][2]+=(confLength);
            die_conflict_cnt   [dieIdx      ].c[CMD->operation][2]++;
            die_conflict_length[dieIdx      ].c[CMD->operation][2]+=(confLength);
        }

        if (confType & CONFLICT_DMA1)
        {
            ch_conflict_cnt    [CPD->Channel].c[CMD->operation][3]++;
            ch_conflict_length [CPD->Channel].c[CMD->operation][3]+=(confLength);
            die_conflict_cnt   [dieIdx      ].c[CMD->operation][3]++;
            die_conflict_length[dieIdx      ].c[CMD->operation][3]+=(confLength);
        }
    }
    #endif

     /*#if LOG_PRINT_CONSUMED_TIME
       //Consumed Time
       DPRINTF(PAM2, "CH%d %s PPN_0x%llX, DMA0WAIT, %llu, DMA0, %llu, MEM, %llu, DMA1WAIT, %llu, DMA1, %llu, SUM, %llu\n",
              task->CPD.Channel, OPER_STRINFO[task->Oper], task->PPN,
              Time_DMA0WAIT, Time_DMA0, Time_MEM, Time_DMA1WAIT, Time_DMA1, Time_SUM );
     #endif*/

     #if 0 //util time (legacy method)
     channel_util_time_sum[CPD->Channel] += ( time_all[TICK_DMA0] + time_all[TICK_DMA1] );
     //DPRINTF(PAM2, "CH=%d , sum=%llu , Ticks-D0=%llu , Ticks-D1=%llu\n", CPD->Channel, channel_util_time_sum[CPD->Channel], time_all[TICK_DMA0], time_all[TICK_DMA1]);
     die_util_time_sum_mem[dieIdx]  += ( time_all[TICK_MEM] );      //Only MEM Oper
     die_util_time_sum_optimum[dieIdx] += ( time_all[TICK_PROC] );  //Optimum DMA0+MEM+DMA1, without any between-delay
     die_util_time_sum_all[dieIdx]  += ( DMA1->EndTick - DMA0->StartTick +1 );     //Die occupy(DMA0~DMA1END) time
     #endif


     #if 0 //stall
     if( time_all[TICK_DMA1WAIT] != 0 )
     {
         CountStall(CMD->operation, TICK_DMA1WAIT);                    //stall cnt  2
         AddStall(CMD->operation, TICK_DMA1WAIT, time_all[TICK_DMA1]); //stall time 2
     }

     //Note: Can NOT count DMA0, DMA1 stall COUNT here. (Only can get stall time.)
     {
         AddStall(CMD->operation, TICK_DMA0_SUSPEND, time_all[TICK_DMA0_SUSPEND]); //stall time 3
         AddStall(CMD->operation, TICK_DMA1_SUSPEND, time_all[TICK_DMA1_SUSPEND]); //stall time 4
     }
     #endif
}

#if 0
void PAMStatistics::AddLatency(Task* task)
{
    uint64* ticks = task->TickStart;
    uint64 time_all[TICK_STAT_NUM];
    memset(time_all,0,sizeof(time_all));

    time_all[TICK_IOREQUESTED]  = ticks[TICK_DMA0WAIT] - ticks[TICK_IOREQUESTED];
    time_all[TICK_DMA0WAIT]     = ticks[TICK_DMA0]     - ticks[TICK_DMA0WAIT];
    time_all[TICK_DMA0]         = lat->GetLatency(task->CPD.Page, task->Oper, BUSY_DMA0);
    time_all[TICK_DMA0_SUSPEND] = (ticks[TICK_MEM]      - ticks[TICK_DMA0]) - time_all[TICK_DMA0];
    time_all[TICK_MEM]          = ticks[TICK_DMA1WAIT] - ticks[TICK_MEM];
    time_all[TICK_DMA1WAIT]     = ticks[TICK_DMA1]     - ticks[TICK_DMA1WAIT];
    time_all[TICK_DMA1]         = lat->GetLatency(task->CPD.Page, task->Oper, BUSY_DMA1);
    time_all[TICK_DMA1_SUSPEND] = (ticks[TICK_IOEND]      - ticks[TICK_DMA1]) - time_all[TICK_DMA1];
    time_all[TICK_FULL]         = ticks[TICK_IOEND]      - ticks[TICK_DMA0];
    time_all[TICK_PROC]         = time_all[TICK_DMA0] + time_all[TICK_MEM] + time_all[TICK_DMA1];

    for (uint32 i=0;i<TICK_STAT_NUM; i++)
    {
        latency_sum[task->Oper][i] += time_all[i];
        latency_cnt[task->Oper][i] += 1;
    }

     /*#if LOG_PRINT_CONSUMED_TIME
       //Consumed Time
       DPRINTF(PAM2, "CH%d %s PPN_0x%llX, DMA0WAIT, %llu, DMA0, %llu, MEM, %llu, DMA1WAIT, %llu, DMA1, %llu, SUM, %llu\n",
              task->CPD.Channel, OPER_STRINFO[task->Oper], task->PPN,
              Time_DMA0WAIT, Time_DMA0, Time_MEM, Time_DMA1WAIT, Time_DMA1, Time_SUM );
     #endif*/

     #if 0 //util time (legacy method)
     channel_util_time_sum[task->CPD.Channel] += ( time_all[TICK_DMA0] + time_all[TICK_DMA1] );
     die_util_time_sum_mem[task->PlaneIdx]  += ( time_all[TICK_MEM] );
     die_util_time_sum_all[task->PlaneIdx]  += ( time_all[TICK_DMA0] + time_all[TICK_DMA0_SUSPEND] + 
                                                     time_all[TICK_MEM] + 
                                                     time_all[TICK_DMA1WAIT] + 
                                                     time_all[TICK_DMA1] + time_all[TICK_DMA0_SUSPEND] );
     #endif

     if( time_all[TICK_DMA1WAIT] != 0 )
     {
         CountStall(task->Oper, TICK_DMA1WAIT);                    //stall cnt  2
         AddStall(task->Oper, TICK_DMA1WAIT, time_all[TICK_DMA1]); //stall time 2
     }

     //Note: Can NOT count DMA0, DMA1 stall COUNT here. (Only can get stall time.)
     {
         AddStall(task->Oper, TICK_DMA0_SUSPEND, time_all[TICK_DMA0_SUSPEND]); //stall time 3
         AddStall(task->Oper, TICK_DMA1_SUSPEND, time_all[TICK_DMA1_SUSPEND]); //stall time 4
     }
}
#endif


#if 0 //stall
void PAMStatistics::CountStall(uint8 oper, uint8 stall_kind)
{
    stall_cnt[oper][stall_kind] += 1;
}

void PAMStatistics::AddStall(uint8 oper, uint8 stall_kind, uint64 lat)
{
    stall_sum[oper][stall_kind] += lat;
}
#endif

/*
void PAMStatistics::AddOccupy(uint32 ch, uint64 ch_time, uint64 pl, uint64 pl_time, uint64 pl2_time)
{
    channel_util_time_sum[ch] += ch_time;
    die_util_time_sum[pl] += pl_time;
    die_util_time_sum2[pl] += pl2_time;

    //DPRINTF(PAM2, "ch_time = %llu, pl_time = %llu\n", ch_time, pl_time);
} 
*/ 

//#define fDPRINTF(out_to, fmt...) do { char buf[1024]; sprintf(buf, fmt); DPRINTF(out_to,"%s",buf); } while(0);

void PAMStatistics::PrintFinalStats(uint64 sim_time_ps){
	printf( "[ PAM Final Stats Report ]\n");
	printf( "PAM: Total execution time (ms), Total SSD active time (ms)\n");
	printf( "PAM: %.2f\t\t\t, %.2f\n", sim_time_ps * 1.0 / 1000000000, SampledExactBusyTime * 1.0 / 1000000000);

	//std::map<uint64, ValueOper*>::iterator e  = Access_Capacity_snapshot.find(sim_time_ps/EPOCH_INTERVAL);
	//assert(e != Access_Capacity_snapshot.end());

	assert(Access_Capacity_snapshot.size() > 0);
	std::map<uint64, ValueOper*>::iterator e  = Access_Capacity_snapshot.end();
	e--;

	e->second->printstat("Info of Access Capacity");
	Access_Bandwidth.printstat_bandwidth(e->second, SampledExactBusyTime, LastExactBusyTime);
	Access_Bandwidth_widle.printstat_bandwidth_widle(e->second, sim_time_ps, LastExecutionTime);
	Access_Oper_Bandwidth.printstat_oper_bandwidth(e->second, OpBusyTime, LastOpBusyTime);

	//std::map<uint64, ValueOper*>::iterator f  = Ticks_Total_snapshot.find(sim_time_ps/EPOCH_INTERVAL);

	assert(Ticks_Total_snapshot.size() > 0);
	std::map<uint64, ValueOper*>::iterator f  = Ticks_Total_snapshot.end();
	f--;

	f->second->printstat_latency("Info of Latency");
	Access_Iops.printstat_iops(e->second, SampledExactBusyTime, LastExactBusyTime);
	Access_Iops_widle.printstat_iops_widle(e->second, sim_time_ps, LastExecutionTime);
	Access_Oper_Iops.printstat_oper_iops(e->second, OpBusyTime, LastOpBusyTime);
	printf( "===================\n");
    PPN_requested_rwe.printstat("Num of PPN IO request");
    printf( "===================\n");

    for(uint32 i=0; i<PAGE_NUM; i++)
    {
        char str[256];
        sprintf(str, "Num of %s page PPN IO request", PAGE_STRINFO[i]);
        PPN_requested_pagetype[i].printstat(str);
    }
    printf( "===================\n");

    for (uint32 i=0;i<gconf->NumChannel; i++)
    {
        char str[256];
        sprintf(str, "Num of CH_%u PPN IO request", i);
        PPN_requested_ch[i].printstat(str);
    }
    printf( "===================\n");

    for (uint32 i=0; i<gconf->GetTotalNumDie(); i++)
    {
        char str[256];
        sprintf(str, "Num of DIE_%u PPN IO request", i);
        PPN_requested_die[i].printstat(str);
    }
    printf( "===================\n");

    CF_DMA0_dma.printstat("Num of conflict DMA0-CH");
    CF_DMA0_mem.printstat("Num of conflict DMA0-MEM");
    CF_DMA0_none.printstat("Num of conflict DMA0-None");
    printf( "===================\n");

    CF_DMA1_dma.printstat("Num of conflict DMA1-CH");
    CF_DMA1_none.printstat("Num of conflict DMA1-None");
    printf( "===================\n");

    Ticks_DMA0WAIT.printstat("Info of DMA0WAIT Tick");
    Ticks_DMA0.printstat("Info of DMA0 Tick");
    Ticks_MEM.printstat("Info of MEM Tick");
    Ticks_DMA1WAIT.printstat("Info of DMA1WAIT Tick");
    Ticks_DMA1.printstat("Info of DMA1 Tick");
    Ticks_Total.printstat("Info of TOTAL(D0W+D0+M+D1W+D1) Tick");
    Ticks_TotalOpti.printstat("Info of OPTIMUM(D0+M+D1) Tick");
    printf( "===================\n");

    for (uint32 i=0;i<gconf->NumChannel; i++)
    {
        char str[256];
        sprintf(str, "Info of CH_%u Active Tick", i);
        Ticks_Active_ch[i].printstat(str);
    }
    printf( "===================\n");

    for (uint32 i=0; i<gconf->GetTotalNumDie(); i++)
    {
        char str[256];
        sprintf(str, "Info of DIE_%u Active Tick", i);
        Ticks_Active_die[i].printstat(str);
    }
    printf( "===================\n");


}

void PAMStatistics::PrintStats(uint64 sim_time_ps)
{
	//DPRINTF(PAM2,"cur_tick=%llu\n",sim_time_ps);
    uint64 elapsed_time_ps = (sim_time_ps - sim_start_time_ps) + 1;
    if (LastExecutionTime == 0)
    	LastExecutionTime = sim_start_time_ps;
    printf("Execution time = %llu\n",sim_time_ps);
    printf("Last Execution time = %llu\n",LastExecutionTime);
    if (sim_start_time_ps >= sim_time_ps) //abnormal case
    {
        elapsed_time_ps = sim_time_ps + 1;
    }

    printf( "[ PAM Stats ]\n");

  #if 0 //latency (legacy method)
    //RWE size
    uint64 page_byte[OPER_NUM] =
    {
        latency_cnt[OPER_READ][TICK_FULL]*gconf->SizePage,
        latency_cnt[OPER_WRITE][TICK_FULL]*gconf->SizePage,
        latency_cnt[OPER_ERASE][TICK_FULL]*gconf->NumPage*gconf->SizePage
    };

    for (int i=0;i<OPER_NUM;i++)
    {
        #if 0 // BYTES
        DPRINTF(PAM2, "%s : %llu Bytes\n", OPER_STRINFO[i], page_byte[i]);
        #else //MBYTES
        DPRINTF(PAM2, "%s : %Lf MB\n", OPER_STRINFO[i], (long double)page_byte[i]/MBYTE);
        #endif

        if( latency_cnt[i][TICK_FULL] > 0)
        {
            DPRINTF(PAM2, "Average %s Full Latency: %llu ps\n", OPER_STRINFO[i], latency_sum[i][TICK_FULL]/latency_cnt[i][TICK_FULL]);
        }
        if( latency_cnt[i][TICK_PROC] > 0)
        {
            //DPRINTF(PAM2, "DBG: PROC lat_sum = %llu\nDBG: PROC lat_cnt = %llu\n", latency_sum[i][TICK_PROC], latency_cnt[i][TICK_PROC]);
            DPRINTF(PAM2, "Average %s Proc Latency: %llu ps\n", OPER_STRINFO[i], latency_sum[i][TICK_PROC]/latency_cnt[i][TICK_PROC]);
        }
    }
  #endif

    #if 0
        #define SIM_TIME_SEC ( (long double)elapsed_time_ps/PSEC )
        #define BUSY_TIME_SEC ( (long double)ExactBusyTime/PSEC )
        #define TRANSFER_TOTAL_MB ( (long double)(Access_Capacity.vals[OPER_READ].sum + Access_Capacity.vals[OPER_WRITE].sum)/MBYTE )

        #if 0 //latency (legacy method)
        DPRINTF(PAM2, "IO count: %llu\n", (latency_cnt[OPER_READ][TICK_NUM] + latency_cnt[OPER_WRITE][TICK_NUM] + latency_cnt[OPER_ERASE][TICK_NUM]) );
        #endif

        printf( "Sim.Time :  %Lf Sec. , %llu ps\n", SIM_TIME_SEC, elapsed_time_ps);
        printf("Transferred :  %Lf MB\n", TRANSFER_TOTAL_MB);
        printf( "Performance: %Lf MB/Sec\n", (long double)TRANSFER_TOTAL_MB/SIM_TIME_SEC);
        printf( "Busy Sim.Time: %Lf Sec. , %llu ps\n", BUSY_TIME_SEC, ExactBusyTime);
        printf("Busy Performance: %Lf MB/Sec\n", (long double)TRANSFER_TOTAL_MB/BUSY_TIME_SEC);
    #endif

  #if 0 //stall
    // *** Stall/Conflict Stat
    for (int i=0;i<OPER_NUM;i++)
    {
        uint8 idxs[5]={ TICK_DMA0_CHANNEL_CONFLICT, TICK_DMA0_PLANE_CONFLICT, TICK_DMA1WAIT, TICK_DMA0_SUSPEND, TICK_DMA1_SUSPEND };
        char STAT_STRS[5][10] = {"DMA0-CH  ", "DMA0-MEM ", "DMA1-CH  ", "DMA0-SUSP", "DMA1-SUSP"};

        //uint64 sumcnt=0;
        /*
        for (int j=0;j<5;j++)
        {
            int k = idxs[j];
            sumcnt+=stall_cnt[i][k];
        }
        if (sumcnt==0) continue;
        */
        for (int j = 0; j < 5; j++)
        {
            int k = idxs[j];
            DPRINTF(PAM2, "%s Stall %s Count: %llu, Total Time: %llu\n", OPER_STRINFO2[i], STAT_STRS[j], stall_cnt[i][k], stall_sum[i][k] );
        }
    }
  #endif


  #if 0 //util time (legacy method)
    // *** Channel Occupy Stat
    uint64 channel_util_time_total = 0;
    for (int i=0;i<gconf->NumChannel;i++)
    {
        #if 1 //LOG_PRINT_util_EACH
        DPRINTF(PAM2, "Ch%3d UtilRatio: %Lf ( UtilTime %Lf sec, IdleTime %Lf sec )\n",
               i,
               (long double) channel_util_time_sum[i] / elapsed_time_ps,
               (long double) channel_util_time_sum[i] / PSEC,
               (long double) (elapsed_time_ps-channel_util_time_sum[i])/PSEC );
        #endif
        channel_util_time_total += channel_util_time_sum[i];
    }
    long double channel_util_time_avg = (long double)channel_util_time_total / (long double)gconf->NumChannel;

    // *** die Occupy Stat - mem
    uint64 die_util_time_total_mem = 0;
    for (int i=0;i<gconf->GetTotalNumDie();i++)
    {
        #if 1//LOG_PRINT_util_EACH
        DPRINTF(PAM2, "Die%5d MemOnly UtilRatio: %Lf ( UtilTime %Lf sec, IdleTime %Lf sec )\n",
               i,
               (long double) die_util_time_sum_mem[i] / elapsed_time_ps,
               (long double) die_util_time_sum_mem[i] / PSEC,
               (long double) (elapsed_time_ps-die_util_time_sum_mem[i])/PSEC );
        #endif
        die_util_time_total_mem += die_util_time_sum_mem[i];
    }
    long double die_util_time_avg_mem = (long double)die_util_time_total_mem / (long double)gconf->GetTotalNumDie();

    // *** die Occupy Stat - optimum
    uint64 die_util_time_total_optimum = 0;
    for (int i=0;i<gconf->GetTotalNumDie();i++)
    {
        #if 1//LOG_PRINT_util_EACH
        DPRINTF(PAM2, "Die%5d Optimum UtilRatio: %Lf ( UtilTime %Lf sec, IdleTime %Lf sec )\n",
               i,
               (long double) die_util_time_sum_optimum[i] / elapsed_time_ps,
               (long double) die_util_time_sum_optimum[i] / PSEC,
               (long double) (elapsed_time_ps-die_util_time_sum_optimum[i])/PSEC );
        #endif
        die_util_time_total_optimum += die_util_time_sum_optimum[i];
    }
    long double die_util_time_avg_optimum = (long double)die_util_time_total_optimum / (long double)gconf->GetTotalNumDie();

    // *** Die Occupy Stat - all
    uint64 die_util_time_total_all = 0;
    for (int i=0;i<gconf->GetTotalNumDie();i++)
    {
        #if 1//LOG_PRINT_util_EACH
        DPRINTF(PAM2, "Die%5d Actual UtilRatio: %Lf ( UtilTime %Lf sec, IdleTime %Lf sec )\n",
               i,
               (long double) die_util_time_sum_all[i] / elapsed_time_ps,
               (long double) die_util_time_sum_all[i] / PSEC,
               (long double) (elapsed_time_ps-die_util_time_sum_all[i])/PSEC );
        #endif
        die_util_time_total_all += die_util_time_sum_all[i];
    }
    long double die_util_time_avg_all = (long double)die_util_time_total_all / (long double)gconf->GetTotalNumDie();
  #endif


  #if 0 //latency (legacy method)
    //Averages
    for (int i=0;i<OPER_NUM;i++)
    {
        // DPRINTF(PAM2, "DBG: MEM = %llu / %llu\n", latency_sum[i][BUSY_MEM], latency_cnt[i][BUSY_MEM]);
        // DPRINTF(PAM2, "DBG: LCM = %llu, %llu, %llu\n", ((LatencyTLC*)lat)->MEM_CNT[0], ((LatencyTLC*)lat)->MEM_CNT[1], ((LatencyTLC*)lat)->MEM_CNT[2]);
      #if DMA_PREEMPTION
        DPRINTF(PAM2, "* Avg %s LATENCY, DMA0WAIT, %llu, DMA0, %llu, DMA0SUSP, %llu, MEM, %llu, DMA1WAIT, %llu, DMA1, %llu, DMA1SUSP, %llu\n",
               OPER_STRINFO[i],
               SAFEDIV( latency_sum[i][TICK_DMA0WAIT]     , latency_cnt[i][TICK_DMA0WAIT]     ),
               SAFEDIV( latency_sum[i][TICK_DMA0]         , latency_cnt[i][TICK_DMA0]         ),
               SAFEDIV( latency_sum[i][TICK_DMA0_SUSPEND] , latency_cnt[i][TICK_DMA0_SUSPEND] ),
               SAFEDIV( latency_sum[i][TICK_MEM]          , latency_cnt[i][TICK_MEM]          ),
               SAFEDIV( latency_sum[i][TICK_DMA1WAIT]     , latency_cnt[i][TICK_DMA1WAIT]     ),
               SAFEDIV( latency_sum[i][TICK_DMA1]         , latency_cnt[i][TICK_DMA1]         ),
               SAFEDIV( latency_sum[i][TICK_DMA1_SUSPEND] , latency_cnt[i][TICK_DMA1_SUSPEND] ) );
      #else
        DPRINTF(PAM2, "* Avg %s LATENCY, DMA0WAIT, %llu, DMA0, %llu, MEM, %llu, DMA1WAIT, %llu, DMA1, %llu\n",
               OPER_STRINFO[i],
               latency_sum[i][TICK_DMA0WAIT] / latency_cnt[i][TICK_DMA0WAIT],
               latency_sum[i][TICK_DMA0]     / latency_cnt[i][TICK_DMA0],
               latency_sum[i][TICK_MEM]      / latency_cnt[i][TICK_MEM],
               latency_sum[i][TICK_DMA1WAIT] / latency_cnt[i][TICK_DMA1WAIT],
               latency_sum[i][TICK_DMA1]     / latency_cnt[i][TICK_DMA1]);
      #endif
    }
  #endif


  #if 0 //util time (legacy method)
    DPRINTF(PAM2, "* Ch Avg          UtilRatio: %Lf ( UtilTime %Lf sec, IdleTime %Lf sec )\n",
           (long double) channel_util_time_avg / elapsed_time_ps,
           (long double) channel_util_time_avg/PSEC,
           (long double) (elapsed_time_ps-channel_util_time_avg)/PSEC );
    DPRINTF(PAM2, "* Die Avg MemOnly UtilRatio: %Lf ( UtilTime %Lf sec, IdleTime %Lf sec )\n",
           (long double) die_util_time_avg_mem / elapsed_time_ps,
           (long double) die_util_time_avg_mem / PSEC,
           (long double) (elapsed_time_ps-die_util_time_avg_mem)/PSEC );
    DPRINTF(PAM2, "* Die Avg Optimum UtilRatio: %Lf ( UtilTime %Lf sec, IdleTime %Lf sec )\n",
           (long double) die_util_time_avg_optimum / elapsed_time_ps,
           (long double) die_util_time_avg_optimum / PSEC,
           (long double) (elapsed_time_ps-die_util_time_avg_optimum)/PSEC );
    DPRINTF(PAM2, "* Die Avg Actual  UtilRatio: %Lf ( UtilTime %Lf sec, IdleTime %Lf sec )\n",
           (long double) die_util_time_avg_all / elapsed_time_ps,
           (long double) die_util_time_avg_all / PSEC,
           (long double) (elapsed_time_ps-die_util_time_avg_all)/PSEC );
  #endif

  #if 0 //ch-die io count (legacy)
    //Page Type Prints===============================
    for (int c=0; c<gconf->NumChannel; c++)
    {
        uint64 allSum = 0;
        DPRINTF(PAM2, "\nCH %d IO-Counts:\n  LSB      CSB      MSB      SUM\n", c);
        for (int op=0;op<OPER_NUM;op++)
        {
            uint64 ptSum = 0;
            DPRINTF(PAM2, "%s ", OPER_STRINFO[op]);
            for (int pt=0;pt<PAGE_NUM;pt++)
            {
                DPRINTF(PAM2, "%8llu ", ch_io_cnt[c].c[op][pt]);
                ptSum += ( ch_io_cnt[c].c[op][pt] );
            }
            DPRINTF(PAM2, "%8llu\n", ptSum);
            allSum += ptSum;
        }
        DPRINTF(PAM2, "CH %d IO-SUM: %llu\n", c, allSum);
    }

    for (int d=0; d<gconf->GetTotalNumDie(); d++)
    {
        uint64 allSum = 0;
        DPRINTF(PAM2, "\nDIE %d IO-Counts:\n  LSB      CSB      MSB\n", d);
        for (int op=0;op<OPER_NUM;op++)
        {
            uint64 ptSum = 0;
            DPRINTF(PAM2, "%s ", OPER_STRINFO[op]);
            for (int pt=0;pt<PAGE_NUM;pt++)
            {
                DPRINTF(PAM2, "%8llu ", die_io_cnt[d].c[op][pt]);
                ptSum += ( die_io_cnt[d].c[op][pt] );
            }
            DPRINTF(PAM2, "%8llu\n", ptSum);
            allSum += ptSum;
        }
        DPRINTF(PAM2, "DIE %d IO-SUM: %llu\n", d, allSum);
    }
  #endif

  #if GATHER_RESOURCE_CONFLICT && 0 //conflict gather (legacy)
    //Conflict Prints - COUNT ========================================
    for (int c=0; c<gconf->NumChannel; c++)
    {
        uint64 allSum = 0;
        DPRINTF(PAM2, "\nCH %d Conflict-Counts:\n  NONE     DMA0     MEM      DMA1      SUM\n", c);
        for (int op=0;op<OPER_NUM;op++)
        {
            uint64 cfSum = 0;
            DPRINTF(PAM2, "%4s ", OPER_STRINFO[op]);
            for (int cf=0;cf<CONFLICT_NUM;cf++)
            {
                DPRINTF(PAM2, "%8llu ", ch_conflict_cnt[c].c[op][cf]);
                cfSum += ( ch_conflict_cnt[c].c[op][cf] );
            }
            DPRINTF(PAM2, "%8llu\n", cfSum);
            allSum += cfSum;
        }
        DPRINTF(PAM2, "CH %d CONF-CNT-SUM: %llu\n", c, allSum);
    }

    for (int d=0; d<gconf->GetTotalNumDie(); d++)
    {
        uint64 allSum = 0;
        DPRINTF(PAM2, "\nDIE %d Conflict-Counts:\n  NONE     DMA0     MEM      DMA1      SUM\n", d);
        for (int op=0;op<OPER_NUM;op++)
        {
            uint64 cfSum = 0;
            DPRINTF(PAM2, "%s ", OPER_STRINFO[op]);
            for (int cf=0;cf<CONFLICT_NUM;cf++)
            {
                DPRINTF(PAM2, "%8llu ", die_conflict_cnt[d].c[op][cf]);
                cfSum += ( die_conflict_cnt[d].c[op][cf] );
            }
            DPRINTF(PAM2, "%8llu\n", cfSum);
            allSum += cfSum;
        }
        DPRINTF(PAM2, "DIE %d CONF-CNT-SUM: %llu\n", d, allSum);
    } 
     
    /* 
    //Conflict Prints - LENGTH ========================================
    for (int c=0; c<gconf->NumChannel; c++)
    {
        uint64 allSum = 0;
        DPRINTF(PAM2, "\nCH %d Conflict-Length:\n  NONE              DMA0             MEM              DMA1              SUM\n", c);
        for (int op=0;op<OPER_NUM;op++)
        {
            uint64 cfSum = 0;
            DPRINTF(PAM2, "%4s ", OPER_STRINFO[op]);
            for (int cf=0;cf<CONFLICT_NUM;cf++)
            {
                DPRINTF(PAM2, "%16llu ", ch_conflict_length[c].c[op][cf]);
                cfSum += ( ch_conflict_length[c].c[op][cf] );
            }
            DPRINTF(PAM2, "%16llu\n", cfSum);
            allSum += cfSum;
        }
        DPRINTF(PAM2, "CH %d CONF-LEN-SUM: %llu\n", c, allSum);
    }

    for (int d=0; d<gconf->GetTotalNumDie(); d++)
    {
        uint64 allSum = 0;
        DPRINTF(PAM2, "\nDIE %d Conflict-Length:\n  NONE             DMA0             MEM              DMA1              SUM\n", d);
        for (int op=0;op<OPER_NUM;op++)
        {
            uint64 cfSum = 0;
            DPRINTF(PAM2, "%s ", OPER_STRINFO[op]);
            for (int cf=0;cf<CONFLICT_NUM;cf++)
            {
                DPRINTF(PAM2, "%16llu ", die_conflict_length[d].c[op][cf]);
                cfSum += ( die_conflict_length[d].c[op][cf] );
            }
            DPRINTF(PAM2, "%16llu\n", cfSum);
            allSum += cfSum;
        }
        DPRINTF(PAM2, "DIE %d CONF-LEN-SUM: %llu\n", d, allSum);
    }

    //Conflict Prints - COUNT% ========================================
    for (int c=0; c<gconf->NumChannel; c++)
    {
        long double allSum = 0;
        DPRINTF(PAM2, "\nCH %d Conflict-Counts(ratio):\n  NONE     DMA0     MEM      DMA1      SUM\n", c);
        for (int op=0;op<OPER_NUM;op++)
        {
            long double cfSum = 0;
            DPRINTF(PAM2, "%4s ", OPER_STRINFO[op]);
            for (int cf=0;cf<CONFLICT_NUM;cf++)
            {
                long double val = SAFEDIV( (long double)ch_conflict_cnt[c].c[op][cf] , (ch_io_cnt[c].c[op][PAGE_LSB]+ch_io_cnt[c].c[op][PAGE_CSB]+ch_io_cnt[c].c[op][PAGE_MSB]) );
                DPRINTF(PAM2, "%8Lf ", val);
                cfSum += ( val );
            }
            DPRINTF(PAM2, "%8Lf\n", cfSum);
            allSum += cfSum;
        }
        DPRINTF(PAM2, "CH %d CONF-CNT-SUM(ratio): %Lf\n", c, allSum);
    }

    for (int d=0; d<gconf->GetTotalNumDie(); d++)
    {
        long double allSum = 0;
        DPRINTF(PAM2, "\nDIE %d Conflict-Counts(ratio):\n  NONE     DMA0     MEM      DMA1      SUM\n", d);
        for (int op=0;op<OPER_NUM;op++)
        {
            long double cfSum = 0;
            DPRINTF(PAM2, "%s ", OPER_STRINFO[op]);
            for (int cf=0;cf<CONFLICT_NUM;cf++)
            {
                long double val = SAFEDIV( (long double)die_conflict_cnt[d].c[op][cf] , (die_io_cnt[d].c[op][PAGE_LSB]+die_io_cnt[d].c[op][PAGE_CSB]+die_io_cnt[d].c[op][PAGE_MSB]) ) ;
                DPRINTF(PAM2, "%8Lf ", val);
                cfSum += ( val );
            }
            DPRINTF(PAM2, "%8Lf\n", cfSum);
            allSum += cfSum;
        }
        DPRINTF(PAM2, "DIE %d CONF-CNT-SUM(ratio): %Lf\n", d, allSum);
    } 
    */ 

    /*
    //Conflict Prints - LENGTH% ========================================
    for (int c=0; c<gconf->NumChannel; c++)
    {
        long double allSum = 0;
        DPRINTF(PAM2, "\nCH %d Conflict-Length(ratio):\n  NONE     DMA0     MEM      DMA1      SUM\n", c);
        for (int op=0;op<OPER_NUM;op++)
        {
            long double cfSum = 0;
            DPRINTF(PAM2, "%4s ", OPER_STRINFO[op]);
            for (int cf=0;cf<CONFLICT_NUM;cf++)
            {
                long double val = SAFEDIV( (long double)ch_conflict_length[c].c[op][cf] , elapsed_time_ps );
                DPRINTF(PAM2, "%8Lf ", val);
                cfSum += ( val );
            }
            DPRINTF(PAM2, "%8Lf\n", cfSum);
            allSum += cfSum;
        }
        DPRINTF(PAM2, "CH %d CONF-LEN-SUM(ratio): %Lf\n", c, allSum);
    }

    for (int d=0; d<gconf->GetTotalNumDie(); d++)
    {
        long double allSum = 0;
        DPRINTF(PAM2, "\nDIE %d Conflict-Length(ratio):\n  NONE     DMA0     MEM      DMA1      SUM\n", d);
        for (int op=0;op<OPER_NUM;op++)
        {
            long double cfSum = 0;
            DPRINTF(PAM2, "%s ", OPER_STRINFO[op]);
            for (int cf=0;cf<CONFLICT_NUM;cf++)
            {
                long double val = SAFEDIV( (long double)die_conflict_length[d].c[op][cf] , elapsed_time_ps );
                DPRINTF(PAM2, "%8Lf ", val);
                cfSum += ( val );
            }
            DPRINTF(PAM2, "%8Lf\n", cfSum);
            allSum += cfSum;
        }
        DPRINTF(PAM2, "DIE %d CONF-LEN-SUM(ratio): %Lf\n", d, allSum);
    }
    */
  #endif



  #if 1 // Polished stats - Improved instrumentation
	std::map<uint64, ValueOper*>::iterator e  = Access_Capacity_snapshot.find(sim_time_ps/EPOCH_INTERVAL-1);
    if (sim_time_ps > 0 && e != Access_Capacity_snapshot.end()){
    PPN_requested_rwe.printstat("Num of PPN IO request");
    printf( "===================\n");

    for(uint32 i=0; i<PAGE_NUM; i++)
    {
        char str[256];
        sprintf(str, "Num of %s page PPN IO request", PAGE_STRINFO[i]);
        PPN_requested_pagetype[i].printstat(str);
    }
    printf( "===================\n");

    for (uint32 i=0;i<gconf->NumChannel; i++)
    {
        char str[256];
        sprintf(str, "Num of CH_%u PPN IO request", i);
        PPN_requested_ch[i].printstat(str);
    }
    printf( "===================\n");

    for (uint32 i=0; i<gconf->GetTotalNumDie(); i++)
    {
        char str[256];
        sprintf(str, "Num of DIE_%u PPN IO request", i);
        PPN_requested_die[i].printstat(str);
    }
    printf( "===================\n");

    CF_DMA0_dma.printstat("Num of conflict DMA0-CH");
    CF_DMA0_mem.printstat("Num of conflict DMA0-MEM");
    CF_DMA0_none.printstat("Num of conflict DMA0-None");
    printf( "===================\n");

    CF_DMA1_dma.printstat("Num of conflict DMA1-CH");
    CF_DMA1_none.printstat("Num of conflict DMA1-None");
    printf( "===================\n");

    Ticks_DMA0WAIT.printstat("Info of DMA0WAIT Tick");
    Ticks_DMA0.printstat("Info of DMA0 Tick");
    Ticks_MEM.printstat("Info of MEM Tick");
    Ticks_DMA1WAIT.printstat("Info of DMA1WAIT Tick");
    Ticks_DMA1.printstat("Info of DMA1 Tick");
    Ticks_Total.printstat("Info of TOTAL(D0W+D0+M+D1W+D1) Tick");
    Ticks_TotalOpti.printstat("Info of OPTIMUM(D0+M+D1) Tick");
    printf( "===================\n");

    for (uint32 i=0;i<gconf->NumChannel; i++)
    {
        char str[256];
        sprintf(str, "Info of CH_%u Active Tick", i);
        Ticks_Active_ch[i].printstat(str);
    }
    printf( "===================\n");

    for (uint32 i=0; i<gconf->GetTotalNumDie(); i++)
    {
        char str[256];
        sprintf(str, "Info of DIE_%u Active Tick", i);
        Ticks_Active_die[i].printstat(str);
    }
    printf( "===================\n");

    	//printf("LastExactBusyTime=%llu\tExactBusyTime=%llu\n",LastExactBusyTime,SampledExactBusyTime);
    	e->second->printstat("Info of Access Capacity");
    	//printf( "PAM: Total execution time (ms), Total SSD active time (ms)\n");
    	//printf( "PAM: %.2f\t\t\t, %.2f\n", elapsed_time_ps * 1.0 / 1000000000, SampledExactBusyTime * 1.0 / 1000000000);
    	printf( "PAM: Total execution time (ms)\n");
    	printf( "PAM: %.2f\n", SampledExactBusyTime * 1.0 / 1000000000);
    	Access_Bandwidth.printstat_bandwidth(e->second, SampledExactBusyTime, LastExactBusyTime);
    	Access_Bandwidth_widle.printstat_bandwidth_widle(e->second, sim_time_ps, LastExecutionTime);
    	Access_Oper_Bandwidth.printstat_oper_bandwidth(e->second, OpBusyTime, LastOpBusyTime);
    	std::map<uint64, ValueOper*>::iterator f  = Ticks_Total_snapshot.find(sim_time_ps/EPOCH_INTERVAL-1);
    	f->second->printstat_latency("Info of Latency");
    	Access_Iops.printstat_iops(e->second, SampledExactBusyTime, LastExactBusyTime);
    	Access_Iops_widle.printstat_iops_widle(e->second, sim_time_ps, LastExecutionTime);
    	Access_Oper_Iops.printstat_oper_iops(e->second, OpBusyTime, LastOpBusyTime);
    	LastExactBusyTime = SampledExactBusyTime;
    	LastExecutionTime = sim_time_ps;
    	LastOpBusyTime[0] = OpBusyTime[0];
    	LastOpBusyTime[1] = OpBusyTime[1];
    	LastOpBusyTime[2] = OpBusyTime[2];
    	std::map<uint64, ValueOper*>::iterator g  = Access_Capacity_snapshot.upper_bound(sim_time_ps/EPOCH_INTERVAL-1);
    	if (g != Access_Capacity_snapshot.end()){
    		for (int i=0;i<OPER_ALL;i++){
    		    g->second->vals[i].sampled_sum = e->second->vals[i].sum;
    		    g->second->vals[i].sampled_cnt = e->second->vals[i].cnt;
    		    e->second->vals[i].sampled_sum = e->second->vals[i].sum;
    		    e->second->vals[i].sampled_cnt = e->second->vals[i].cnt;
    		}
    	}
    	else{
    		Access_Capacity_snapshot[sim_time_ps/EPOCH_INTERVAL] = new ValueOper(Access_Capacity_snapshot[sim_time_ps/EPOCH_INTERVAL-1]);
    		for (int i=0;i<OPER_ALL;i++){
    			Access_Capacity_snapshot[sim_time_ps/EPOCH_INTERVAL]->vals[i].sampled_sum = e->second->vals[i].sum;
    			Access_Capacity_snapshot[sim_time_ps/EPOCH_INTERVAL]->vals[i].sampled_cnt = e->second->vals[i].cnt;
    		}
    	}
    	std::map<uint64, ValueOper*>::iterator h  = Ticks_Total_snapshot.upper_bound(sim_time_ps/EPOCH_INTERVAL-1);
    	if (h != Ticks_Total_snapshot.end()){
    		for (int i=0;i<OPER_ALL;i++){
    		    h->second->vals[i].sampled_sum = f->second->vals[i].sum;
    		    h->second->vals[i].sampled_cnt = f->second->vals[i].cnt;
    		    f->second->vals[i].sampled_sum = f->second->vals[i].sum;
    		    f->second->vals[i].sampled_cnt = f->second->vals[i].cnt;
    		}
    	}
    	else{
    		Ticks_Total_snapshot[sim_time_ps/EPOCH_INTERVAL] = new ValueOper(Ticks_Total_snapshot[sim_time_ps/EPOCH_INTERVAL-1]);
    		for (int i=0;i<OPER_ALL;i++){
    			Ticks_Total_snapshot[sim_time_ps/EPOCH_INTERVAL]->vals[i].sampled_sum = f->second->vals[i].sum;
    			Ticks_Total_snapshot[sim_time_ps/EPOCH_INTERVAL]->vals[i].sampled_cnt = f->second->vals[i].cnt;
    		}
    	}
    	Access_Capacity_snapshot.erase(sim_time_ps/EPOCH_INTERVAL-1);
    	Ticks_Total_snapshot.erase(sim_time_ps/EPOCH_INTERVAL-1);
    }


    if (Access_Capacity.vals[OPER_NUM].cnt > Access_Capacity.vals[OPER_NUM].sampled_cnt){
    	//printf("ExactBusyTime is %llu, LastExactBusyTime is %llu, difference is %llu\n",ExactBusyTime,LastExactBusyTime,ExactBusyTime-LastExactBusyTime);
    	//LastExactBusyTime = ExactBusyTime;
    }
    for (int i=0;i<OPER_ALL;i++){
    	Access_Capacity.vals[i].backup();
    }

  #endif // Polished stats

}
