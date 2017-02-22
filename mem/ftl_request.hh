
#ifndef __MEM_FTL_REQUST_HH__
#define __MEM_FTL_REQUST_HH__

#include "mem/ftl_defs.hh"

class InputRequest{
public:
	
	Tick	 	time;
	int 		operation;
	Addr 		sector_address;
	Addr 		sector_size;

	Addr 		page_address;
	Addr 		page_size;

	Tick		service_latency;
	Addr			serviced_page;
	Addr 		updated_page; 
	bool 		done; 	
	Mask 		* request_masks; // mask for all subrequests 
	InputRequest(Tick time, int op, Addr address, Addr size, int ps, int superpage_degree); 
	~InputRequest(); 
	bool page_service_done(Tick latency);
	bool page_status_updated(); 
	void toString(){
		std::cout << "NOT " <<time << "\t" <<  operation << "\t" << page_address << "\t" << page_size <<"\t" <<serviced_page << std::endl; 
	} 
};

class OutputRequest{

public:	
	long int 	seq_num; 
	Tick	time;
	int 	operation; 
	Addr	logical_page_address; 
	Addr	physical_page_address; 
	bool 	latency_reported; 
	InputRequest * parent_req; 
	
	OutputRequest(long int seq_num, Tick arrive_time, Addr lpn,Addr ppn, InputRequest * parent, int op);	
};
#endif
