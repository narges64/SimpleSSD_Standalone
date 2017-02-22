//
//  Block.h
//  FTLSim_functional
//
//  Created by Narges on 6/24/15.
//  Copyright (c) 2015 narges shahidi. All rights reserved.
//

#ifndef FTLSim_functional_Block_h
#define FTLSim_functional_Block_h

#include <iostream>

#define INT_SIZE 32



class Block{
private:
    
public:
    enum	PAGE_STATE {PAGE_VALID = 0, PAGE_INVALID, PAGE_FREE};
    int		page_per_block;
    Addr	block_number;
    int		erase_count;
    unsigned int*	page_bit_map;         
    Addr             page_sequence_number;   	// page within the block have to be written in order
    bool bad_block; 				// true: bad_block false:good_block
    Mask * superpage_masks;     

    Block 		 (){} 
    void initialize	(int page_per_block, Addr bn, int superpage_degree);          // initialize page_bit_map, etc.
    void erase_block     ();                                     // add erase count, reset other variables to initial state
    void set_page_state  (int page_offset, PAGE_STATE state);    // change page state: PAGE_VALID or PAGE_INVALID
    int get_page_state   (int page_offset);                      
	void get_page_mask(int page_offset, Mask & mask); 	
    bool is_empty        ();                                     
    bool is_full         ();                                     
    STATE write_page     (Addr logical_page, const Mask & mask, int &page_offset);  // write a new page into the block.
                                                                 // if page_offset == -1: write using page_sequence_number
                                                                 // otherwise, invalid all free pages until reach to
                                                                 // this page
    int valid_page_count ();                                     // return the number of valid pages within the block
    int free_page_count  ();                                     // return number of free pages inside the block
    void to_string       ();                                     // some print form of the block (use for debug)
    
};


#endif
