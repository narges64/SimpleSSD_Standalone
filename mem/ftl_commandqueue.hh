//
//  CommandQueue.h
//  FTLSim_functional
//
//  Created by Narges on 6/25/15.
//  Copyright (c) 2015 narges shahidi. All rights reserved.
//

#ifndef __FTLSim_functional__CommandQueue__
#define __FTLSim_functional__CommandQueue__
#include <iostream>
#include <fstream>
#include <istream>

#include "mem/ftl_defs.hh"
#include "mem/ftl.hh"

class FTL; 

class Command{
public:
    
    Command(long int sn, Tick t, Addr addr, int op, const Mask & mask, bool syn = false, long int seq_num = -1, bool init = false):seq_num(sn),arrived_time(t),page_address(addr),operation(op),synced(syn), initial (init), next_command(NULL), finished_time(0){
		superpage_mask.assign(mask);
		 
	}
	long int seq_num; 
    Tick arrived_time; 
	Addr page_address;
	int operation;
	bool synced;
	bool initial; 
	Command * next_command;
	void to_string(int disk); 
	void to_string(std::ofstream & file, int disk);
	Tick finished_time;
	Mask superpage_mask; 
};

class CommandQueue{
private:
	FTL * ftl; 
	Command * queue_head;
	Command * queue_tail;
    	int command_count;
    
    
	
public:
    CommandQueue(FTL * f); 
    ~CommandQueue();
    
	// Statistics
	int pam_read_req_count; 
	int pam_write_req_count; 
	int pam_erase_req_count; 

	double pam_read_capacity; // sector (512 Byte) 
	double pam_write_capacity; // sector (512 Byte) 

	double pam_read_lat_min; 
	double pam_read_lat_max; 
	double pam_read_lat_avg; 

	double pam_write_lat_min;  
	double pam_write_lat_max; 
	double pam_write_lat_avg; 

	double pam_erase_lat_min; 
	double pam_erase_lat_max; 
	double pam_erase_lat_avg; 
	
    int pushToQueue(long int seq_num, Tick time, Addr address, int type, const Mask & mask, bool synced, bool init);
    void addToTail(Command * com);
	void removeCommand(Command ** prev, Command ** com);  
	Command * popFromQueue();
    Command * getFromQueue(long int index);
    void flushQueue();
    int get_command_count(){return command_count;}
    void PrintStats();
	void ResetStats();
	void updateStats(Command * com);  
    
};
#endif /* defined(__FTLSim_CommandQueue__) */
