#ifndef __FILE_MANANGER_H__
#define __FILE_MANANGER_H__

#include "page_structure.h"

#define HEADERPAGENUM 0
#define ROOTPAGENUM 1

/*
typedef uint64_t pagenum_t; 
struct page_t { 
// in-memory page structure 
};
*/

int db_fd;
header_page_t * header_page;
page_t * root_page;

//Open existing data file or create one if not existed
int open_db(char* pathname);

// Allocate an on-disk page from the free page list 
//pagenum_t file_alloc_page();
pagenum_t file_alloc_page();

// Free an on-disk page to the free page list 
void file_free_page(pagenum_t pagenum);

// Read an on-disk page into the in-memory page structure(dest) 
void file_read_page(pagenum_t pagenum, page_t* dest);

// Write an in-memory page(src) to the on-disk page 
void file_write_page(pagenum_t pagenum, const page_t* src);

#endif /* __FILE_MANAGER_H__*/
