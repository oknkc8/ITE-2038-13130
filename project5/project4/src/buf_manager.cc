#include "buf_manager.h"
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>	

int buf_count, buf_start, buf_size;
buffer_t * buf;
remain_t * buf_remain;

int table_fd[10], table_fd_count, table_col[10];
char table_id_arr[10][100];

int close_table(int table_id){
	int buf_idx = buf_start;
	buffer_t prev_buf;

	// find buf has same table_id from buffer pool
	while(buf_idx != -1){
		if(buf[buf_idx].table_id == table_id){
			// if buffer is dirty, sync it
			if(buf[buf_idx].is_dirty)
				file_put_page(table_id, &buf[buf_idx]);

			// delete buf from buffer pool
			if(buf_idx == buf_start){
				remain_t * new_buf_remain = (remain_t*)calloc(1,sizeof(remain_t));
				new_buf_remain->buf_idx = buf_idx;
				new_buf_remain->next = buf_remain;
				buf_remain = new_buf_remain;

				buf_start = buf[buf_idx].next_LRU;
			}
			else{
				remain_t * new_buf_remain = (remain_t*)calloc(1,sizeof(remain_t));
				new_buf_remain->buf_idx = buf_idx;
				new_buf_remain->next = buf_remain;
				buf_remain = new_buf_remain;

				prev_buf.next_LRU = buf[buf_idx].next_LRU;
			}
			buf_idx = buf[buf_idx].next_LRU;
			buf_count--;
			continue;
		}
		prev_buf = buf[buf_idx];
		buf_idx = buf[buf_idx].next_LRU;
	}

	// discard table_id
	int i, j;
	for(i = 0; i < table_fd_count; i++){
		if(table_fd[i] == table_id){
			for(j = i; j < table_fd_count - 1; j++){
				table_fd[j] = table_fd[j+1];
				strcpy(table_id_arr[j], table_id_arr[j+1]);
			}
			table_fd_count--;
			break;
		}
	}
}

int init_db(int num_buf){
	buf = (buffer_t*)malloc(sizeof(buffer_t) * num_buf);
	buf_count = 0;
	buf_start = -1;
	buf_size = num_buf;
//printf("\ninit_db fucntion\n");
	// init remain
	int i;
	buf_remain = NULL;
	for(i = num_buf - 1; i >= 0; i--){
		remain_t * new_remain = (remain_t*)calloc(1,sizeof(remain_t));
		//printf("\ninit_db fucntion\n");
		new_remain->buf_idx = i;
		new_remain->next = buf_remain;
		buf_remain = new_remain;
	}
//	printf("\ninit_db fucntion\n");
}

int shutdown_db(void){
	while(table_fd_count){
		close_table(table_fd[0]);
	}

	int buf_idx = buf_start, i;
	while(buf_idx != -1){
		if(buf[buf_idx].is_dirty)
			file_put_page(buf[buf_idx].table_id, &buf[buf_idx]);
		buf_idx = buf[buf_idx].next_LRU;
	}
	free(buf);

	return 0;
}

buffer_t * file_get_page(int table_id, pagenum_t pagenum){
	buffer_t * ret_buf = (buffer_t*)calloc(1,sizeof(buffer_t));
	//printf("%d\n", pagenum);
	if(pagenum == HEADERPAGENUM) 
		file_read_page(pagenum, (page_t*)(&(ret_buf->header_page)), table_id);	// header page
	else 
		file_read_page(pagenum, &(ret_buf->page), table_id);		// other page
	//printf("c2\n");
	ret_buf->table_id = table_id;
	ret_buf->pagenum = pagenum;
	return ret_buf;
}

//void file_put_page(int table_id, pagenum_t pagenum){
void file_put_page(int table_id, buffer_t * put_buf){
	if(put_buf->pagenum == HEADERPAGENUM)		// header page
		file_write_page(put_buf->pagenum, (page_t*)(&(put_buf->header_page)), table_id);
	else
		file_write_page(put_buf->pagenum, &(put_buf->page), table_id);
}

buffer_t * buf_get_page(int table_id, pagenum_t pagenum){
	//printf("\nbuf_get_page\n");
	int buf_idx = buf_start;
	while(buf_idx != -1){
		if(buf[buf_idx].table_id == table_id && buf[buf_idx].pagenum == pagenum)
			break;
		buf_idx = buf[buf_idx].next_LRU;
	}

	//printf("check1\n");
	// required page exist
	if(buf_idx != -1){
		buf[buf_idx].is_pinned ++;
		return &buf[buf_idx];
	}
	//printf("check2\n");
	// required page not exist
	buffer_t * new_buf = file_get_page(table_id, pagenum);
	new_buf->is_pinned++;
	new_buf->next_LRU = buf_start;
	//printf("check3\n");

	//printf("check3\n");
	if(buf_count < buf_size){
		//printf("check4\n");
		int new_buf_idx = buf_remain->buf_idx;
		buf[new_buf_idx] = *new_buf;
		buf_start = new_buf_idx;
		buf_count++;

		remain_t * d_buf = buf_remain;
		buf_remain = d_buf->next;
		//free(d_buf);
		
		//free(new_buf);
		//printf("check6\n");
		return &(buf[new_buf_idx]);
	}
	else{
		//printf("check5\n");
		// applying LRU policy
		buf_idx = buf_start;
		buffer_t * out_buf, * prev_buf = NULL, * t_prev_buf = NULL;
		int out_buf_idx;
		//printf("%d\n",buf_idx);
		//print_buf();
		while(buf_idx != -1){
			//printf("%d ",buf[buf_idx].pagenum);
			if(!buf[buf_idx].is_pinned){
				//printf("is pin\n");
				out_buf = &buf[buf_idx];
				out_buf_idx = buf_idx;
				prev_buf = t_prev_buf;
			}
			t_prev_buf = &buf[buf_idx];
			buf_idx = buf[buf_idx].next_LRU;
		}
		if(prev_buf != NULL)
			prev_buf->next_LRU = out_buf->next_LRU;
		else
			buf_start = out_buf->next_LRU;

		//printf("check6\n");
		if(out_buf->is_dirty)
			file_put_page(table_id, out_buf);

		buf[out_buf_idx] = *new_buf;
		buf[out_buf_idx].next_LRU = buf_start;
		//buf[out_buf_idx].is_pinned--;
		buf_start = out_buf_idx;

		//free(new_buf);
		return &(buf[out_buf_idx]);
	}
}

void buf_put_page(buffer_t * put_buf){
	//printf("\nbuf_put_page\n");
	int buf_idx = buf_start, put_buf_idx;
	buffer_t * prev_buf = NULL;
	while(buf_idx != -1){
		if(buf[buf_idx].table_id == put_buf->table_id && buf[buf_idx].pagenum == put_buf->pagenum){
			put_buf_idx = buf_idx;
			break;
		}
		prev_buf = &buf[buf_idx];
		buf_idx = buf[buf_idx].next_LRU;
	}
	//printf("check1\n");
	//printf("buf_start %d\nbuf_idx %d\n",buf_start,buf_idx);
	//printf("%d %d\n", put_buf_idx, put_buf->next_LRU);
	if(prev_buf != NULL)
		prev_buf->next_LRU = put_buf->next_LRU;
	else{
		if(put_buf->next_LRU == -1){
			//printf("dfadsf %d\n", buf[put_buf_idx].is_pinned);
			buf[put_buf_idx].is_pinned--;
			buf[put_buf_idx].next_LRU = -1;
			return;
		}
		//printf("dffsafsafsd\n");
		buf_start = put_buf->next_LRU;
	}

	//printf("check2\n");
	buf_idx = buf_start;
	//printf("%d\n",buf_idx);
	while(buf[buf_idx].next_LRU != -1){
		buf_idx = buf[buf_idx].next_LRU;
		//printf("%d ",buf_idx);
	}
	buf[buf_idx].next_LRU = put_buf_idx;
	buf[put_buf_idx].is_pinned--;
	buf[put_buf_idx].next_LRU = -1;
	//printf("check3\n");
}

pagenum_t buf_alloc_page(int table_id){
	buffer_t * header_buf = buf_get_page(table_id, HEADERPAGENUM);
	pagenum_t ret_pagenum = header_buf->header_page.free_page_offset_num;
	buf_put_page(header_buf);
	return ret_pagenum;
}

void buf_free_page(int table_id, pagenum_t pagenum){
	//printf("\nbuf free page\n");
	buffer_t * free_buf = buf_get_page(table_id, pagenum), * header_buf = buf_get_page(table_id, HEADERPAGENUM);
	memset(&free_buf->page, 0, sizeof(free_buf->page));
	free_buf->page.pointer_num = header_buf->header_page.free_page_offset_num;
	free_buf->is_dirty = 1;
	header_buf->header_page.free_page_offset_num = pagenum;
	header_buf->is_dirty = 1;
	buf_put_page(free_buf);
	buf_put_page(header_buf);
}

buffer_t * make_leaf(int table_id){
	buffer_t * ret_buf = make_node(table_id);
	ret_buf->page.is_leaf = true;
	return ret_buf;
}

buffer_t * make_node(int table_id){
	page_t * new_page = (page_t*)calloc(1,PAGESIZE);
	buffer_t * header_buf = buf_get_page(table_id, HEADERPAGENUM);
	file_write_page(header_buf->header_page.num_pages, new_page, table_id);
	free(new_page);
	//printf("%d\n",header_buf->header_page.num_pages);
	buffer_t * new_buf = buf_get_page(table_id,header_buf->header_page.num_pages);
	header_buf->header_page.num_pages++;
	header_buf->is_dirty = 1;
	buf_put_page(header_buf);
	buf_put_page(new_buf);
	return new_buf;
}

void print_buf(void){
	int buf_idx = buf_start;
	printf("\n-----------------------------------------\n");
	printf("Buffer Pool\n");
	while(buf_idx != -1){
		printf("table_id : %d   pagenum : %lld	idx : %d 	is_pinned : %d 		is_dirty : %d\n", buf[buf_idx].table_id, buf[buf_idx].pagenum, buf_idx, buf[buf_idx].is_pinned, buf[buf_idx].is_dirty);
		buf_idx = buf[buf_idx].next_LRU;
	}
	printf("-----------------------------------------\n\n");
}

int get_num_col(int table_id){
    return table_col[table_id - 1];
}