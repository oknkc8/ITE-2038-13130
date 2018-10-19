#include "file_manager.h"
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

//Open existing data file or create one if not existed
int open_db(char* pathname){
	db_fd = open(pathname, O_RDWR | O_CREAT | O_EXCL | O_SYNC, 0777); 
	header_page = (header_page_t*)calloc(1, PAGESIZE);

	// If data file not existed
	if(db_fd > 0){
		header_page->num_pages = 1;
		int flag = pwrite(db_fd, header_page, PAGESIZE, SEEK_SET);
		if(flag < PAGESIZE){
			printf("Failed to write header page\n");
			exit(EXIT_FAILURE);
		}
		return 0;
	}

	// If data file existed, open it
	db_fd = open(pathname, O_RDWR | O_SYNC);
	if(db_fd > 0){
		int flag = pread(db_fd, header_page, PAGESIZE, SEEK_SET);
		if(flag < PAGESIZE){
			printf("Failed to read header page\n");
			exit(EXIT_FAILURE);
		}

		if(header_page->root_page_offset_num){
			root_page = (page_t*)calloc(1, PAGESIZE);
			flag = pread(db_fd, root_page, PAGESIZE, header_page->root_page_offset_num);
			if(flag < PAGESIZE){
				printf("Failed to read root page\n");
				exit(EXIT_FAILURE);
			}
		}
		else{
			printf("DB file is empty\n");
		}

		return 0;
	}

	printf("Faild to open db file %s\n", pathname);
	return -1;
}

// Allocate an on-disk page from the free page list 
pagenum_t file_alloc_page(){
	return header_page->free_page_offset_num;
}

// Free an on-disk page to the free page list 
void file_free_page(pagenum_t pagenum){
	page_t* page = (page_t*)calloc(1, PAGESIZE);
	file_read_page(pagenum, page);
	memset(page, 0, PAGESIZE);
	page->pointer_num = header_page->free_page_offset_num;
	header_page->free_page_offset_num = pagenum;
	file_write_page(HEADERPAGENUM, header_page);
	free(page);
}

// Read an on-disk page into the in-memory page structure(dest) 
void file_read_page(pagenum_t pagenum, page_t* dest){
	int flag = pread(db_fd, dest, PAGESIZE, PAGESIZE * pagenum);
	if(flag < PAGESIZE){
		printf("Failed to read page\n");
		exit(EXIT_FAILURE);
	}
}

// Write an in-memory page(src) to the on-disk page 
void file_write_page(pagenum_t pagenum, const page_t* src){
	int flag = pwrite(db_fd, src, PAGESIZE, PAGESIZE * pagenum);
	if(flag < PAGESIZE){
		printf("Failed to write page\n");
		exit(EXIT_FAILURE);
	}
}
