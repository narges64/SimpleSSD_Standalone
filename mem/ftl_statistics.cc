#include "ftl_statistics.hh"
#include <stdio.h>

FTLStats::FTLStats(){

	sim_read_active_time = 0; 
	sim_write_active_time = 0; 
	sim_rw_active_time = 0; 

	read_active_last_update = 0; 
	write_active_last_update = 0; 
	rw_active_last_update = 0; 
	
	sim_read_outstanding_count = 0; 
	sim_write_outstanding_count = 0; 
	sim_rw_outstanding_count = 0; 

	last_epoch_collected = 0;

	host_sim_read_count = 0; 
	host_sim_write_count = 0; 
	host_sim_read_capacity = 0; 
	host_sim_write_capacity = 0; 

	// epoch Statistics 	
	epoch_number = 0; 
	next_epoch_read_count = 0; 
	next_epoch_write_count = 0; 
	next_epoch_read_capacity = 0; 
	next_epoch_write_capacity = 0;
	next_epoch_read_lat_sum = 0; 
	next_epoch_write_lat_sum = 0; 
	next_epoch_read_size_sum = 0; 
	next_epoch_write_size_sum = 0; 

	reset_epoch_stats(epoch_number); 

}

void FTLStats::reset_epoch_stats(int epoch_number){
	
	vector<RequestInterval>::iterator it;

	for (it = events.begin(); it < events.end();){		
		if (it->epoch_number == epoch_number) 
			it = events.erase(it); 
		else it++; 
	}
	
	for (it = read_events.begin(); it < read_events.end(); ){		
		if (it->epoch_number == epoch_number)
			it = read_events.erase(it); 
		else it++; 
	}

	for (it = write_events.begin(); it < write_events.end(); ){	
		if ( it->epoch_number == epoch_number)
			it = write_events.erase(it); 
		else it++; 
	}
	current_epoch_read_count = next_epoch_read_count; 
	next_epoch_read_count = 0; 
	current_epoch_write_count = next_epoch_write_count; 
	next_epoch_write_count = 0; 
	current_epoch_read_capacity = next_epoch_read_capacity; 
	next_epoch_read_capacity = 0; 
	current_epoch_write_capacity = next_epoch_write_capacity; 
	next_epoch_write_capacity = 0; 
	current_epoch_read_lat_sum = next_epoch_read_lat_sum; 
	next_epoch_read_lat_sum = 0; 
	current_epoch_write_lat_sum = next_epoch_write_lat_sum; 
	next_epoch_write_lat_sum = 0; 
	current_epoch_read_size_sum = next_epoch_read_size_sum; 
	next_epoch_read_size_sum = 0; 
	current_epoch_write_size_sum = next_epoch_write_size_sum; 
	next_epoch_write_size_sum = 0; 

	host_epoch_superpage_utilization.reset(); 	
}

void FTLStats::print_epoch_stats(Tick sim_time, int epoch_number, bool final_epoch = false){
	printf("FTL Host epoch %d , time: %lld \n", epoch_number, sim_time); 

	printf("FTL Host read  count %d Total %d\n"			, (int)host_epoch_read_count.Get(), (int) host_sim_read_count);
	printf("FTL Host read  size %.2f KB\n"		, host_epoch_read_size.Get());
	printf("FTL Host read  latency %.2f us\n"	, host_epoch_read_latency.Get());
	printf("FTL Host read  capacity %.2f KB\n"	, host_epoch_read_capacity.Get()); 
	printf("FTL Host write count %d Total %d \n"			, (int)host_epoch_write_count.Get(), (int)host_sim_write_count);
	printf("FTL Host write size %.2f KB\n"		, host_epoch_write_size.Get());
	printf("FTL Host write latency %.2f us\n"	, host_epoch_write_latency.Get());
	printf("FTL Host write capacity %.2f KB\n"	, host_epoch_write_capacity.Get()); 
	
	// BW 
	printf("FTL Host read  BW (active): %.2f MB/s\n", host_epoch_read_BW_active.Get()); 
	printf("FTL Host write BW (active): %.2f MB/s\n", host_epoch_write_BW_active.Get()); 
	printf("FTL Host rw    BW (active): %.2f MB/s\n", host_epoch_rw_BW_active.Get());  
	printf("FTL Host read  BW (total ): %.2f MB/s\n", host_epoch_read_BW_total.Get()); 
	printf("FTL Host write BW (total ): %.2f MB/s\n", host_epoch_write_BW_total.Get()); 
	printf("FTL Host rw    BW (total ): %.2f MB/s\n", host_epoch_rw_BW_total.Get());  
	printf("FTL Host read  BW (only  ): %.2f MB/s\n", host_epoch_read_BW_only.Get()); 
	printf("FTL Host write BW (only  ): %.2f MB/s\n", host_epoch_write_BW_only.Get()); 

	// IOPS  
	printf("FTL Host read  IOPS (active): %.2f \n", host_epoch_read_IOPS_active.Get()); 
	printf("FTL Host write IOPS (active): %.2f \n", host_epoch_write_IOPS_active.Get()); 
	printf("FTL Host rw    IOPS (active): %.2f \n", host_epoch_rw_IOPS_active.Get());  
	printf("FTL Host read  IOPS (total ): %.2f \n", host_epoch_read_IOPS_total.Get()); 
	printf("FTL Host write IOPS (total ): %.2f \n", host_epoch_write_IOPS_total.Get()); 
	printf("FTL Host rw    IOPS (total ): %.2f \n", host_epoch_rw_IOPS_total.Get());   
	printf("FTL Host read  IOPS (only  ): %.2f \n", host_epoch_read_IOPS_only.Get()); 
	printf("FTL Host write IOPS (only  ): %.2f \n", host_epoch_write_IOPS_only.Get()); 
	
	printf("FTL Host suprepage utilization: "); host_epoch_superpage_utilization.print(); 
		
	if (final_epoch) {
		printf("FTL Host epoch summarized statistics time %lld\n", sim_time); 
		printf("FTL Host read  count "); 		host_epoch_read_count.print();
		printf("FTL Host read  size  "); 		host_epoch_read_size.print();
		printf("FTL Host read  latency "); 		host_epoch_read_latency.print();
		printf("FTL Host read  capacity "); 	host_epoch_read_capacity.print(); 
		printf("FTL Host write count "); 		host_epoch_write_count.print();
		printf("FTL Host write size "); 		host_epoch_write_size.print();
		printf("FTL Host write latency ");		host_epoch_write_latency.print();
		printf("FTL Host write capacity "); 	host_epoch_write_capacity.print(); 
	
		// BW 
		printf("FTL Host read  BW (active) "); host_epoch_read_BW_active.print(); 
		printf("FTL Host write BW (active) "); host_epoch_write_BW_active.print(); 
		printf("FTL Host rw    BW (active) "); host_epoch_rw_BW_active.print();  
		printf("FTL Host read  BW (total ) "); host_epoch_read_BW_total.print(); 
		printf("FTL Host write BW (total ) "); host_epoch_write_BW_total.print(); 
		printf("FTL Host rw    BW (total ) "); host_epoch_rw_BW_total.print(); 
		printf("FTL Host read  BW (only  ) "); host_epoch_read_BW_only.print(); 
		printf("FTL Host write BW (only  ) "); host_epoch_write_BW_only.print(); 
		
		// IOPS  
		printf("FTL Host read  IOPS (active) ");	host_epoch_read_IOPS_active.print();  
		printf("FTL Host write IOPS (active) ");	host_epoch_write_IOPS_active.print(); 
		printf("FTL Host rw    IOPS (active) "); 	host_epoch_rw_IOPS_active.print(); 
		printf("FTL Host read  IOPS (total ) "); 	host_epoch_read_IOPS_total.print(); 
		printf("FTL Host write IOPS (total ) "); 	host_epoch_write_IOPS_total.print(); 
		printf("FTL Host rw    IOPS (total ) "); 	host_epoch_rw_IOPS_total.print(); 
		printf("FTL Host read  IOPS (only  ) "); 	host_epoch_read_IOPS_only.print(); 
		printf("FTL Host write IOPS (only  ) "); 	host_epoch_write_IOPS_only.print(); 
		
	}
}
														
void FTLStats::print_stats(const Tick sim_time, bool final_call){
	bool final_epoch = final_call; 	

	int epoch_number = sim_time / EPOCH_INTERVAL; 
	if (sim_time > (epoch_number * EPOCH_INTERVAL)){ 
		final_epoch = true;  
		epoch_number++; 
	}
	
	// in case print_epoch_stats has not been called for a while 
	for (int i = last_epoch_collected + 1; i <= epoch_number; i++){ 
		if (final_epoch && (i == epoch_number)){
			collect_epoch_stats(i, true); 
			print_epoch_stats(sim_time, i, true);  // final stats 
			reset_epoch_stats(i); 	
			print_simulation_stats(sim_time); 	
			return; 
		}	
		collect_epoch_stats(i, false); 
		print_epoch_stats(i*EPOCH_INTERVAL, i, false); 
		reset_epoch_stats(i); 	

	}
	last_epoch_collected = epoch_number;
}



void FTLStats::collect_epoch_stats(int ep_num, bool last_epoch){
	double current_epoch_read_size = 0; 
	double current_epoch_read_latency = 0; 

	double current_epoch_write_size = 0; 
	double current_epoch_write_latency = 0; 

	if (current_epoch_read_count != 0) {
		current_epoch_read_latency = current_epoch_read_lat_sum / (double)current_epoch_read_count; 
		current_epoch_read_size = current_epoch_read_size_sum / (double)current_epoch_read_count; 
	}
	if (current_epoch_write_count != 0) {
		current_epoch_write_latency = current_epoch_write_lat_sum / (double)current_epoch_write_count; 
		current_epoch_write_size = current_epoch_write_size_sum / (double)current_epoch_write_count; 
	}
		
	host_epoch_read_count			.update(current_epoch_read_count); 
	host_epoch_read_size			.update(current_epoch_read_size); 
	host_epoch_read_latency			.update(current_epoch_read_latency); 
	host_epoch_read_capacity		.update(current_epoch_read_capacity);
 
	host_epoch_write_count			.update(current_epoch_write_count); 
	host_epoch_write_size			.update(current_epoch_write_size); 
	host_epoch_write_latency		.update(current_epoch_write_latency); 
	host_epoch_write_capacity		.update(current_epoch_write_capacity);


	Tick total_time 		= epoch_total_time(&events, ep_num); 
	Tick active_time		= epoch_active_time(&events, ep_num);
	Tick read_active_time 	= epoch_active_time(&read_events, ep_num);  
	Tick write_active_time 	= epoch_active_time(&write_events, ep_num); 

	if (total_time == 0) {
		cout << "ERROR total time should never be zero " << endl; 
		return; 
	}
	host_epoch_read_BW_total		.update((current_epoch_read_capacity * USEC * USEC) / (KBYTE * total_time) );
	host_epoch_write_BW_total		.update(current_epoch_write_capacity * USEC * USEC / (KBYTE * total_time)); 
	host_epoch_rw_BW_total			.update((current_epoch_read_capacity + current_epoch_write_capacity) * USEC * USEC/ (KBYTE * total_time)); 
	host_epoch_read_IOPS_total		.update(current_epoch_read_count * USEC * USEC / (total_time)); 
	host_epoch_write_IOPS_total		.update(current_epoch_write_count * USEC * USEC / (total_time)); 
	host_epoch_rw_IOPS_total		.update((current_epoch_read_count + current_epoch_write_count) * USEC * USEC  / (total_time)); 
	
	if ((current_epoch_read_count == 0) || (current_epoch_read_capacity == 0)) {
	host_epoch_read_BW_active		.update(0); 
	host_epoch_read_BW_only			.update(0);  
	host_epoch_read_IOPS_active		.update(0); 
	host_epoch_read_IOPS_only		.update(0);
	}else {
	host_epoch_read_BW_active		.update(current_epoch_read_capacity * USEC * USEC / (KBYTE * active_time) ); 
	host_epoch_read_BW_only			.update(current_epoch_read_capacity * USEC * USEC / (KBYTE * read_active_time) );  
	host_epoch_read_IOPS_active		.update(current_epoch_read_count * USEC * USEC / (active_time)); 
	host_epoch_read_IOPS_only		.update(current_epoch_read_count * USEC * USEC / (read_active_time)); 	
	}

	if ((current_epoch_write_count == 0) || (current_epoch_write_capacity == 0)) {
	host_epoch_write_BW_active		.update(0); 
	host_epoch_write_BW_only		.update(0);
	host_epoch_write_IOPS_active	.update(0); 
	host_epoch_write_IOPS_only		.update(0); 
	}else {
	host_epoch_write_BW_active		.update(current_epoch_write_capacity * USEC * USEC / (KBYTE * active_time)); 
	host_epoch_write_BW_only		.update(current_epoch_write_capacity * USEC * USEC / (KBYTE * write_active_time));
	host_epoch_write_IOPS_active	.update(current_epoch_write_count * USEC * USEC / (active_time)); 
	host_epoch_write_IOPS_only		.update(current_epoch_write_count * USEC * USEC / (write_active_time)); 		
	}

	if ((current_epoch_read_count + current_epoch_write_count == 0) || (current_epoch_read_capacity + current_epoch_write_capacity == 0)) {
	host_epoch_rw_BW_active			.update(0); 
	host_epoch_rw_IOPS_active		.update(0); 
	}else{
	host_epoch_rw_BW_active			.update((current_epoch_read_capacity + current_epoch_write_capacity) * USEC * USEC / (KBYTE  * active_time)); 
	host_epoch_rw_IOPS_active		.update((current_epoch_read_count + current_epoch_write_count) * USEC * USEC / (active_time)); 
	}
	
}

Tick FTLStats::epoch_total_time(vector<RequestInterval> * ev, int epoch_number){

	Tick start_time = (epoch_number - 1) * EPOCH_INTERVAL; 

	vector<RequestInterval>::iterator it; 
	for (it = ev->begin(); it != ev->end(); it++){ 
		if (it->epoch_number != epoch_number) continue; 
		Tick s = it->arrived; 
		if (start_time > s) start_time = s;
	}
	Tick extra_time = 0; 
	if (start_time < ((epoch_number - 1) * EPOCH_INTERVAL)) 
		extra_time =  ((epoch_number - 1) * EPOCH_INTERVAL) - start_time; 
		
	return extra_time + EPOCH_INTERVAL; 
}
bool myfunction(pair<Tick, int> i, pair<Tick, int> j) {return (i.first < j.first);}

Tick FTLStats::epoch_active_time(vector<RequestInterval> * ev, int epoch_number){
	Tick at =   0;  
	vector<pair<Tick, int> > sort_list; 
	vector<RequestInterval >::iterator it; 
	for (it = ev->begin(); it != ev->end(); it++){
		if (it->epoch_number != epoch_number) continue; 
		Tick s = it->arrived;
		Tick e = it->left; 
		sort_list.push_back(pair<Tick, int> (s, 1)); 
		sort_list.push_back(pair<Tick, int>(e, -1)); 
	}
	std::sort(sort_list.begin(), sort_list.end(), myfunction); 
		
	int reqcount = 0; 

	vector<pair<Tick, int> >::iterator vit;
	
	Tick prev_time = 0; 
	for (vit = sort_list.begin(); vit != sort_list.end(); vit++){	
		if (reqcount != 0) {
			at += vit->first - prev_time; 
		}
		if (vit->second > 0) reqcount++; else reqcount--; 		
		prev_time = vit->first; 
	}
	return at; 
}

void FTLStats::updateStats(InputRequest * iReq){

	if (iReq == NULL) return; 	
	
	Tick arrived_tick = iReq->time; 
	Tick left_tick = arrived_tick + iReq->service_latency;

	if (iReq->operation == OP_READ) 
		read_req_leave(left_tick); 
	else if (iReq->operation == OP_WRITE) 
		write_req_leave(left_tick); 
	else 
		return;
	
	add_req_pair(arrived_tick , left_tick, iReq->operation);	
 
	
	int ep_num =( left_tick / EPOCH_INTERVAL) + 1; 
	
	update_stats_for_request(iReq, ep_num);

}

void FTLStats::update_stats_for_request(InputRequest * ireq, int ep_num){
	if (ireq == NULL) return; 
	if (ep_num == last_epoch_collected + 1){
		// update for current epoch 
		if (ireq->operation == OP_READ) {
			current_epoch_read_count++; 
			current_epoch_read_size_sum += ireq->sector_size / 2; // convert from sector to KB 
			current_epoch_read_lat_sum += ireq->service_latency / USEC; // convert from ps to us 
			current_epoch_read_capacity += ireq->sector_size / 2; 
			
			host_sim_read_count++; 
			host_sim_read_size 		.update (ireq->sector_size / 2); 
			host_sim_read_latency	.update(ireq->service_latency / USEC); 
			host_sim_read_capacity += ireq->sector_size / 2; 

		}else if (ireq->operation == OP_WRITE){
			current_epoch_write_count++; 
			current_epoch_write_size_sum += ireq->sector_size / 2; // convert from sector to KB 
			current_epoch_write_lat_sum += ireq->service_latency / USEC; // convert from ps to us 
			current_epoch_write_capacity += ireq->sector_size / 2; 

			host_sim_write_count++; 
			host_sim_write_size 	.update (ireq->sector_size / 2); 
			host_sim_write_latency	.update(ireq->service_latency / USEC); 
			host_sim_write_capacity += ireq->sector_size / 2 ;  // convert to KB 
		}
	
	} 
	else {
		// update for next epoch 
		if (ireq->operation == OP_READ) {
			next_epoch_read_count++; 
			next_epoch_read_size_sum += ireq->sector_size / 2; // convert from sector to KB 
			next_epoch_read_lat_sum += ireq->service_latency / USEC; // convert from ps to us 
			next_epoch_read_capacity += ireq->sector_size / 2; 
			
			host_sim_read_count++; 
			host_sim_read_size 		.update (ireq->sector_size / 2); 
			host_sim_read_latency	.update(ireq->service_latency / USEC); 
			host_sim_read_capacity += ireq->sector_size / 2; 

		}else if (ireq->operation == OP_WRITE){
			next_epoch_write_count++; 
			next_epoch_write_size_sum += ireq->sector_size / 2; // convert from sector to KB 
			next_epoch_write_lat_sum += ireq->service_latency / USEC; // convert from ps to us 
			next_epoch_write_capacity += ireq->sector_size / 2; 

			host_sim_write_count++; 
			host_sim_write_size 	.update (ireq->sector_size / 2); 
			host_sim_write_latency	.update(ireq->service_latency / USEC); 
			host_sim_write_capacity += ireq->sector_size / 2 ;  // convert to KB 
		}
	
	} 
	
}

void FTLStats::add_req_pair(Tick arrived_tick, Tick left_tick, int operation){
	int epoch_number = (left_tick / EPOCH_INTERVAL) + 1; 
	vector<RequestInterval>::iterator it; 
	for (it = events.begin(); it != events.end(); it++){
		if (it->epoch_number != epoch_number) continue; 
		Tick it_arrived = it->arrived; 
		Tick it_left = it->left;  
		if ( arrived_tick <= it_arrived && left_tick >= it_arrived){
			it->arrived = arrived_tick; 
			if (left_tick > it_left) 
				it->left = left_tick; 
			break; 
		}
		if (arrived_tick >= it_arrived && arrived_tick <= it_left) {
			if (left_tick >= it_left){
				it->left = left_tick;  
			} 
			break; 
		}
	}
	if (it == events.end()){
		RequestInterval ri(arrived_tick, left_tick, epoch_number); 
		events.push_back(ri); 
	}
	if (operation == OP_READ) {
		for (it = read_events.begin(); it != read_events.end(); it++){
			if (it->epoch_number != epoch_number) continue; 
			Tick it_arrived = it->arrived; 
			Tick it_left = it->left; 
			if ( arrived_tick <= it_arrived && left_tick >= it_arrived){
				it->arrived = arrived_tick; 
				if (left_tick > it_left) 
					it->left = left_tick; 
				break; 
			}
			if (arrived_tick >= it_arrived && arrived_tick <= it_left) {
				if (left_tick >= it_left){
					it->left = left_tick;  
				} 
				break; 
			}
		}
		if (it == read_events.end()){
			RequestInterval ri(arrived_tick, left_tick, epoch_number); 
			read_events.push_back(ri);
		} 
	
	}	
	else {
		for (it = write_events.begin(); it != write_events.end(); it++){
			if (it->epoch_number != epoch_number) continue; 
			Tick it_arrived = it->arrived; 
			Tick it_left = it->left;  
			if ( arrived_tick <= it_arrived && left_tick >= it_arrived){
				it->arrived = arrived_tick; 
				if (left_tick > it_left) 
					it->left = left_tick; 
				break; 
			}
			if (arrived_tick >= it_arrived && arrived_tick <= it_left) {
				if (left_tick >= it_left){
					it->left = left_tick;  
				} 
				break; 
			}
		}
		if (it == write_events.end()){
			RequestInterval ri (arrived_tick, left_tick, epoch_number); 
			write_events.push_back(ri); 	
		}
	}
}

void FTLStats::read_req_arrive(Tick arrive_time){
	// call rw_req_arrive to update overall statistics
	 
	rw_req_arrive(arrive_time); 

	sim_read_outstanding_count++; 

	if (arrive_time < read_active_last_update) return; 

	if (sim_read_outstanding_count > 1) 
		sim_read_active_time += arrive_time - read_active_last_update;

	read_active_last_update = arrive_time;  	
}

void FTLStats::write_req_arrive(Tick arrive_time){
	// call rw_req_arrive to update overall statistics 
	rw_req_arrive(arrive_time);
 
	sim_write_outstanding_count++; 
	
	if (arrive_time < write_active_last_update) return; 

	if (sim_write_outstanding_count > 1) 
		sim_write_active_time += arrive_time - write_active_last_update; 

	write_active_last_update = arrive_time; 
}

void FTLStats::rw_req_arrive(Tick arrive_time){
	sim_rw_outstanding_count++; 
	
	if (arrive_time < rw_active_last_update) return; 
	
	if (sim_rw_outstanding_count > 1) 
		sim_rw_active_time += arrive_time - rw_active_last_update; 

	rw_active_last_update = arrive_time;
}

void FTLStats::read_req_leave (Tick leave_time){
	// call rw_req_leave to update overall statistics 
	rw_req_leave(leave_time); 

	sim_read_outstanding_count--; 
	
	if (leave_time < read_active_last_update) return;
 
	sim_read_active_time += leave_time - read_active_last_update; 
	read_active_last_update = leave_time; 
}

void FTLStats::write_req_leave (Tick leave_time){
	// call rw_req_leave to update overall statistics 
	rw_req_leave(leave_time); 
	sim_write_outstanding_count--; 
	if (leave_time < write_active_last_update) return;
	sim_write_active_time += leave_time - write_active_last_update; 
	write_active_last_update = leave_time; 
}

void FTLStats::rw_req_leave (Tick leave_time){
	sim_rw_outstanding_count--; 
	if (leave_time < rw_active_last_update) return;
	sim_rw_active_time += leave_time - rw_active_last_update; 
	rw_active_last_update = leave_time;
}
Tick FTLStats::get_read_active_time (Tick current_time){
	// make sure active time is updated up to now 
	if (current_time != read_active_last_update) 
		cout << "ERROR Active time for read is not updated! " << endl; 

	// active time should be LE total time 
	if (sim_read_active_time > current_time) 
		cout << "ERROR Active time of read is more than total time! " << endl; 

	return sim_read_active_time; 
}

Tick FTLStats::get_write_active_time (Tick current_time){
	// make sure active time is updated up to now 
	if (current_time != write_active_last_update && write_active_last_update != 0) 
		cout << "ERROR Active time for write is not updated! " << endl; 

	// active time should be LE total time 
	if (sim_write_active_time > current_time) 
		cout << "ERROR Active time of write is more than total time! " << endl; 

	return sim_write_active_time; 
	
}
Tick FTLStats::get_rw_active_time(Tick current_time){
	// make sure active time is updated up to now 
	if (current_time != rw_active_last_update) 
		cout << "ERROR Active time for rw is not updated! " << endl; 

	// active time should be LE total time 
	if (sim_rw_active_time > current_time) 
		cout << "ERROR Active time of rw is more than total time! " << endl; 

	return sim_rw_active_time; 
	
}

void FTLStats::print_simulation_stats(Tick sim_time){
	printf("FTL Host simulation statistics , time: %lld \n", sim_time); 

	// superpage utilization 
	printf("FTL Host superpage utilization "); 	host_sim_superpage_utilization.print(); 

	printf("FTL Host sim read  count %d \n", 		(int)host_sim_read_count);
	printf("FTL Host sim read  size "); 		  	host_sim_read_size.print();
	printf("FTL Host sim read  latency "); 	  	  	host_sim_read_latency.print();
	printf("FTL Host sim read  capacity %.2f KB\n",	host_sim_read_capacity); 
	printf("FTL Host sim write count %d \n", 		(int)host_sim_write_count);
	printf("FTL Host sim write size ");	 		  	host_sim_write_size.print();
	printf("FTL Host sim write latency "); 		  	host_sim_write_latency.print();
	printf("FTL Host sim write capacity %.2f KB\n", host_sim_write_capacity); 
	
	Tick active_time = get_rw_active_time(sim_time); 
	Tick total_time = sim_time; 
	Tick read_active_time = get_read_active_time(sim_time); 
	Tick write_active_time = get_write_active_time(sim_time); 


	if (active_time != 0){
        host_sim_read_BW_active		.update(host_sim_read_capacity * USEC * USEC / (KBYTE * active_time)); // to convert ps to s 
        host_sim_write_BW_active	.update(host_sim_write_capacity * USEC * USEC  / (KBYTE * active_time));
        host_sim_rw_BW_active		.update((host_sim_read_capacity + host_sim_write_capacity) * USEC * USEC / (KBYTE * active_time));
                         
        host_sim_read_IOPS_active	.update(host_sim_read_count * USEC * USEC / (active_time));
        host_sim_write_IOPS_active	.update(host_sim_write_count * USEC * USEC / (active_time));
        host_sim_rw_IOPS_active		.update((host_sim_read_count + host_sim_write_count)  * USEC * USEC / (active_time));
    }
    if (total_time != 0){
        host_sim_read_BW_total		.update(host_sim_read_capacity * USEC * USEC / (KBYTE * total_time));
        host_sim_write_BW_total		.update(host_sim_write_capacity * USEC * USEC / (KBYTE * total_time));
        host_sim_rw_BW_total		.update((host_sim_read_capacity + host_sim_write_capacity) * USEC * USEC / (KBYTE * total_time));
                        
        host_sim_read_IOPS_total	.update(host_sim_read_count * USEC * USEC / (total_time));
        host_sim_write_IOPS_total	.update(host_sim_write_count * USEC * USEC / (total_time));
        host_sim_rw_IOPS_total		.update((host_sim_read_count + host_sim_write_count) * USEC * USEC  / (total_time));
    }
    if (read_active_time != 0){
        host_sim_read_BW_only		.update(host_sim_read_capacity * USEC * USEC / (KBYTE * read_active_time));
        host_sim_read_IOPS_only		.update(host_sim_read_count * USEC * USEC / (read_active_time));
    }
    if (write_active_time != 0){
        host_sim_write_BW_only		.update(host_sim_write_capacity * USEC * USEC / (KBYTE * write_active_time));
        host_sim_write_IOPS_only	.update(host_sim_write_count * USEC * USEC / (write_active_time));
	}

	printf("FTL Host sim read  BW (active): %.2f MB/s\n", host_sim_read_BW_active.Get()); 
	printf("FTL Host sim write BW (active): %.2f MB/s\n", host_sim_write_BW_active.Get()); 
	printf("FTL Host sim rw    BW (active): %.2f MB/s\n", host_sim_rw_BW_active.Get());  
	printf("FTL Host sim read  IOPS (active): %.2f \n"	, host_sim_read_IOPS_active.Get()); 
	printf("FTL Host sim write IOPS (active): %.2f \n"	, host_sim_write_IOPS_active.Get()); 
	printf("FTL Host sim rw    IOPS (active): %.2f \n"	, host_sim_rw_IOPS_active.Get());  
	printf("FTL Host sim read  BW (total ): %.2f MB/s\n", host_sim_read_BW_total.Get()); 
	printf("FTL Host sim write BW (total ): %.2f MB/s\n", host_sim_write_BW_total.Get()); 
	printf("FTL Host sim rw    BW (total ): %.2f MB/s\n", host_sim_rw_BW_total.Get());  	
	printf("FTL Host sim read  IOPS (total ): %.2f \n"	, host_sim_read_IOPS_total.Get()); 
	printf("FTL Host sim write IOPS (total ): %.2f \n"	, host_sim_write_IOPS_total.Get()); 
	printf("FTL Host sim rw    IOPS (total ): %.2f \n"	, host_sim_rw_IOPS_total.Get());  
	printf("FTL Host sim read  BW (only  ): %.2f MB/s\n", host_sim_read_BW_only.Get()); 
	printf("FTL Host sim read  IOPS (only  ): %.2f \n"	, host_sim_read_IOPS_only.Get()); 
	printf("FTL Host sim write BW (only  ): %.2f MB/s\n", host_sim_write_BW_only.Get()); 
	printf("FTL Host sim write IOPS (only  ): %.2f \n"	, host_sim_write_IOPS_only.Get()); 
}
