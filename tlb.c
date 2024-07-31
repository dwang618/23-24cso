#define _XOPEN_SOURCE 600
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include "mlpt.h"                           //header file containing ptbr, translate, page_allocate
#include "config.h"                         //defines the constants LEVELS and POBITS
#include "tlb.h"
#include <stddef.h>

#define NUM_SETS 16
#define NUM_WAYS 4

size_t tag_bits[NUM_SETS][NUM_WAYS];  // Tag bits
size_t pa[NUM_SETS][NUM_WAYS];   // pa
int valid_bit[NUM_SETS][NUM_WAYS];   // Valid bit
int lru[NUM_SETS][NUM_WAYS];     // LRU

size_t index_bits = 4;

/** invalidate all cache lines in the TLB */
void tlb_clear() {

    for (int set = 0; set < NUM_SETS; set++) {
        for (int way = 0; way < NUM_WAYS; way++) {
            valid_bit[set][way] = 0;  
            pa[set][way] = 0;  
            lru[set][way] = 0;  
            tag_bits[set][way] = 0;  

        }
    }

}
		
/**
 * return 0 if this virtual address does not have a valid
 * mapping in the TLB. Otherwise, return its LRU status: 1
 * if it is the most-recently used, 2 if the next-to-most,
 * etc.
 */
int tlb_peek(size_t va) {

    size_t tag_mask = va >> (index_bits + POBITS);
    size_t index_mask = (1 << index_bits) - 1;
    size_t set_number = (va & (index_mask << POBITS));
    set_number >>= POBITS;

    // Search for virtual address in TLB set
    for (int way = 0; way < NUM_WAYS; way++) {
        //valid bit must be 1 for entry to exist, va without index or offset bits must match
        if (valid_bit[set_number][way] && tag_bits[set_number][way] == tag_mask) {
            return lru[set_number][way];
        }
    }
    return 0;
}

 /* As an exception, if translate(va) returns -1, do not
 * update the TLB: just return -1. */
size_t translateHelper(size_t set_number, size_t way, size_t index_bits, size_t offset_mask, size_t va, size_t lru_va) {
    size_t pagenumber = translate((va >> POBITS) << POBITS);
    if(pagenumber < 0) {
        return -1;
    }
    
    // update the physical address table and fill in corresponding info bits -> lru, physical address, valid bits
    for(int way_index=0; way_index<NUM_WAYS; way_index++) {
        size_t lru_index = lru[set_number][way_index];
                
        //update lru to be most recent
        if(way_index==way) { 
            lru[set_number][way_index] = 1;
        }
        //uninitialized way in set, then skip
        else if(lru_index == 0) {
            continue;
        }
        //shift all other lru down 
        else if(lru_va == 0 || lru_va > lru_index) {
            lru[set_number][way_index]++;
        }

    }
    valid_bit[set_number][way] = 1;
    tag_bits[set_number][way] = va >> (index_bits + POBITS);
    pa[set_number][way] = pagenumber;     

    return pagenumber + offset_mask;
}
/**
 * If this virtual address is in the TLB, return its
 * corresponding physical address. If not, use
 * `translate(va)` to find that address, store the result
 * in the TLB, and return it. In either case, make its
 * cache line the most-recently used in its set.
 */
size_t tlb_translate(size_t va) {

    size_t offset_mask = (va & ((1 << POBITS) - 1));
    size_t tag_mask = va >> (index_bits + POBITS);  
    size_t index_mask = (1 << index_bits) - 1;
    size_t set_number = (va & (index_mask << POBITS));
    set_number >>= POBITS;

    // Search for virtual address in TLB set
    size_t lru_va;
    for (int way = 0; way < NUM_WAYS; way++) {

        //iterate until reaching 
        if (valid_bit[set_number][way] && tag_bits[set_number][way] == tag_mask) {       //cache hit case
            lru_va = lru[set_number][way];

            for(int way_index=0; way_index<NUM_WAYS; way_index++) {
                size_t lru_index = lru[set_number][way_index];
                
                //shift all other lru down 
                if(lru_index != 0) {
                    if(lru_va == 0 || lru_va > lru_index) {
                        lru[set_number][way_index]++;
                    }   
                }
                
                //update lru to be most recent
                else if(way_index==way) { 
                    lru[set_number][way_index] = 1;
                }
                

            }
            return pa[set_number][way] + offset_mask;

        }
        else if(!valid_bit[set_number][way]) {                                           //cache miss case
            lru_va = lru[set_number][way];

            for(int way_index=0; way_index<NUM_WAYS; way_index++) {

                //find first available set
                size_t lru_index = lru[set_number][way_index];
                if(lru_index == 0) {
                    return translateHelper(set_number, way_index, index_bits, offset_mask, va, lru_va);
                }

            }
        }
        else {     
            lru_va = lru[set_number][way];
            //no entry case or no available slots in the set
            for(int way_index=0; way_index<NUM_WAYS; way_index++) { 

                //find oldest entry and replace it
                size_t lru_index = lru[set_number][way_index];

                if (lru_index >= NUM_WAYS) {
                    return translateHelper(set_number, way_index, index_bits, offset_mask, va, lru_va);
                }

            }
        }
    }

    return 0;
}
