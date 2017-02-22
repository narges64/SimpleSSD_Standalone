//
//  CommandQueue.cpp
//  FTLSim_functional
//
//  Created by Narges on 6/26/15.
//  Copyright (c) 2015 narges shahidi. All rights reserved.
//

#include "mem/ftl_commandqueue.hh"
//#include "debug/FTLOut.hh"

void Command::to_string(int disk){
	
	switch (operation) {
	case OP_READ:
		printf( "FTL: disk[%d] %lld Read %lld \n", disk, curTick, page_address);
		break;
	case OP_WRITE:
		printf( "FTL: disk[%d] %lld Write %lld \n", disk, curTick, page_address);
		break;
	case OP_ERASE:
		printf( "FTL: disk[%d] %lld Erase %lld \n", disk, curTick, page_address);
		break;
	default:
		break;
	}

}
void Command::to_string(std::ofstream &file, int disk){
	switch (operation) {
	case OP_READ:
		printf("FTL: disk[%d] %lld Read %lld \n", disk, curTick, page_address);
		break;
	case OP_WRITE:
		printf( "FTL: disk[%d] %lld Write %lld \n", disk, curTick, page_address);
		break;
	case OP_ERASE:
		printf("FTL: disk[%d] %lld Erase %lld \n", disk, curTick, page_address);
		break;
	default:
		break;
	}
}
// ===========================================================
CommandQueue::CommandQueue(FTL * f){
    command_count = 0;
    queue_head = NULL;
    queue_tail = NULL;
	ftl = f; 
	
	ResetStats(); 
}
CommandQueue::~CommandQueue(){}
Command * CommandQueue::popFromQueue(){
    if (queue_head == NULL) return NULL;
    Command * com = queue_head;
    queue_head = queue_head->next_command;
    command_count--;
    return com;
}
void CommandQueue::flushQueue(){
	Command * prev_com = NULL; 
	Command * com = getFromQueue(0); 	
	while (com) {	
		if (com->synced){ 
			ftl->PAM_SetLatency(com->seq_num, com->finished_time - com->arrived_time + 1); 
		}
		if (!com->initial){
			if (com->finished_time != 0 && com->finished_time <= ftl->current_time){
				updateStats(com);
				removeCommand(&prev_com, &com); // com is going to be the next command after calling this 
			}else {
				prev_com = com; 
				com = com->next_command; 
			}
		}else {
			if (com->synced) 
				ftl->PAM_SetLatency(com->seq_num, com->finished_time - com->arrived_time + 1); 
			removeCommand(&prev_com, &com); 
		}
	}
}

void CommandQueue::removeCommand(Command ** prev, Command ** com){

	command_count--; 	
	if ((*com) == queue_tail){
		queue_tail = (*prev);
		if ((*prev) != NULL) { 
			(*prev)->next_command = NULL;
			
		} else {
			queue_head = NULL; 
		}
		delete (*com); 
		(*com) = NULL; 
		return;  
	}
		
	if ((*com) == queue_head){
		queue_head = (*com)->next_command; 
		delete (*com);
		(*com) = queue_head;  
		return; 
	}
	
	(*prev)->next_command = (*com)->next_command; 
	delete (*com); 
	(*com) = (*prev)->next_command; 
}
int CommandQueue::pushToQueue(long int seq_num, Tick time, Addr address, int type, const Mask & mask, bool synced, bool init = false){
	if (address == -1){
		assert("wrong command pushed to the queue"); 
	}
	Command * com = new Command(seq_num, time, address, type, mask, synced, init);

	if (queue_head == NULL){
		queue_head = com;
		queue_tail = com;
	}else {
		queue_tail->next_command = com;
		queue_tail = com;	
	}
    	command_count++;
	return command_count-1;
}
Command * CommandQueue::getFromQueue(long int index){
	
    if (index >= command_count) return NULL;
    Command * com = queue_head;
    while (index != 0) {
        com = com->next_command;
        index--;
    }
    return com;
	 
}

void CommandQueue::addToTail(Command * com){
    if (queue_head == NULL){
        queue_head = com;
        queue_tail = com;
    }else {
        queue_tail->next_command = com;
        queue_tail = com;
    }
    
    command_count++;
	
}
void CommandQueue::ResetStats(){
    // resetting Statistics
	pam_read_req_count = 0; 
	pam_write_req_count = 0; 
	pam_erase_req_count = 0; 

	pam_read_capacity = 0; // sector
	pam_write_capacity = 0; // sector 

	pam_read_lat_min = DBL_MAX;
	pam_read_lat_max = 0; 
	pam_read_lat_avg = 0; 
	
	pam_write_lat_min = DBL_MAX;  
	pam_write_lat_max = 0; 
	pam_write_lat_avg = 0; 
	
	pam_erase_lat_min = DBL_MAX; 
	pam_erase_lat_max = 0; 
	pam_erase_lat_avg = 0; 	
}

void CommandQueue::PrintStats(){
    // printing Statistics

	flushQueue(); 	
	
	printf("FTL PAM read request count %d \n", pam_read_req_count);
	printf("FTL PAM write request count %d \n", pam_write_req_count);
	printf("FTL PAM erase request count %d \n", pam_erase_req_count);

	printf("FTL PAM read capacity %.2f MB \n", pam_read_capacity * 512 / MBYTE);
	printf("FTL PAM write capacity %.2f MB \n", pam_write_capacity * 512 / MBYTE);

	if (pam_read_lat_min == DBL_MAX) 
		printf("FTL PAM read latency (min,max,avg) ( NA , NA , NA ) us \n");
	else
		printf("FTL PAM read latency (min,max,avg) ( %.2f , %.2f , %.2f ) us \n", pam_read_lat_min, pam_read_lat_max, pam_read_lat_avg);
 	
	if (pam_write_lat_min == DBL_MAX) 
		printf("FTL PAM write latency (min,max,avg) ( NA , NA , NA ) us \n");
	else 
		printf("FTL PAM write latency (min,max,avg) ( %.2f , %.2f , %.2f ) us \n", pam_write_lat_min, pam_write_lat_max, pam_write_lat_avg);
	
	if (pam_erase_lat_min == DBL_MAX) 
		printf("FTL PAM erase latency (min,max,avg) ( NA , NA , NA ) us \n");
	else 
		printf("FTL PAM erase latency (min,max,avg) ( %.2f , %.2f , %.2f ) us \n", pam_erase_lat_min, pam_erase_lat_max, pam_erase_lat_avg);
	
}

void CommandQueue::updateStats(Command * com){
	
	Tick command_latency = (com->finished_time - com->arrived_time + 1) / USEC; // convert ps to us  

	switch (com->operation) {
		case OP_READ:
			if (command_latency < pam_read_lat_min)
				pam_read_lat_min = command_latency; 
			if (command_latency > pam_read_lat_max) 
				pam_read_lat_max = command_latency; 

			pam_read_lat_avg = pam_read_lat_avg * ((double)pam_read_req_count / (pam_read_req_count + 1)) + (command_latency / (double)(pam_read_req_count + 1)); 	
			pam_read_req_count++; 
	    	pam_read_capacity += ftl->param->page_size; 
			break;
		case OP_WRITE: 
			if (command_latency < pam_write_lat_min)
				pam_write_lat_min = command_latency; 
			if (command_latency > pam_write_lat_max) 
				pam_write_lat_max = command_latency; 

			pam_write_lat_avg = pam_write_lat_avg * ((double)pam_write_req_count / (pam_write_req_count + 1)) + (command_latency / (double)(pam_write_req_count + 1)); 	
			pam_write_req_count++; 
	    	pam_write_capacity += ftl->param->page_size; 
	
			break; 
		case OP_ERASE: 
			if (command_latency < pam_erase_lat_min)
				pam_erase_lat_min = command_latency; 
			if (command_latency > pam_erase_lat_max) 
				pam_erase_lat_max = command_latency; 

			pam_erase_lat_avg = pam_erase_lat_avg * ((double)pam_erase_req_count / (pam_erase_req_count + 1)) + (command_latency / (double)(pam_erase_req_count + 1)); 	

			pam_erase_req_count++; 
			break; 
	}
}
