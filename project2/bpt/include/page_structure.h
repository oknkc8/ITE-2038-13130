#ifndef __PAGE_STRUCTURE_H__
#define __PAGE_STRUCTURE_H__

#define PAGESIZE 4096

#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>

typedef uint64_t pagenum_t; 

typedef struct record_t{
    int64_t key;
    char value[120];
} record_t;

typedef struct branch_factor_t{
    int64_t key;
    pagenum_t child_page_offset_num;
} branch_factor_t;

typedef struct header_page_t{
    pagenum_t free_page_offset_num;
    pagenum_t root_page_offset_num;
    int64_t num_pages;
    char reserved[4072];
} header_page_t;

typedef struct page_t{
	pagenum_t parent_page_offset_num;	// if parent_page_offset_num is 0, this page is root
    int is_leaf;
    int num_keys;
    char reserved[104];
    pagenum_t pointer_num;
    union{
        branch_factor_t entries[248];
        record_t records[31];
    };
} page_t;

#endif /*__PAGE_STRUCTURE_H__*/
