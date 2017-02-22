#ifndef __FTL_STATISTICS_HH__
#define __FTL_STATISTICS_HH__

#include <iostream>
#include <vector>
#include <utility>
#include <float.h>
#include <algorithm>
#include "mem/ftl_request.hh"
#include "mem/ftl_defs.hh"
using namespace std; 

class RequestInterval{
public: 
	RequestInterval(Tick s, Tick e, int ep){
		arrived = s; 
		left = e; 
		epoch_number = ep; 
	}
	Tick arrived; 
	Tick left; 
	int epoch_number;  
}; 

class Tuple {
public: 
	Tuple () {
		reset(); 
	}

	double current_value; 
	double min_value; 
	double max_value; 
	double avg_value; 
	int update_count; 

	void reset(){
		current_value = 0; 
		min_value = DBL_MAX; 
		max_value = 0; 
		avg_value = 0; 
		update_count = 0; 
	}

	void update(double value){
		current_value = value; 
		if (value < min_value) min_value = value; 
		if (value > max_value) max_value = value; 
		avg_value = ((avg_value * update_count) + value) / (update_count+1); 
		update_count++;  
	}
	
	void print(){
		if (update_count == 0)
			printf("(min,max,avg)( -- , -- , -- )\n");
		else  
			printf("(min,max,avg)( %.2f , %.2f , %.2f )\n", min_value, max_value, avg_value); 		
	}
	double Get(){
		return current_value; 
	}
};

class FTLStats {
public:
	int last_epoch_collected; 
	int epoch_number; 

	// To find active time of each epoch 
	vector <RequestInterval > events; 
	vector <RequestInterval > read_events; 
	vector <RequestInterval > write_events; 

	Tick sim_read_active_time = 0; 
	Tick sim_write_active_time = 0; 
	Tick sim_rw_active_time = 0; 

	Tick read_active_last_update = 0; 
	Tick write_active_last_update = 0; 
	Tick rw_active_last_update = 0; 

	int sim_read_outstanding_count = 0; 
	int sim_write_outstanding_count = 0; 
	int sim_rw_outstanding_count = 0; 

	void print_epoch_stats(Tick sim_time, int epoch_number, bool final_epoch); 
	void collect_epoch_stats(int ep_num, bool last_epoch); 
	Tick epoch_total_time(vector<RequestInterval > * ev, int epoch_number); 
	Tick epoch_active_time(vector<RequestInterval> * ev, int epoch_number); 
	void add_req_pair(Tick arrived_tick, Tick left_tick, int operation); 
	void reset_epoch_stats(int epoch_number);
	void init_sim_statistics();
	Tick get_read_active_time (Tick current_time);
	Tick get_write_active_time (Tick current_time);
	Tick get_rw_active_time(Tick current_time);

	// epoch stats  
	Tuple host_epoch_read_count; 
	Tuple host_epoch_read_size; 
	Tuple host_epoch_read_latency; 
	Tuple host_epoch_read_capacity;
 
	Tuple host_epoch_write_count; 
	Tuple host_epoch_write_size; 
	Tuple host_epoch_write_latency; 
	Tuple host_epoch_write_capacity;
	
	Tuple host_epoch_read_BW_active; 
	Tuple host_epoch_write_BW_active; 
	Tuple host_epoch_rw_BW_active; 
	Tuple host_epoch_read_BW_total; 
	Tuple host_epoch_write_BW_total; 
	Tuple host_epoch_rw_BW_total; 
	Tuple host_epoch_read_BW_only;  
	Tuple host_epoch_write_BW_only;  
	
	Tuple host_epoch_read_IOPS_active; 
	Tuple host_epoch_write_IOPS_active; 
	Tuple host_epoch_rw_IOPS_active; 
	Tuple host_epoch_read_IOPS_total; 
	Tuple host_epoch_write_IOPS_total; 
	Tuple host_epoch_rw_IOPS_total; 
	Tuple host_epoch_read_IOPS_only;  
	Tuple host_epoch_write_IOPS_only;  
	
	Tuple host_epoch_superpage_utilization; 
	
	// simulation stats 
	Tuple host_sim_read_size; 
	Tuple host_sim_write_size; 
	Tuple host_sim_read_latency; 
	Tuple host_sim_write_latency; 	
	
	Tuple host_sim_read_BW_active; 
	Tuple host_sim_write_BW_active; 
	Tuple host_sim_rw_BW_active; 
	Tuple host_sim_read_BW_total; 
	Tuple host_sim_write_BW_total; 
	Tuple host_sim_rw_BW_total; 
	Tuple host_sim_read_BW_only;  
	Tuple host_sim_write_BW_only;  
	
	Tuple host_sim_read_IOPS_active; 
	Tuple host_sim_write_IOPS_active; 
	Tuple host_sim_rw_IOPS_active; 
	Tuple host_sim_read_IOPS_total; 
	Tuple host_sim_write_IOPS_total; 
	Tuple host_sim_rw_IOPS_total; 
	Tuple host_sim_read_IOPS_only;  
	Tuple host_sim_write_IOPS_only;  
	Tuple host_sim_superpage_utilization; 
	
	double host_sim_read_count; 
	double host_sim_write_count; 
	double host_sim_read_capacity; 
	double host_sim_write_capacity; 	

	double current_epoch_read_count; 
	double current_epoch_write_count; 
	double current_epoch_read_capacity; 
	double current_epoch_write_capacity;
	double current_epoch_read_lat_sum; 
	double current_epoch_write_lat_sum; 
	double current_epoch_read_size_sum; 
	double current_epoch_write_size_sum; 

	double next_epoch_read_count; 
	double next_epoch_write_count; 
	double next_epoch_read_capacity; 
	double next_epoch_write_capacity;
	double next_epoch_read_lat_sum; 
	double next_epoch_write_lat_sum; 
	double next_epoch_read_size_sum; 
	double next_epoch_write_size_sum; 


	FTLStats(); 
	void print_simulation_stats(Tick sim_time); 
	void print_stats(const Tick sim_time, bool final_call); 
	void read_req_arrive(Tick arrive_time); 
	void write_req_arrive(Tick arrive_time);
	void rw_req_arrive(Tick arrive_time);
	void read_req_leave (Tick leave_time);
	void write_req_leave (Tick leave_time);
	void rw_req_leave (Tick leave_time);
	void update_stats_for_request(InputRequest * ireq, int ep_num); 
	void updateStats(InputRequest * iReq); 
	
}; 
#endif 
