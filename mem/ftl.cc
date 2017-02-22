//
//  FTL.cpp
//  FTL-3
//
//  Created by Narges on 7/3/15.
//  Copyright (c) 2015 narges shahidi. All rights reserved.
//

#include "mem/ftl.hh"
#include "mem/ftl_statistics.hh"

#define SECTOR_SIZE 512

FTL::FTL(Parameter *p, int disk){
	disk_number = disk; 
    param = p;
    
    map = new HybridMapping(this);
    que = new CommandQueue(this);    

	seq_num = 0; 
}

//Added for integration
FTL::FTL(GlobalConfig *g, int disk){
	disk_number = disk;
	Parameter *p = new Parameter();
	p->over_provide = g->FTLOP;
	p->superpage_degree = g->SuperpageDegree; 
	p->page_per_block = g->NumPage * g->SuperblockDegree / g->SuperpageDegree ;
	p->physical_block_number = g->GetTotalNumBlock() / g->SuperblockDegree; 
	p->logical_block_number = (double)p->physical_block_number * (1 - p->over_provide); 	
	p->physical_page_number = p->physical_block_number * p->page_per_block; 
	p->logical_page_number = p->logical_block_number * p->page_per_block; 
	p->warmup = g->Warmup; 	
	//ToDo: Maybe needed to add to ConfigReader
	p->gc_threshold = g->FTLGCThreshold;
	p->page_size = g->SizePage * g->SuperpageDegree / SECTOR_SIZE;
	p->mapping_N = g->FTLMapN;
	p->mapping_K = g->FTLMapK;
	p->erase_cycle = g->FTLEraseCycle; 
	param = p;
	map = new HybridMapping(this);
	que = new CommandQueue(this);
   
 	seq_num = 0;

}

FTL::~FTL(){
   
}

void FTL::initialize(){
	std::cout << "Total physical block/page/sector "  << param->physical_block_number << "  " << param->physical_page_number << " " << param->physical_page_number * param->page_size << endl; 
	std::cout << "Total logical block/page/sector "  << param->logical_block_number << "  " << param->logical_page_number << " " << param->logical_page_number * param->page_size << endl; 

	current_time = 0; 

	for (Addr i = 0; i < param->logical_block_number; i++){
		int to_fill_page_number = (param->page_per_block * param->warmup);
		if (to_fill_page_number > param->page_per_block)  { 
			cout << "error in initialization " << endl; 
			return; 
		}
		if (to_fill_page_number == 0) {
			cout << "initialization done! " << endl; 
			return; 
		}
		Addr sector_size = to_fill_page_number * param->page_size; 
		Addr lsn = i * param->page_per_block * param->page_size; 
		
		InputRequest *ireq = new InputRequest (0, OP_WRITE, lsn, sector_size, param->page_size, param->superpage_degree);
		for (Addr j = 0; j < ireq->page_size; j++){
			write(ireq->page_address+j, ireq , ireq->request_masks[j], false, true);	
		}
		delete ireq; 
	}
	std::cout << "Initialization done! " << std::endl;  
}

void FTL::read(Addr lpn, InputRequest * ireq, const  Mask & superpage_mask, bool synced, bool init ){ 
	Addr ppn;
	Mask read_mask (param->superpage_degree, true);  
	if (map->read(lpn, ppn, read_mask) == SUCCESS){
		if (read_mask.superset(superpage_mask)) 
			PAM_SendRequest(seq_num++, current_time, ppn,OP_READ, superpage_mask, synced, lpn, ireq, init); 
		else	 
			assert("read mask does not match"); 
	}
	else{
		assert("Unsuccessful read operation");
	}
        
}
void FTL::write(Addr lpn, InputRequest * ireq, const Mask & superpage_mask, bool synced, bool init ){

	Addr old_ppn; 
	Mask old_mask(param->superpage_degree, false);
	if (!init && map->getppn(lpn, old_mask, old_ppn) == SUCCESS) {
		old_mask.subtract(superpage_mask); 
		if (old_mask.any()) 
			PAM_SendRequest(seq_num++, current_time, old_ppn, OP_READ, old_mask, synced, lpn, NULL, init); 
	} 
	Mask write_mask(param->superpage_degree, true); 
	write_mask.assign_union(old_mask, superpage_mask); 
	Addr ppn; 
	if (map->write(lpn, ppn, write_mask) == SUCCESS){
		PAM_SendRequest(seq_num++, current_time, ppn, OP_WRITE, write_mask, synced, lpn, ireq, init);
		if (!init) {  
			ftl_statistics.host_epoch_superpage_utilization.update(write_mask.count() / (double)param->superpage_degree); 
			ftl_statistics.host_sim_superpage_utilization.update(write_mask.count() / (double)param->superpage_degree); 
		} 
	}
	else{
		assert("unsuccessful write");
	}
	if (map->need_gc()) {
		map->garbage_collect();
	}
}

void FTL::readTransaction(Addr lsn, int sector_size, Tick TransTick, bool synced, bool init){	
	ftl_statistics.read_req_arrive(TransTick); 
	current_time = TransTick; 
    //que->flushQueue();
	if (((lsn+sector_size)/param->page_size) >  param->logical_page_number ){
		my_assert( "read address, out of space! " );
		lsn = lsn % (param->logical_page_number * param->page_size); 
	}
	InputRequest * ireq = new InputRequest (TransTick, OP_READ, lsn, sector_size, param->page_size, param->superpage_degree);	

	for (int i = 0; i < ireq->page_size; i++){
		read(ireq->page_address + i , ireq , ireq->request_masks[i], synced);
	}

	if (!synced){
		delete ireq; 
	}
	
}

void FTL::writeTransaction(Addr lsn, int sector_size, Tick TransTick, bool synced, bool init){
	if(!init) ftl_statistics.write_req_arrive(TransTick); 

	current_time = TransTick; 
	// que->flushQueue(); 

	if (((lsn+sector_size)/param->page_size) >  param->logical_page_number ){
		my_assert( "write address, out of space! ");
		lsn = lsn % (param->logical_page_number * param->page_size); 
	}

	
	InputRequest * ireq = new InputRequest (TransTick, OP_WRITE, lsn, sector_size, param->page_size, param->superpage_degree);	
	for (Addr i = 0; i < ireq->page_size; i++){
		write(ireq->page_address + i, ireq, ireq->request_masks[i],  synced, init); 
	}
	if (!synced){
		delete ireq; 
	}

}
void FTL::PAM_SetLatency(long int seq_num, Tick latency){
	
	if (seq_num == -1) return; 
	std::map<long int, OutputRequest *>::iterator iOutReq = PAMMap.find(seq_num);
	if (iOutReq == PAMMap.end()){
		return;
	}	

	InputRequest * inReq = iOutReq->second->parent_req;
	if (inReq != NULL){
		if (inReq->done){
			if (inReq->page_status_updated()){ 	
			
				ftl_statistics.updateStats(inReq); 
				delete inReq; 
			}
			delete iOutReq->second;  
			PAMMap.erase(iOutReq);
			return;  
		}

		if (iOutReq->second->latency_reported){ 
			delete iOutReq->second; 
			PAMMap.erase(iOutReq); 
			return;
		} 
		else{ 
			iOutReq->second->latency_reported = true; 
		}
		bool done = inReq->page_service_done(latency);
	
		if (done){
			hil->setLatency(inReq->sector_address, inReq->service_latency);
		}
	}else {
		if (iOutReq->second->operation == OP_ERASE && (iOutReq->second->time + latency - 1 <= current_time)){
			// Update garbage Collection latency 
			map->updateStats(latency);
			delete iOutReq->second; 
			PAMMap.erase(iOutReq);
		}else if (iOutReq->second->operation != OP_ERASE){
			delete iOutReq->second; 
			PAMMap.erase(iOutReq); 
		} 
	}

		
}

void FTL::PAM_SendRequest(long int seq_num, Tick time, Addr ppn, int operation, const Mask & superpage_mask, bool synced, Addr lpn, InputRequest * ireq, bool init ){	
 
	if (synced){
		OutputRequest *oreq = new OutputRequest(seq_num, time, lpn, ppn, ireq, operation); 
		PAMMap.insert(std::pair<long int, OutputRequest *> (seq_num, oreq)); 
 	}
	if (!init){
		que->pushToQueue(seq_num, time, ppn, operation, superpage_mask, synced, init); 
	}
}

void FTL::PrintStats(Tick sim_time, bool final_call) { 
	if (sim_time > current_time) current_time = sim_time; 
	// release finished requests 
	que->flushQueue(); 
	que->PrintStats(); 
	map->PrintStats(); 
	ftl_statistics.print_stats(sim_time, final_call); 
} 
