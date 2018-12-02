#include "file_manager.h"
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

extern int table_fd[10], table_fd_count, table_col[10];
extern char table_id_arr[10][100];

// Open existing data file or create one if not existed
// If success, return table_id
// If failed, return -1 
int open_table(char* pathname, int num_column){
	if(table_fd_count >= 10) return -1;

	int db_fd = open(pathname, O_RDWR | O_CREAT | O_EXCL | O_SYNC, 0777); 
	header_page_t * header_page = (header_page_t*)calloc(1, PAGESIZE);

	// If data file not existed
	if(db_fd > 0){
		header_page->num_pages = 1;
		header_page->num_column = num_column;
		int flag = pwrite(db_fd, header_page, PAGESIZE, SEEK_SET);
		if(flag < PAGESIZE){
			printf("Failed to write header page (table_id : %d)\n", table_fd_count);
			exit(EXIT_FAILURE);
		}
		table_fd[table_fd_count] = db_fd;
		table_col[table_fd_count] = num_column + 1;
		strcpy(table_id_arr[table_fd_count++], pathname);
		printf("%d\n",db_fd);
		return table_fd_count;
	}

	// If file is already opened, return table_id
	int i, ret_table_id;
	for(i = 0; i < table_fd_count; i++){
		if(!strcmp(table_id_arr[i], pathname)){
			ret_table_id = i + 1;
			//printf("find fd\n");
			break;
		}
	}
	if(i != table_fd_count) return ret_table_id;

	// If data file existed, open it
	db_fd = open(pathname, O_RDWR | O_SYNC);
	if(db_fd > 0){
		int flag = pread(db_fd, header_page, PAGESIZE, SEEK_SET);
		if(flag < PAGESIZE){
			printf("Failed to read header page\n");
			exit(EXIT_FAILURE);
		}

		if(!header_page->root_page_offset_num){
			printf("DB file is empty\n");
		}

		ret_table_id = 0;
		for(i = 0; i < table_fd_count; i++){
			if(!strcmp(table_id_arr[i], pathname)){
				ret_table_id = i + 1;
				//printf("find fd\n");
				break;
			}
		}
		if(i == table_fd_count){
			table_fd[table_fd_count] = db_fd;
			table_col[table_fd_count] = num_column + 1;
			strcpy(table_id_arr[table_fd_count++], pathname);
			ret_table_id = table_fd_count;
			//printf("%d\n",db_fd);
		}

		//printf("dfasdf\n");
		return ret_table_id;
	}

	printf("Faild to open db file %s\n", pathname);
	return -1;
}

// Read an on-disk page into the in-memory page structure(dest) 
void file_read_page(pagenum_t pagenum, page_t* dest, int table_id){
	//printf("%d\n", table_id);
	int flag = pread(table_fd[table_id - 1], dest, PAGESIZE, PAGESIZE * pagenum);
	if(flag < PAGESIZE){
		printf("Failed to read page\n");
		exit(EXIT_FAILURE);
	}
}

// Write an in-memory page(src) to the on-disk page 
void file_write_page(pagenum_t pagenum, const page_t* src, int table_id){
	int flag = pwrite(table_fd[table_id - 1], src, PAGESIZE, PAGESIZE * pagenum);
	if(flag < PAGESIZE){
		printf("Failed to write page\n");
		exit(EXIT_FAILURE);
	}
}
