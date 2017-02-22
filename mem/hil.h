/*
 * hil_new.h
 *
 *  Created on: Jul 13, 2016
 *      Author: zhang_000
 */

#ifndef MEM_HIL_H_
#define MEM_HIL_H_

#include <string>
#include <vector>
#include <map>

//#include "base/statistics.hh"
#include "base/types.hh"
#include "mem/ftl.hh"
#include "mem/SimpleSSD.h"

enum layer_type{HDD_LAYER, HIL_LAYER, FTL_HOST_LAYER, FTL_MAP_LAYER, FTL_PAM_LAYER, PAM_LAYER};
enum RWE_type{RD, WR, ER, ALL, GC, MERGE, RWE_NUM };
enum STAT_type{CAPACITY, BANDWIDTH, LATENCY, IOPS, SIZE, BANDWIDTH_WIDLE, IOPS_WIDLE, BANDWIDTH_OPER, IOPS_OPER, DEVICE_IDLE, DEVICE_BUSY, STAT_NUM};
enum SUB_STAT_type{AVG, MIN, MAX, TOT, COUNT, SUB_STAT_NUM};

struct output_result{
	double statistics[RWE_NUM][STAT_NUM][SUB_STAT_NUM]; // shouldn't we replace stat_num and rwe_num order. I replaced it
	output_result(){
		for (unsigned i = 0; i < RWE_NUM; i++)
			for (unsigned j = 0; j< STAT_NUM; j++)
				for (unsigned k = 0; k < SUB_STAT_NUM; k++)
					statistics[i][j][k] = 0;
	}
};

class PAMStatistics;
class PAM2;
class HIL
{
protected:

  FTL * ftl;
  PAMStatistics* stats;
  PAM2* pam2;
  int disk_number;
  int SSD;
  //********** Jie: add instrumentation parameters *************************
  Tick hdd_next_available_time;
  Tick sample_period, last_record, last_service_time;
  char operation[2][8] = {"Read","Write"};
  unsigned int Min_size[2], Max_size[2];
  unsigned long long sample_access_count[2], sample_total_volume[2];
  float Min_bandwidth[2], Max_bandwidth[2];
  float Min_bandwidth_woidle[2], Max_bandwidth_woidle[2];
  float Min_bandwidth_only[2], Max_bandwidth_only[2];
  float Min_IOPS[2], Max_IOPS[2];
  float Min_IOPS_woidle[2], Max_IOPS_woidle[2];
  float Min_IOPS_only[2], Max_IOPS_only[2];
  unsigned long long access_count[2], total_volume[2], total_time[2], sample_time[2];
  unsigned long long Min_latency[2], Max_latency[2], accumulated_latency[2];
  //*****************************************************************************
public:

  std::map<Addr, Tick> delayMap;
  std::map<Addr, Tick> dlMap;
  HIL(int dn, int SSDenable,string SSDConfig);
  void SSDoperation(Addr address, int pgNo, Tick curTick, bool writeOp);
  void AsyncOperation(Addr address, int pgNo, Tick curTick, bool writeOp){
	  SSDoperation(address, pgNo, curTick, writeOp);
  }
  void SyncOperation(Addr prev_address, Tick prev_Tick, Addr address, int pgNo, bool writeOp);
  void updateFinishTick(Addr prev_address);
  Tick updateDelay(Addr prev_address);
  void setSSD(int SSDparam);
  void setLatency(Addr sector_address, Tick delay);
  Tick getLatency(Addr address);
  Tick getMinNANDLatency();
  void print_sample(Tick& sampled_period);
  void printStats();
  void get_parameter(enum layer_type layer, struct output_result &output);

  ~HIL();

};



#endif /* MEM_HIL_H_ */
