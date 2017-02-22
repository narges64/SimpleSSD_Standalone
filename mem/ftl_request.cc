#include "mem/ftl_request.hh"

InputRequest::~InputRequest(){
	delete[] request_masks; 
}

InputRequest::InputRequest(Tick arrive_time, int op, Addr address, Addr size, int ps, int superpage_degree){
	time = arrive_time; 
	operation = op; 
	sector_address = address; 
	sector_size = size; 
	
	int sector_per_superpage = ps; 
	int sector_per_page = sector_per_superpage / superpage_degree; 

	Addr start_superpage = sector_address / sector_per_superpage; 
	Addr end_superpage = (sector_address + sector_size -1) / sector_per_superpage; 
	
	Addr start_lpn = start_superpage; 
	Addr end_lpn = end_superpage;  
	
	Addr start_page = (sector_address / sector_per_page) % superpage_degree;   
	Addr end_page = ((sector_address + sector_size -1) / sector_per_page) % superpage_degree;  
	
	page_address = start_lpn; 
	page_size = end_lpn - start_lpn + 1;

	service_latency = 0; 

	serviced_page = 0; 
	updated_page = 0; 
	done = false;

	request_masks = new Mask[page_size];
	for (int i = 0; i < page_size; i++){
		request_masks[i].reserve(superpage_degree, true); 
	}  
	
	for (int i = 0; i < start_page; i++){
		request_masks[0].set(i, false); 
	}
	for (int i = end_page +1 ; i < superpage_degree; i++){
		request_masks[page_size-1].set(i, false); 
	}
}
bool InputRequest::page_service_done(Tick latency){ // return true if all page requests has been completed 
	if (latency > service_latency)
		service_latency = latency; 
	serviced_page++; 
	if (serviced_page == page_size){
		done = true; 
		return true; 
	}
	return false;
}

bool InputRequest::page_status_updated(){
	updated_page++; 
	if (updated_page == page_size){
		return true; 
	}
	return false; 
}

OutputRequest::OutputRequest(long int sequence_number, Tick arrive_time, Addr lpn, Addr ppn, InputRequest * parent, int op){
	seq_num = sequence_number; 
	time = arrive_time; 
	operation = op; 
	logical_page_address = lpn; 
	parent_req = parent; 
	physical_page_address = ppn; 
	latency_reported = false; 
}
