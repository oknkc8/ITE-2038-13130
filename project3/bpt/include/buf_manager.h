#ifndef __BUF_MANANGER_H__
#define __BUF_MANANGER_H__

#include "structure.h"
#include "file_manager.h"

#define HEADERPAGENUM 0
#define ROOTPAGENUM 1

int buf_count, buf_start, buf_size;
buffer_t * buf;
remain_t * buf_remain;

int close_table(int table_id);

int init_db(int num_buf);

int shutdown_db(void);

// get page from file and put into buffer pool
buffer_t * file_get_page(int table_id, pagenum_t pagenum);

void file_put_page(int table_id, buffer_t * put_buf);

buffer_t * buf_get_page(int table_id, pagenum_t pagenum);

void buf_put_page(buffer_t * put_buf);

void buf_free_page(int table_id, pagenum_t pagenum);

buffer_t * make_leaf(int table_id);

buffer_t * make_node(int table_id);

void print_buf(void);

#endif /* __BUF_MANAGER_H__*/