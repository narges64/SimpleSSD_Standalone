//
//  Defs.h
//  FTLSim_functional
//
//  Created by Narges on 6/24/15.
//  Copyright (c) 2015 narges shahidi. All rights reserved.
//

#ifndef FTLSim_functional_Defs_h
#define FTLSim_functional_Defs_h

#include <fstream>
#include <istream>
#include <vector>

#include <assert.h>
#include <stdint.h>
#include <iostream>
#include "base/types.hh"
#include "mem/SimpleSSD_types.h"

using namespace std; 
#define EPOCH_INTERVAL 50000000000 

enum STATE {ERROR = -1, FAIL, SUCCESS, RESERVED};
enum OPERATION{OP_READ, OP_WRITE, OP_ERASE };

inline void my_assert(const char message[]){
	printf( "FTL: ERROR %s \n", message);
}

class Parameter{
public:
    	int page_per_block;
	
    	Addr physical_page_number;
    	Addr logical_page_number;
	Addr physical_block_number; 
	Addr logical_block_number; 	
	int mapping_N;
    	int mapping_K;
    	double gc_threshold;
    	int page_size; // sector per page 
	double over_provide;
	double warmup; 
	int erase_cycle; 
	int superpage_degree; 
	void to_string(){
		std::cout << std::endl; 
		std::cout << "Start simulation for N: " << mapping_N << " and K: " << mapping_K << std::endl;
		std::cout << "  page/block=" << page_per_block << " over-provisioning rate: " << over_provide <<  std::endl << std::endl;
	}
    
};

class Mask {
private: 
	vector<bool> bits; 
public: 
	Mask(){}
	Mask(int size, bool value){
		reserve(size, value); 
	}
	void reserve(int size, bool value) {
		bits.resize(size); 
		set(value); 
	}	
	void set(bool value){
		for (int i = 0; i < bits.size(); i++){
			bits[i] = value; 
		}
	}
	void set(int index, bool value){
		if (index > bits.size()) return; 
		bits[index] = value; 
	}
	bool equal(const Mask & other_mask) const{
		if (size() != other_mask.size()) return false; 
		for (int i = 0; i < size(); i++){
			if (bits[i] != other_mask.get(i)) return false;  
		}
		return true; 
	}
	bool superset(const Mask & other_mask) const {
		if (size() != other_mask.size()) return false; 
		for (int i = 0; i < size(); i++){
			if ((other_mask.get(i) == true) &&  (bits[i] == false)) return false; 
		}
		return true; 
	}
	void assign(const Mask & value) {
		reserve(value.size(), true); 
		for (int i = 0; i < size(); i++){
			bits[i] = value.get(i);  
		}
	}
	bool get(int index) const{
		if (index > bits.size()) return false; 
		return bits[index]; 
	}
	int size() const {
		return bits.size(); 
	}
	void subtract(const Mask & mask){
		if (size() != mask.size()) return; 
		for (int i = 0; i < size(); i++){
			if (mask.get(i)) 
				bits[i] = false; 
		}
	}
	void assign_union(const Mask & op1 , const Mask & op2){
		if (op1.size() != op2.size() || op1.size() != size()) return; 
		for (int i = 0; i < size(); i++){
			bits[i] = op1.get(i) | op2.get(i); 
		}
	}
	bool any(){
		for (int i = 0; i < size(); i++){
			if (bits[i]) return true; 
		}
		return false; 
	}
	int count(){
		int c = 0; 
		for (int i = 0; i < size(); i++){
			if (bits[i]) c++; 
		}
		return c; 
	}
};


#endif
