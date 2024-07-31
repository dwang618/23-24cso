#define _XOPEN_SOURCE 600
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include "mlpt.h"                           //header file containing ptbr, translate, page_allocate
#include "config.h"                         //defines the constants LEVELS and POBITS

size_t ptbr = 0;
size_t page_size = 1 << POBITS;              //equivalent of 2 to the number of page offsize bits
size_t index_bits = POBITS - 3;


void page_allocate(size_t va) {
    size_t virtual_page_numbers[LEVELS];
    for (int x = 0; x < LEVELS; x += 1) {
        virtual_page_numbers[x] = ((1 << (index_bits)) - 1) & ((va >> POBITS) >> (index_bits) * (LEVELS - (x + 1)));            //populate virtual page array
    }

    if (ptbr == 0) {
        size_t *pagetable;
        if (posix_memalign((void **) &pagetable, page_size, page_size)) {
            printf("memalign error, allocation not working");
            return;
        }
        memset(pagetable, 0, page_size);
        ptbr = (size_t) pagetable;

    }      

    size_t *base_address_array = (size_t *) ptbr;  
    for (int x = 0; x < LEVELS; x += 1) {
        size_t pagetable_entry = virtual_page_numbers[x];
        size_t valid_bit = base_address_array[pagetable_entry] & 1;


        if (valid_bit == 0) {
            size_t *pagetable;
            if(posix_memalign((void **) &pagetable, page_size, page_size)) {
                printf("memalign error, allocation not working");
                return;
            }
            memset(pagetable, 0, page_size);

            base_address_array[pagetable_entry] = (size_t) pagetable;                                       ////update baseAddressArray to correspond to next level      
            base_address_array[pagetable_entry] = base_address_array[pagetable_entry] | 1;                  

        }   

        size_t least_sig_bit_mask = base_address_array[pagetable_entry] & (~1);                             //isolate physical page number by removing least sig bits
        base_address_array = (size_t *) (least_sig_bit_mask);                                               ////update current array to next level pagetable
    }
    
}   

size_t translate(size_t va) {
    size_t *pagetable;
    size_t virtual_page_numbers[LEVELS];

    if (ptbr == 0) {           //invalid address, cannot be converted to a physical location
        return ~0;
    }
    
    size_t page_table_entry;
    for (int x=0; x<LEVELS; x += 1) {
        virtual_page_numbers[x] = (va >> ((index_bits * LEVELS + POBITS) - ((x + 1) * index_bits))) & ((1 << index_bits) - 1);
    }
        
    pagetable = (size_t *) ptbr;
    for (int x = 0; x < LEVELS; x += 1) {
        page_table_entry = pagetable[*(virtual_page_numbers + x)];

        size_t valid_bit = (page_table_entry & 1);
        if(valid_bit != 0) {
            page_table_entry >>= POBITS;
            page_table_entry <<= POBITS;
            pagetable = (size_t *) page_table_entry;
                
        }
        else {
            return ~0;
        }
        
        
    }
    size_t offset_bits = va & (page_size - 1);
    return page_table_entry + offset_bits;           
}

void deallocate() {
    int index = LEVELS; 
    size_t* pagetable = (size_t *) ptbr;

    if (ptbr == 0) {                                                 //empty pagetable, no memory designated for lookup or allocation
        free(pagetable);
        return;
    }

    while (index > 0) {
        size_t* placeholder;
        for (int x = 0; x < 1 << (index_bits); x += 1) {
            size_t page_table_entry = *(pagetable + x);
            size_t valid_bit = page_table_entry & 1;
            if (valid_bit) {                                          //existing pagetable has a valid bit that must be replaced in case of further usage
                pagetable[x] = page_table_entry & ~1;                 //remove valid bit from existing entry
                pagetable = (size_t *)(page_table_entry & ~1);        //establish next level virtual address by removing valid bit
            }
        }

        placeholder = pagetable;                                    //store pointer to next level table lookup
        free(pagetable);                                            //free current page
        pagetable = placeholder;
        index -= 1;
    }

    ptbr = 0;                                                       //re-establish that no memory has been designated and pagetable has no levels setup
}


int main() {
    // 0 pages have been allocated
    assert(ptbr == 0);

    page_allocate(0x456789abcdef);
    // 5 pages have been allocated: 4 page tables and 1 data
    assert(ptbr != 0);

    page_allocate(0x456789abcd00);
    // no new pages allocated (still 5)
    
    int *p1 = (int *)translate(0x456789abcd00);
    *p1 = 0xaabbccdd;
    short *p2 = (short *)translate(0x456789abcd02);
    printf("%04hx\n", *p2); // prints "aabb\n"

    assert(translate(0x456789ab0000) == 0xFFFFFFFFFFFFFFFF);
    
    page_allocate(0x456789ab0000);
    // 1 new page allocated (now 6; 4 page table, 2 data)

    assert(translate(0x456789ab0000) != 0xFFFFFFFFFFFFFFFF);
    
    page_allocate(0x456780000000);
    // 2 new pages allocated (now 8; 5 page table, 3 data)
}