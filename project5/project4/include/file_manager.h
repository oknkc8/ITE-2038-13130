#ifndef __FILE_MANANGER_H__
#define __FILE_MANANGER_H__

#include "structure.h"

#define HEADERPAGENUM 0
#define ROOTPAGENUM 1

//Open existing data file or create one if not existed
int open_table(char* pathname, int num_column);

// Allocate an on-disk page from the free page list 
//pagenum_t file_alloc_page();

// Free an on-disk page to the free page list 
//void file_free_page(pagenum_t pagenum, int table_id);

// Read an on-disk page into the in-memory page structure(dest) 
void file_read_page(pagenum_t pagenum, page_t* dest, int table_id);

// Write an in-memory page(src) to the on-disk page 
void file_write_page(pagenum_t pagenum, const page_t* src, int table_id);

#endif /* __FILE_MANAGER_H__*/