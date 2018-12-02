/*
 *  bpt.c  
 */
#define Version "1.14"
/*
 *
 *  bpt:  B+ Tree Implementation
 *  Copyright (C) 2010-2016  Amittai Aviram  http://www.amittai.com
 *  All rights reserved.
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright notice, 
 *  this list of conditions and the following disclaimer.
 *
 *  2. Redistributions in binary form must reproduce the above copyright notice, 
 *  this list of conditions and the following disclaimer in the documentation 
 *  and/or other materials provided with the distribution.
 
 *  3. Neither the name of the copyright holder nor the names of its 
 *  contributors may be used to endorse or promote products derived from this 
 *  software without specific prior written permission.
 
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 *  POSSIBILITY OF SUCH DAMAGE.
 
 *  Author:  Amittai Aviram 
 *    http://www.amittai.com
 *    amittai.aviram@gmail.edu or afa13@columbia.edu
 *  Original Date:  26 June 2010
 *  Last modified: 17 June 2016
 *
 *  This implementation demonstrates the B+ tree data structure
 *  for educational purposes, includin insertion, deletion, search, and display
 *  of the search path, the leaves, or the whole tree.
 *  
 *  Must be compiled with a C99-compliant C compiler such as the latest GCC.
 *
 *  Usage:  bpt [order]
 *  where order is an optional argument
 *  (integer MIN_ORDER <= order <= MAX_ORDER)
 *  defined as the maximal number of pointers in any node.
 *
 */

#include "bpt.h"
#include <string.h>
#include <inttypes.h>

// GLOBALS.

/* The order determines the maximum and minimum
 * number of entries (keys and pointers) in any
 * node.  Every node has at most order - 1 keys and
 * at least (roughly speaking) half that number.
 * Every leaf has as many pointers to data as keys,
 * and every internal node has one more pointer
 * to a subtree than the number of keys.
 * This global variable is initialized to the
 * default value.
 */
int order = DEFAULT_ORDER;
bool verbose = false;

/* The queue is used to print the tree in
 * level order, starting from the root
 * printing each entire rank on a separate
 * line, finishing with the leaves.
 */
queue_node * queue = NULL;

/* The user can toggle on and off the "verbose"
 * property, which causes the pointer addresses
 * to be printed out in hexadecimal notation
 * next to their corresponding keys.
 */

// FUNCTION DEFINITIONS.

// OUTPUT AND UTILITIES

/* Copyright and license notice to user at startup. 
 */
void license_notice( void ) {
    printf("bpt version %s -- Copyright (C) 2010  Amittai Aviram "
            "http://www.amittai.com\n", Version);
    printf("This program comes with ABSOLUTELY NO WARRANTY; for details "
            "type `show w'.\n"
            "This is free software, and you are welcome to redistribute it\n"
            "under certain conditions; type `show c' for details.\n\n");
}


/* Routine to print portion of GPL license to stdout.
 */
void print_license( int license_part ) {
    int start, end, line;
    FILE * fp;
    char buffer[0x100];

    switch(license_part) {
    case LICENSE_WARRANTEE:
        start = LICENSE_WARRANTEE_START;
        end = LICENSE_WARRANTEE_END;
        break;
    case LICENSE_CONDITIONS:
        start = LICENSE_CONDITIONS_START;
        end = LICENSE_CONDITIONS_END;
        break;
    default:
        return;
    }

    fp = fopen(LICENSE_FILE, "r");
    if (fp == NULL) {
        perror("print_license: fopen");
        exit(EXIT_FAILURE);
    }
    for (line = 0; line < start; line++)
        fgets(buffer, sizeof(buffer), fp);
    for ( ; line < end; line++) {
        fgets(buffer, sizeof(buffer), fp);
        printf("%s", buffer);
    }
    fclose(fp);
}


/* First message to the user.
 */
void usage_1( void ) {
    printf("B+ Tree of Order %d.\n", order);
    printf("Following Silberschatz, Korth, Sidarshan, Database Concepts, "
           "5th ed.\n\n"
           "To build a B+ tree of a different order, start again and enter "
           "the order\n"
           "as an integer argument:  bpt <order>  ");
    printf("(%d <= order <= %d).\n", MIN_ORDER, MAX_ORDER);
    printf("To start with input from a file of newline-delimited integers, \n"
           "start again and enter the order followed by the filename:\n"
           "bpt <order> <inputfile> .\n");
}


/* Second message to the user.
 */
void usage_2( void ) {
    printf("Enter any of the following commands after the prompt > :\n"
    "\ti <k>  -- Insert <k> (an integer) as both key and value).\n"
    "\tf <k>  -- Find the value under key <k>.\n"
    "\tp <k> -- Print the path from the root to key k and its associated "
           "value.\n"
    "\tr <k1> <k2> -- Print the keys and values found in the range "
            "[<k1>, <k2>\n"
    "\td <k>  -- Delete key <k> and its associated value.\n"
    "\tx -- Destroy the whole tree.  Start again with an empty tree of the "
           "same order.\n"
    "\tt -- Print the B+ tree.\n"
    "\tl -- Print the keys of the leaves (bottom row of the tree).\n"
    "\tv -- Toggle output of pointer addresses (\"verbose\") in tree and "
           "leaves.\n"
    "\tq -- Quit. (Or use Ctl-D.)\n"
    "\t? -- Print this help mesge.\n");
}


/* Brief usage note.
 */
void usage_3( void ) {
    printf("Usage: ./bpt [<order>]\n");
    printf("\twhere %d <= order <= %d .\n", MIN_ORDER, MAX_ORDER);
}


/* Helper function for printing the
 * tree out.  See print_tree.
 */
void enqueue( pagenum_t new_pagenum ) {
    if (queue == NULL) {
        queue = (queue_node*)calloc(1, sizeof(queue_node));
        queue->num = new_pagenum;
        queue->next = NULL;
    }
    else {
        queue_node * c = queue;
        queue_node * end_c = (queue_node*)calloc(1, sizeof(queue_node));

		while(c->next != NULL) {
            c = c->next;
        }
        end_c->num = new_pagenum;
        end_c->next = NULL;
        c->next = end_c;
    }
}


/* Helper function for printing the
 * tree out.  See print_tree.
 */
pagenum_t dequeue( void ){
    pagenum_t ret = queue->num;
    queue_node * n = queue;
    queue = queue->next;
    n->next = NULL;
    free(n);
    return ret;
}


/* Prints the bottom row of keys
 * of the tree (with their respective
 * pointers, if the verbose_output flag is set.
 */
void print_leaves( int table_id ) {
    int i;
    //page_t * c = (page_t*)calloc(1, PAGESIZE);
    pagenum_t c_pagenum = ROOTPAGENUM;
    //file_read_page(c_pagenum, c);
    buffer_t * c = buf_get_page(table_id, c_pagenum), * header_buf = buf_get_page(table_id, HEADERPAGENUM);
    if(header_buf->header_page.root_page_offset_num == 0){
        buf_put_page(header_buf);
        printf("Empty tree.\n");
        return;
    }
    buf_put_page(header_buf);
    while (!c->page.is_leaf){
        c_pagenum = c->page.pointer_num;
        buf_put_page(c);
        //file_read_page(c_pagenum, c);
        c = buf_get_page(table_id, c_pagenum);
    }
    while (true) {
        for (i = 0; i < c->page.num_keys; i++) {
            printf("%" PRId64 " ", c->page.records[i].key);
        }
        if(c->page.pointer_num != 0) {
            printf(" | ");
            c_pagenum = c->page.pointer_num;
            buf_put_page(c);
            //file_read_page(c_pagenum, c);
            c = buf_get_page(table_id, c_pagenum);
        }
        else
            break;
    }

    buf_put_page(c);
    printf("\n");
}



/* Utility function to give the length in edges
 * of the path from any node to the root.
 */
int path_to_root( int table_id, pagenum_t child_pagenum ) {
    int length = 0;
    //page_t * child = (page_t*)calloc(1, PAGESIZE);
    //file_read_page(child_pagenum, child);
    buffer_t * child = buf_get_page(table_id, child_pagenum);
	
	while(child->page.parent_page_offset_num != 0){
        child_pagenum = child->page.parent_page_offset_num;
        buf_put_page(child);
        child = buf_get_page(table_id, child_pagenum);
        //file_read_page(child_pagenum, child);
        length++;
    }

    buf_put_page(child);

    return length;
}

void print_free_page_list(int table_id){
    buffer_t * header_buf = buf_get_page(table_id, HEADERPAGENUM);
	if(header_buf->header_page.free_page_offset_num == 0){
        buf_put_page(header_buf);
		printf("\nEmpty Free Page List.\n");
		return;
	}
	
	printf("\nFree Page List\n");

	//page_t * fpl = (page_t*)calloc(1, PAGESIZE);
	pagenum_t fpl_pagenum = header_buf->header_page.free_page_offset_num;
	//file_read_page(fpl_pagenum, fpl);
    buffer_t * fpl = buf_get_page(table_id, fpl_pagenum);
	printf("[ " "%" PRId64, fpl_pagenum);
	while(fpl->page.pointer_num != 0){
		fpl_pagenum = fpl->page.pointer_num;
        buf_put_page(fpl);
		//file_read_page(fpl_pagenum, fpl);
        fpl = buf_get_page(table_id, fpl_pagenum);
		printf(" -> %"PRId64, fpl_pagenum);
	}
    buf_put_page(fpl);
    buf_put_page(header_buf);
	printf(" ]\n\n");
}


/* Prints the B+ tree in the command
 * line in level (rank) order, with the 
 * keys in each node and the '|' symbol
 * to separate nodes.
 * With the verbose_output flag set.
 * the values of the pointers corresponding
 * to the keys also appear next to their respective
 * keys, in hexadecimal notation.
 */
void print_tree( int table_id ) {
    //page_t * n = (page_t*)calloc(1, PAGESIZE);
    //print_buf();
    int i = 0;
    int rank = 0;
    int new_rank = 0;
    buffer_t * header_buf = buf_get_page(table_id, HEADERPAGENUM), * n;
    buf_put_page(header_buf);
    //print_buf();

    //if(root_page == NULL) {
    if(header_buf->header_page.root_page_offset_num == 0){
        printf("Empty tree.\n");
		print_free_page_list(table_id);
        return;
    }

    //print_buf();
	queue = NULL;
	printf("B+ Tree\n");
    enqueue(ROOTPAGENUM);
    while( queue != NULL ) {
        pagenum_t n_pagenum = dequeue();
        //file_read_page(n_pagenum, n);
        n = buf_get_page(table_id, n_pagenum);
        pagenum_t parent_pagenum = n->page.parent_page_offset_num;
        //page_t * parent = (page_t*)calloc(1, PAGESIZE);
        //file_read_page(parent_pagenum, parent);
        buffer_t * parent = buf_get_page(table_id, parent_pagenum);
        
		if(n->page.parent_page_offset_num != 0 && n_pagenum == parent->page.pointer_num) {
            new_rank = path_to_root(table_id, n_pagenum);
            if (new_rank != rank) {
                rank = new_rank;
                printf("\n");
            }
        }
        //free(parent);
        buf_put_page(parent);

		printf("(""%"PRId64", ""%"PRId64") ", n_pagenum, n->page.parent_page_offset_num);
        for (i = 0; i < n->page.num_keys; i++) {
            if(!n->page.is_leaf) printf("%" PRId64 " ", n->page.entries[i].key);
            else printf("%" PRId64 " ", n->page.records[i].key);
        }
        if (!n->page.is_leaf){
            enqueue(n->page.pointer_num);
            for (i = 0; i < n->page.num_keys; i++)
				enqueue(n->page.entries[i].child_page_offset_num);
        }
        buf_put_page(n);
        printf("| ");
    }
    printf("\n");
   // print_buf();
	print_free_page_list(table_id);
    //print_buf();
}


/* Finds the record under a given key and prints an
 * appropriate message to stdout.
 */
void find_and_print( int table_id, int64_t key, bool is_p ) {
    verbose = is_p;
    int64_t * r = find(table_id, key);
    pagenum_t pagenum;
    buffer_t * c = find_leaf(table_id, key, &pagenum);

    if (r == NULL)
        printf("Record not found under key ""%"PRId64".\n", key);
    else 
        printf("Record at %lx(pagenum) -- key ""%"PRId64", value %s.\n",
                (unsigned long)pagenum, key, r);

    verbose = false;
}



/* Traces the path from the root to a leaf, searching
 * by key.  Displays information about the path
 * if the verbose flag is set.
 * Returns the leaf containing the given key.
 */
buffer_t * find_leaf( int table_id, int64_t key, pagenum_t * index) {
    //printf("\nfind_leaf\n%d\n", table_id);
    int i = 0;

    //print_buf();
    buffer_t * header_buf = buf_get_page(table_id, HEADERPAGENUM);
    //printf("check1\n");
    //printf("%d\n", header_buf->header_page.root_page_offset_num);
    //print_buf();
    //printf("find leaf check1\n%d\n",header_buf->header_page.root_page_offset_num);
    if(header_buf->header_page.root_page_offset_num == 0){
        //printf("check4\n");
        if (verbose) 
            printf("Empty tree.\n");
        buf_put_page(header_buf);
        return NULL;
    }
    //printf("find leaf check2\n");
    //printf("check2\n");
    buf_put_page(header_buf);
    //printf("check3\n");

    //page_t* c = (page_t*)calloc(1, PAGESIZE);
    *index = ROOTPAGENUM;
	//file_read_page(*index, c);
    buffer_t * c = buf_get_page(table_id, *index);
    //printf("check4\n");
    while (!c->page.is_leaf) {
        if (verbose) {
            printf("[");
            for (i = 0; i < c->page.num_keys - 1; i++)
                printf("%" PRId64 " ", c->page.entries[i].key);
            printf("%"PRId64 "] ", c->page.entries[i].key);

        }
 
		i = 0;
        while (i < c->page.num_keys) {
            if(key >= c->page.entries[i].key) i++;
            else break;
        }
        if (verbose)
            printf("%d ->\n", i);

		if(i == 0) *index = c->page.pointer_num;
		else *index = c->page.entries[i - 1].child_page_offset_num;

        c->is_dirty = 1;
        buf_put_page(c);
        //file_read_page(*index, c);
        c = buf_get_page(table_id, *index);
    }
    //printf("check5\n");
    
	if (verbose) {
        printf("Leaf [");
        for (i = 0; i < c->page.num_keys - 1; i++)
            printf("%" PRId64 " ", c->page.records[i].key);
        printf("%" PRId64 "] ->\n", c->page.records[i].key);
    }
 
    buf_put_page(c);
	return c;
}


/* Finds and returns the record to which
 * a key refers.
 */
//char * find( int table_id, int64_t key ) {
int64_t * find( int table_id, int64_t key ) {
    //printf("find function");
    int i = 0;
    pagenum_t pagenum;
    buffer_t * c = find_leaf(table_id, key, &pagenum);

    if (c == NULL) return NULL;

    //printf("dfdfd\n");
    //print_buf();

    for (i = 0; i < c->page.num_keys; i++){
		//if (c->keys[i] == key) break;
		//printf("%" PRId64 " ",c->records[i].key);
        //printf("%d ",c->page.records[i].key);
        if(c->page.records[i].key == key) break;
	}
    //printf("\n");

    if (i == c->page.num_keys) {
        //printf("not found\n");
        //buf_put_page(c);
        return NULL;
    }
    else {
        //char * ret = (char*)malloc(sizeof(char)*120);
        int64_t * ret = (int64_t*)malloc(sizeof(int64_t)*15);
        //strcpy(ret, c->page.records[i].value);
        for(i = 0; i < 15; i++) ret[i] = c->page.records[i].values[i];
        //buf_put_page(c);
        return ret;
    }
}

/* Finds the appropriate place to
 * split a node that is too big into two.
 */
int cut( int length ) {
    if (length % 2 == 0)
        return length/2;
    else
        return length/2 + 1;
}

// INSERTION

/* Creates a new record to hold the value
 * to which a key refers.
 */
record_t * make_record( int table_id, int64_t key, int64_t * values ) {
    record_t * new_record = (record_t*)malloc(sizeof(record_t));
    
    if (new_record == NULL) {
        perror("Record creation.");
        exit(EXIT_FAILURE);
    }
    else {
        new_record->key = key;
        //strcpy(new_record->value, value);
        int num_col = get_num_col(table_id);
        for(int i = 0; i < num_col - 1; i++){
            new_record->values[i] = values[i];
        }
    }

    return new_record;
}


/* Creates a new general node, which can be adapted
 * to serve as either a leaf or an internal node.
 */
/*page_t * make_node( void ) {
    page_t * new_page = (page_t*)calloc(1, PAGESIZE);
	header_page->num_pages++;
	file_write_page(HEADERPAGENUM, (page_t*)header_page);

    if(new_page == NULL) {
        perror("Node creation.");
        exit(EXIT_FAILURE);
    }

    return new_page;
}*/

/* Creates a new leaf by creating a node
 * and then adapting it appropriately.
 */
/*page_t * make_leaf( void ) {
    page_t * leaf = make_node();
    leaf->is_leaf = true;

    return leaf;
}*/


/* Helper function used in insert_into_parent
 * to find the index of the parent's pointer to 
 * the node to the left of the key to be inserted.
 */
int get_left_index( buffer_t * parent, pagenum_t left_pagenum) {
    int left_index = 0;
    parent = buf_get_page(parent->table_id, parent->pagenum);
    buf_put_page(parent);

    if(parent->page.pointer_num == left_pagenum)
        return left_index;

    while (left_index <= parent->page.num_keys && 
        parent->page.entries[left_index].child_page_offset_num != left_pagenum)
        left_index++;

    return left_index + 1;
}

/* Inserts a new pointer to a record and its corresponding
 * key into a leaf.
 * Returns the altered leaf.
 */
void insert_into_leaf( buffer_t * leaf, record_t * record ) {
    //printf("insert into leaf\n");
    int i, insertion_point = 0;
    leaf = buf_get_page(leaf->table_id, leaf->pagenum);
    int num_col = get_num_col(leaf->table_id);
    //printf("%d\n", num_col);

    while(insertion_point < leaf->page.num_keys && leaf->page.records[insertion_point].key < record->key)
        insertion_point++;

    for (i = leaf->page.num_keys; i > insertion_point; i--) {
		leaf->page.records[i].key = leaf->page.records[i - 1].key;
        //strcpy(leaf->page.records[i].value, leaf->page.records[i - 1].value);
        for(int j = 0; j < num_col - 1; j++){
            leaf->page.records[i].values[j] = leaf->page.records[i - 1].values[j];
        }
    }
    leaf->page.records[insertion_point].key = record->key;
    //strcpy(leaf->page.records[insertion_point].value, record->value);
    for(i = 0; i < num_col - 1; i++){
        leaf->page.records[insertion_point].values[i] = record->values[i];
        //printf("%lld ", record->values[i]);
    }
    leaf->page.num_keys++;
    leaf->is_dirty = 1;

    buf_put_page(leaf);
}


/* Inserts a new key and pointer
 * to a new record into a leaf so as to exceed
 * the tree's order, causing the leaf to be split
 * in half.
 */
void insert_into_leaf_after_splitting( buffer_t * leaf, record_t * record, pagenum_t pagenum ) {
    //printf("insert leaf after\n");
    //print_buf();
    buffer_t * header_buf = buf_get_page(leaf->table_id, HEADERPAGENUM);
    buf_put_page(header_buf);
    buffer_t * new_leaf;
    pagenum_t new_leaf_pagenum = header_buf->header_page.num_pages;
    leaf = buf_get_page(leaf->table_id, leaf->pagenum);
    int num_col = get_num_col(leaf->table_id);
    //printf("dfdsf\n");
    //print_buf();

    /* Check the free_pagenum from header_page
     * get the first free page on free page list
    */
    if(header_buf->header_page.free_page_offset_num != 0){
        pagenum_t new_free_pagenum = buf_alloc_page(leaf->table_id);
        header_buf = buf_get_page(leaf->table_id, HEADERPAGENUM);

        new_leaf_pagenum = new_free_pagenum;
        //file_read_page(new_free_pagenum, new_leaf);
        new_leaf = buf_get_page(leaf->table_id, new_leaf_pagenum);

        header_buf->header_page.free_page_offset_num = new_leaf->page.pointer_num;

        new_leaf->page.pointer_num = 0;
		new_leaf->page.is_leaf = 1;

        header_buf->is_dirty = 1;
        new_leaf->is_dirty = 1;
        buf_put_page(header_buf);
        buf_put_page(new_leaf);
    }
    else{
        //printf("new leaf\n");
		new_leaf = make_leaf(leaf->table_id);
	}

    //printf("dfasewfwe\n");
    //print_buf();

    new_leaf = buf_get_page(new_leaf->table_id, new_leaf->pagenum);

    page_t * temp_page = (page_t*)calloc(1, PAGESIZE);
    int insertion_index = 0, split, i, j;
    int64_t new_key;

    while (insertion_index < order - 1 && leaf->page.records[insertion_index].key < record->key)
        insertion_index++;

    for (i = 0, j = 0; i < leaf->page.num_keys; i++, j++) {
        if (j == insertion_index) j++;
        temp_page->records[j].key = leaf->page.records[i].key;
        //strcpy(temp_page->records[j].value, leaf->page.records[i].value);
        for(int k = 0; k < num_col - 1; k++){
            temp_page->records[j].values[k] = leaf->page.records[i].values[k];
        }
    }
    temp_page->records[insertion_index].key = record->key;
    //strcpy(temp_page->records[insertion_index].value, record->value);
    for(i = 0; i < num_col - 1; i++){
        temp_page->records[insertion_index].values[i] = record->values[i];
    }
    leaf->page.num_keys = 0;

    split = cut(order - 1);

    for (i = 0; i < split; i++) {
        leaf->page.records[i].key = temp_page->records[i].key;
        //strcpy(leaf->page.records[i].value, temp_page->records[i].value);
        for(j = 0; j < num_col - 1; j++){
            leaf->page.records[i].values[j] = temp_page->records[i].values[j];
        }
        leaf->page.num_keys++;
    }

    for (i = split, j = 0; i < order; i++, j++) {
        new_leaf->page.records[j].key = temp_page->records[i].key;
        //strcpy(new_leaf->page.records[j].value, temp_page->records[i].value);
        for(int k = 0; k < num_col - 1; k++){
            new_leaf->page.records[j].values[k] = temp_page->records[i].values[k];
        }
        new_leaf->page.num_keys++;
    }

    free(temp_page);

    new_leaf->page.pointer_num = leaf->page.pointer_num;
    leaf->page.pointer_num = new_leaf_pagenum;
    //strcpy(new_leaf->page.records[order-1].value, leaf->page.records[order-1].value);
    for(i = 0; i < num_col - 1; i++){
        new_leaf->page.records[order-1].values[i] = leaf->page.records[order-1].values[i];
    }

    for (i = leaf->page.num_keys; i < order - 1; i++) {
        leaf->page.records[i].key = 0;
        memset(leaf->page.records[i].values, 0, sizeof(leaf->page.records[i].values));
    }
    for (i = new_leaf->page.num_keys; i < order - 1; i++) {
        new_leaf->page.records[i].key = 0;
        memset(new_leaf->page.records[i].values, 0, sizeof(new_leaf->page.records[i].values));
    }

    new_leaf->page.parent_page_offset_num = leaf->page.parent_page_offset_num;
    new_key = new_leaf->page.records[0].key;

    //file_write_page(pagenum, leaf);
    //file_write_page(new_leaf_pagenum, new_leaf);
    leaf->is_dirty = 1;
    new_leaf->is_dirty = 1;
    //printf("before leaf put\n");
    //print_buf();
    buf_put_page(leaf);
    //printf("before new leaf put\n");
    //print_buf();
    buf_put_page(new_leaf);

    //printf("123123123\n");
    //print_buf();
    insert_into_parent(leaf, &pagenum, new_key, new_leaf, &new_leaf_pagenum);

    return;
}


/* Inserts a new key and pointer to a node
 * into a node into which these can fit
 * without violating the B+ tree properties.
 */
void insert_into_node( buffer_t * n, pagenum_t n_pagenum, int left_index, int64_t key, pagenum_t right_pagenum ){
    int i;
    n = buf_get_page(n->table_id, n->pagenum);

    for (i = n->page.num_keys; i > left_index; i--) {
        n->page.entries[i].key = n->page.entries[i - 1].key;
        n->page.entries[i].child_page_offset_num = n->page.entries[i - 1].child_page_offset_num;
    }
    n->page.entries[left_index].key = key;
    n->page.entries[left_index].child_page_offset_num = right_pagenum;
    n->page.num_keys++;

    n->is_dirty = 1;
    buf_put_page(n);

    return;
}


/* Inserts a new key and pointer to a node
 * into a node, causing the node's size to exceed
 * the order, and causing the node to split into two.
 */
void insert_into_node_after_splitting( buffer_t * old_page, pagenum_t * old_page_pagenum, int left_index, int64_t key, buffer_t * right, pagenum_t * right_pagenum){
    int i, j, split;
    int64_t k_prime;
    buffer_t * new_page, * child, * header_buf = buf_get_page(old_page->table_id, HEADERPAGENUM);
    buf_put_page(header_buf);
	pagenum_t new_page_pagenum = header_buf->header_page.num_pages, child_page_pagenum;
    old_page = buf_get_page(old_page->table_id, old_page->pagenum);

    /* First create a temporary set of keys and pointers
     * to hold everything in order, including
     * the new key and pointer, inserted in their
     * correct places. 
     * Then create a new node and copy half of the 
     * keys and pointers to the old node and
     * the other half to the new.
     */

    /* Check the free_pagenum from header_page
     * get the first free page on free page list
    */
    if(header_buf->header_page.free_page_offset_num != 0){
        pagenum_t new_free_pagenum = buf_alloc_page(old_page->table_id);
        new_page_pagenum = new_free_pagenum;
        new_page = buf_get_page(old_page->table_id, new_page_pagenum);
        header_buf = buf_get_page(old_page->table_id, HEADERPAGENUM);

        header_buf->header_page.free_page_offset_num = new_page->page.pointer_num;
        new_page->page.pointer_num = 0;
		new_page->page.num_keys = 0;

        header_buf->is_dirty = 1;
        new_page->is_dirty = 1;
        buf_put_page(header_buf);
        buf_put_page(new_page);
    }
    else {
        new_page = make_node(old_page->table_id);
    }

    new_page = buf_get_page(new_page->table_id, new_page->pagenum);
    page_t * temp_page = (page_t*)calloc(1,PAGESIZE);

    temp_page->pointer_num = old_page->page.pointer_num;
    for(i = 0, j = 0; i < old_page->page.num_keys; i++, j++) {
        if(j == left_index) j++;
        temp_page->entries[j].key = old_page->page.entries[i].key;
        temp_page->entries[j].child_page_offset_num = old_page->page.entries[i].child_page_offset_num;
    }
    temp_page->entries[left_index].key = key;
    temp_page->entries[left_index].child_page_offset_num = *right_pagenum;

    /* Create the new node and copy
     * half the keys and pointers to the
     * old and half to the new.
     */  
    split = cut(order);

    old_page->page.num_keys = 0;
    for (i = 0; i < split - 1; i++) {
        old_page->page.entries[i].key = temp_page->entries[i].key;
        old_page->page.entries[i].child_page_offset_num = temp_page->entries[i].child_page_offset_num;
        old_page->page.num_keys++;
    }

    k_prime = temp_page->entries[split - 1].key;

    new_page->page.pointer_num = temp_page->entries[split - 1].child_page_offset_num;
    for (++i, j = 0; i < order; i++, j++) {
        new_page->page.entries[j].key = temp_page->entries[i].key;
        new_page->page.entries[j].child_page_offset_num = temp_page->entries[i].child_page_offset_num;
        new_page->page.num_keys++;
    }

    free(temp_page);

    new_page->page.parent_page_offset_num = old_page->page.parent_page_offset_num;
	
	//file_write_page(*old_page_pagenum, old_page);
    //file_write_page(new_page_pagenum, new_page);
    old_page->is_dirty = 1;
    new_page->is_dirty = 1;
    buf_put_page(old_page);
    buf_put_page(new_page);


    /* Insert a new key into the parent of the two
     * nodes resulting from the split, with
     * the old node to the left and the new to the right.
     */

    insert_into_parent(old_page, old_page_pagenum, k_prime, new_page, &new_page_pagenum);

    //child = (page_t*)calloc(1, PAGESIZE);

	//file_read_page(*old_page_pagenum, old_page);
	//file_read_page(new_page_pagenum, new_page);
    old_page = buf_get_page(old_page->table_id, old_page->pagenum);
    new_page = buf_get_page(new_page->table_id, new_page->pagenum);

	child_page_pagenum = old_page->page.pointer_num;
	//file_read_page(child_page_pagenum, child);
    child = buf_get_page(old_page->table_id, child_page_pagenum);
	child->page.parent_page_offset_num = *old_page_pagenum;
	//file_write_page(child_page_pagenum, child);
    //buf_put_page(old_page->table_id, child_page_pagenum);
    buf_put_page(child);

	for (i = 0; i< old_page->page.num_keys; i++){
		child_page_pagenum = old_page->page.entries[i].child_page_offset_num;
		//file_read_page(child_page_pagenum, child);
        child = buf_get_page(old_page->table_id, child_page_pagenum);
		child->page.parent_page_offset_num = *old_page_pagenum;
        child->is_dirty = 1;
		//buf_put_page(old_page->table_id, child_page_pagenum);
        buf_put_page(child);
	}

	child_page_pagenum = new_page->page.pointer_num;
	//file_read_page(child_page_pagenum, child);
    child = buf_get_page(old_page->table_id, child_page_pagenum);
	child->page.parent_page_offset_num = new_page_pagenum;
	//file_write_page(child_page_pagenum, child);
    //buf_put_page(old_page->table_id, child_page_pagenum);
    buf_put_page(child);
    
    for (i = 0; i < new_page->page.num_keys; i++) {
        child_page_pagenum = new_page->page.entries[i].child_page_offset_num;
        //file_read_page(child_page_pagenum, child);
        child = buf_get_page(old_page->table_id, child_page_pagenum);
        child->page.parent_page_offset_num = new_page_pagenum;
        child->is_dirty = 1;
        //buf_put_page(old_page->table_id, child_page_pagenum);
        buf_put_page(child);
    }

	//free(child);
    //free(new_page);
    buf_put_page(old_page);
    buf_put_page(new_page);

    return;
}



/* Inserts a new node (leaf or internal node) into the B+ tree.
 * Returns the root of the tree after insertion.
 */
void insert_into_parent(buffer_t * left, pagenum_t * left_pagenum, int64_t key, 
    buffer_t * right, pagenum_t * right_pagenum) {
    int left_index;

    /* Case: new root. */

    if(left->page.parent_page_offset_num == 0) {
        insert_into_new_root(left, left_pagenum, key, right, right_pagenum);
        return;
    }

    /* Case: leaf or node. (Remainder of
     * function body.)  
     */

    /* Find the parent's pointer to the left 
     * node.
     */

    //page_t * parent = (page_t*)calloc(1, PAGESIZE);
    pagenum_t parent_pagenum = left->page.parent_page_offset_num;
    buffer_t * parent = buf_get_page(left->table_id, parent_pagenum);
    buf_put_page(parent);

    left_index = get_left_index(parent, *left_pagenum);

    /* Simple case: the new key fits into the node. 
     */

    if (parent->page.num_keys < order - 1) {
        insert_into_node(parent, parent_pagenum, left_index, key, *right_pagenum);
        return;
    }

    /* Harder case:  split a node in order 
     * to preserve the B+ tree properties.
     */

    insert_into_node_after_splitting(parent, &parent_pagenum, left_index, key, right, right_pagenum);

    return;
}


/* Creates a new root for two subtrees
 * and inserts the appropriate key into
 * the new root.
 */
void insert_into_new_root(buffer_t * left, pagenum_t * left_pagenum, int64_t key, buffer_t * right, pagenum_t * right_pagenum) {
    left = buf_get_page(left->table_id, left->pagenum);
    right = buf_get_page(right->table_id, right->pagenum);

    buffer_t * new_root, * header_buf = buf_get_page(left->table_id, HEADERPAGENUM);
    buf_put_page(header_buf);
    pagenum_t new_root_pagenum = header_buf->header_page.num_pages;
	int i;

    if(header_buf->header_page.free_page_offset_num != 0){
        //new_root = (page_t*)calloc(1, PAGESIZE);
        header_buf = buf_get_page(left->table_id, HEADERPAGENUM);
        pagenum_t new_free_pagenum = buf_alloc_page(left->table_id);
        new_root_pagenum = new_free_pagenum;
        
        //file_read_page(new_free_pagenum, new_root);
        new_root = buf_get_page(left->table_id, new_root_pagenum);
        header_buf->header_page.free_page_offset_num = new_root->page.pointer_num;

        new_root->page.pointer_num = 0;
		new_root->page.num_keys = 0;
		new_root->page.is_leaf = 0;

        header_buf->is_dirty = 1;
        new_root->is_dirty = 1;
        buf_put_page(header_buf);
        buf_put_page(new_root);
    }
    else{
		new_root = make_node(left->table_id);
	}

    new_root = buf_get_page(new_root->table_id, new_root->pagenum);

    new_root->page.pointer_num = new_root_pagenum;
    new_root->page.entries[0].key = key;
    new_root->page.entries[0].child_page_offset_num = *right_pagenum;
    new_root->page.num_keys++;

	left->page.parent_page_offset_num = *left_pagenum;
	right->page.parent_page_offset_num = *left_pagenum;
	new_root->page.parent_page_offset_num = 0;

    //file_write_page(*left_pagenum, new_root);
    //file_write_page(new_root_pagenum, left);
    //file_write_page(*right_pagenum, right);

    new_root->pagenum = *left_pagenum;
    left->pagenum = new_root_pagenum;
    right->pagenum = *right_pagenum;

    buf_put_page(new_root);
    buf_put_page(left);
    buf_put_page(right);

    //file_read_page(*left_pagenum, root_page);
	*left_pagenum = new_root_pagenum;

    //free(new_root);
    return;
}



/* First insertion:
 * start a new tree.
 */
void start_new_tree( record_t * record, int table_id) {
    //printf("\nstart_new_tree\n");
    buffer_t * root_buf, * header_buf = buf_get_page(table_id, HEADERPAGENUM);
    buf_put_page(header_buf);
    if(header_buf->header_page.free_page_offset_num != 0){
		pagenum_t root_pagenum = buf_alloc_page(table_id);
        root_buf = buf_get_page(table_id, root_pagenum);
        header_buf = buf_get_page(table_id, HEADERPAGENUM);
		
        header_buf->header_page.free_page_offset_num = root_buf->page.pointer_num;
        root_buf->page.pointer_num = 0;
        root_buf->page.num_keys = 0;
        root_buf->page.is_leaf = 1;

        header_buf->is_dirty = 1;
        root_buf->is_dirty = 1;
        buf_put_page(header_buf);
        buf_put_page(root_buf);
	}
	else{
	    root_buf = make_leaf(table_id);
	}

    root_buf = buf_get_page(root_buf->table_id, root_buf->pagenum);

    root_buf->page.parent_page_offset_num = 0;
    root_buf->page.records[0].key = record->key;
    //strcpy(root_buf->page.records[0].value, record->value);
    int num_col = get_num_col(table_id);
    for(int i = 0; i < num_col - 1; i++){
        root_buf->page.records[0].values[i] = record->values[i];
    }
    root_buf->page.num_keys++;
    buf_put_page(root_buf);
    //printf("end start new tree\n");
}



/* Master insertion function.
 * Inserts a key and an associated value into
 * the B+ tree, causing the tree to be adjusted
 * however necessary to maintain the B+ tree
 * properties.
 */
int insert ( int table_id, int64_t key, int64_t * values ) {
    //printf("\ninsert fuction\n");
    record_t * record;
    pagenum_t pagenum;

    /* The current implementation ignores
     * duplicates.
     */
    //print_buf();
	int64_t * find_ret = find(table_id, key);
    //print_buf();

	if(find_ret != NULL)
        return 1;   //Already, tree has the record

    /* Create a new record for the
     * value.
     */
    //printf("not find!\n");
    record = make_record(table_id, key, values);

    /* Case: the tree does not exist yet.
     * Start a new tree.
     */

    buffer_t * header_buf = buf_get_page(table_id, HEADERPAGENUM);
    buf_put_page(header_buf);
    if(header_buf->header_page.root_page_offset_num == 0){
        start_new_tree(record, table_id);
        header_buf = buf_get_page(table_id, HEADERPAGENUM);
        header_buf->header_page.root_page_offset_num = ROOTPAGENUM;
        header_buf->is_dirty = 1;
        buf_put_page(header_buf);
        
        free(record);
      //  printf("dffd\n");
        return 0;
    }


    /* Case: the tree already exists.
     * (Rest of function body.)
     */

    buffer_t * leaf = find_leaf(table_id, key, &pagenum);

    /* Case: leaf has room for key and pointer.
     */

    if (leaf->page.num_keys < order - 1) {
		insert_into_leaf(leaf, record);
        free(record);
        return 0;
    }

    /* Case:  leaf must be split.
     */
    insert_into_leaf_after_splitting(leaf, record, pagenum);

    //free(leaf);
    free(record);
    return 0;
}




// DELETION.

/* Utility function for deletion.  Retrieves
 * the index of a node's nearest neighbor (sibling)
 * to the left if one exists.  If not (the node
 * is the leftmost child), returns -1 to signify
 * this special case.
 */
pagenum_t get_neighbor_index( buffer_t * n, pagenum_t n_pagenum, bool * is_most_left, int * parent_entries_index) {

    int i;
    //page_t * parent_page = (page_t*)calloc(1,PAGESIZE);

    //file_read_page(n->parent_page_offset_num, parent_page);
    buffer_t * parent_buf = buf_get_page(n->table_id, n->page.parent_page_offset_num);

    /* Return the index of the key to the left
     * of the pointer in the parent pointing
     * to n.  
     * If n is the leftmost child, this means
     * return -1.
     */

    if(parent_buf->page.pointer_num == n_pagenum){
        pagenum_t ret = parent_buf->page.entries[0].child_page_offset_num;

        buf_put_page(parent_buf);
        *is_most_left = true;
        *parent_entries_index = 0;

        return ret;
    }

    for(i = 0; i < parent_buf->page.num_keys; i++){
        if(parent_buf->page.entries[i].child_page_offset_num == n_pagenum){
            pagenum_t ret;
            if(i == 0)
                ret = parent_buf->page.pointer_num;
            else
                ret = parent_buf->page.entries[i-1].child_page_offset_num;

            buf_put_page(parent_buf);
            *is_most_left = false;
            *parent_entries_index = i;

            return ret;
        }
    }

    // Error state.
    printf("Search for nonexistent pointer to node in parent.\n");
    printf("Node:  %#lx\n", (unsigned long)n);
    exit(EXIT_FAILURE);
}


void remove_entry_from_node( buffer_t * n, pagenum_t n_pagenum, int64_t key ) {
    
    int i, num_col = get_num_col(n->table_id);
    n = buf_get_page(n->table_id, n->pagenum);

    // Remove the key and shift other keys accordingly.
    i = 0;
    if(n->page.is_leaf){
        while(n->page.records[i].key != key)
            i++;
        for(++i; i < n->page.num_keys; i++){
            n->page.records[i - 1].key = n->page.records[i].key;
            //strcpy(n->page.records[i - 1].value, n->page.records[i].value);
            for(int j = 0; j < num_col -1; j++){
                n->page.records[i - 1].values[j] = n->page.records[i].values[j];
            }
        }
    }
    else{
        while(n->page.entries[i].key != key)
            i++;
        for(++i; i < n->page.num_keys; i++){
            n->page.entries[i - 1].key = n->page.entries[i].key;
            n->page.entries[i - 1].child_page_offset_num = n->page.entries[i].child_page_offset_num;
        }
    }

    // Remove the pointer and shift other pointers accordingly.
    // First determine number of pointers.
    //num_pointers = n->is_leaf ? n->num_keys : n->num_keys + 1;

    // One key fewer.
    n->page.num_keys--;

    // Set the other pointers to NULL for tidiness.
    // A leaf uses the last pointer to point to the next leaf.
    if (n->page.is_leaf){
        for (i = n->page.num_keys; i < order; i++){
            n->page.records[i].key = 0;
            memset(n->page.records[i].values, 0, sizeof(n->page.records[i].values));
        }
    }
    else{
        for (i = n->page.num_keys; i < order; i++){
            n->page.entries[i].key = 0;
            n->page.entries[i].child_page_offset_num = 0;
        }
    }

    n->is_dirty = 1;
    buf_put_page(n);

    return;
}


void adjust_root(int table_id) {
    //printf("\nadjust root\n");

    /* Case: nonempty root.
     * Key and pointer have already been deleted,
     * so nothing to be done.
     */
    buffer_t * root_buf = buf_get_page(table_id, ROOTPAGENUM);

    if(root_buf->page.num_keys > 0)
        return;

    /* Case: empty root. 
     */

    //print_buf();
    if(!root_buf->page.is_leaf) {
        //printf("root not leaf\n");
        //page_t * new_root_page = (page_t*)calloc(1, PAGESIZE);
        pagenum_t new_root_page_pagenum = root_buf->page.pointer_num;

        //file_read_page(new_root_page_pagenum, new_root_page);
        buffer_t * new_root_buf = buf_get_page(table_id, new_root_page_pagenum);
        //printf("is leaf %d\n", new_root_buf->page.is_leaf);

		new_root_buf->page.parent_page_offset_num = 0;
        new_root_buf->pagenum = ROOTPAGENUM;
        root_buf->pagenum = new_root_page_pagenum;
        root_buf->is_dirty = 1;
        buf_put_page(root_buf);

		//file_write_page(ROOTPAGENUM, new_root_page);
        new_root_buf->is_dirty = 1;
        //print_buf();
        buf_put_page(new_root_buf);
		//file_read_page(ROOTPAGENUM, root_page);
        root_buf = buf_get_page(table_id, ROOTPAGENUM);

        //print_buf();

        //printf("root is leaf %d\n", root_buf->page.is_leaf);

        //file_free_page(new_root_page_pagenum);
        buf_free_page(table_id, new_root_page_pagenum);
        //free(new_root_page);

		if(!root_buf->page.is_leaf){
            //printf("adfsfa\n");
			//page_t * tmp = (page_t*)calloc(1, PAGESIZE);
			pagenum_t tmp_pagenum = root_buf->page.pointer_num;

			//file_read_page(tmp_pagenum, tmp);
            buffer_t * tmp = buf_get_page(table_id, tmp_pagenum);
			tmp->page.parent_page_offset_num = ROOTPAGENUM;
			//file_write_page(tmp_pagenum, tmp);
            tmp->is_dirty = 1;
            buf_put_page(tmp);

			int i;
			for(i = 0; i < root_buf->page.num_keys; i++){
				//tmp = (page_t*)calloc(1, PAGESIZE);
				tmp_pagenum = root_buf->page.entries[i].child_page_offset_num;
				//file_read_page(tmp_pagenum, tmp);
                tmp = buf_get_page(table_id, tmp_pagenum);
				tmp->page.parent_page_offset_num = ROOTPAGENUM;
				//file_write_page(tmp_pagenum, tmp);
                tmp->is_dirty = 1;
                buf_put_page(tmp);
			}
		}
        buf_put_page(root_buf);
    }

    // If it is a leaf (has no children),
    // then the whole tree is empty.

    else{
        //printf("root leaf\n");
		//file_free_page(ROOTPAGENUM);
        buf_put_page(root_buf);
        buf_free_page(table_id, ROOTPAGENUM);
        buffer_t * header_buf = buf_get_page(table_id, HEADERPAGENUM);
        header_buf->header_page.root_page_offset_num = 0;
        header_buf->is_dirty = 1;
        buf_put_page(header_buf);
	}

    return;
}


/* Coalesces a node that has become
 * too small after deletion
 * with a neighboring node that
 * can accept the additional entries
 * without exceeding the maximum.
 */
void coalesce_nodes( buffer_t * n, pagenum_t n_pagenum, buffer_t * neighbor, pagenum_t neighbor_pagenum, buffer_t * parent, pagenum_t parent_pagenum, bool is_most_left, int parent_entries_index) {
    //printf("merge node\n");
    int i, j, neighbor_insertion_index;
	buffer_t * tmp;
    pagenum_t tmp_pagenum;

    /* Starting point in the neighbor for copying
     * keys and pointers from n.
     * Recall that n and neighbor have swapped places
     * in the special case of n being a leftmost child.
     */

    /* Case:  nonleaf node.
     * Append k_prime and the following pointer.
     * Append all pointers and keys from the neighbor.
     */

    if(is_most_left) {
        tmp = n;
        n = neighbor;
        neighbor = tmp;
        tmp_pagenum = n_pagenum;
        n_pagenum = neighbor_pagenum;
        neighbor_pagenum = tmp_pagenum;
    }

    /* Starting point in the neighbor for copying
     * keys and pointers from n.
     * Recall that n and neighbor have swapped places
     * in the special case of n being a leftmost child.
     */

    n = buf_get_page(n->table_id, n->pagenum);
    neighbor = buf_get_page(neighbor->table_id, neighbor->pagenum);
    parent = buf_get_page(parent->table_id, parent->pagenum);
    neighbor_insertion_index = neighbor->page.num_keys;

    /* Case:  nonleaf node.
     * Append k_prime and the following pointer.
     * Append all pointers and keys from the neighbor.
     */

    if (!n->page.is_leaf) {

        /* Append k_prime.
         */
        neighbor->page.entries[neighbor_insertion_index].key = parent->page.entries[parent_entries_index].key;
        neighbor->page.entries[neighbor_insertion_index].child_page_offset_num = n->page.pointer_num;
        neighbor->page.num_keys++;

        for(i = neighbor_insertion_index + 1, j = 0; j < n->page.num_keys; i++, j++) {
            neighbor->page.entries[i].child_page_offset_num = n->page.entries[j].child_page_offset_num;
            neighbor->page.entries[i].key = n->page.entries[j].key;
            neighbor->page.num_keys++;
        }

        /* The number of pointers is always
         * one more than the number of keys.
         */

        /* All children must now point up to the same parent.
         */

		//page_t * temp = (page_t*)calloc(1, PAGESIZE);
        for (i = 0; i < neighbor->page.num_keys; i++) {
            tmp_pagenum = neighbor->page.entries[i].child_page_offset_num;
            //file_read_page(tmp_pagenum, temp);
            buffer_t * temp = buf_get_page(n->table_id, tmp_pagenum);
            temp->page.parent_page_offset_num = neighbor_pagenum;
            //file_write_page(tmp_pagenum, temp);
            buf_put_page(temp);
        }
    }

    /* In a leaf, append the keys and pointers of
     * n to the neighbor.
     * Set the neighbor's last pointer to point to
     * what had been n's right neighbor.
     */

    else {
        int num_col = get_num_col(neighbor->table_id);
        for (i = neighbor_insertion_index, j = 0; j < n->page.num_keys; i++, j++) {
            neighbor->page.records[i].key = n->page.records[j].key;
            //strcpy(neighbor->page.records[i].value, n->page.records[j].value);
            for(int k = 0; k < num_col - 1; k++){
                neighbor->page.records[i].values[k] = n->page.records[j].values[k];
            }
            neighbor->page.num_keys++;
        }
        neighbor->page.pointer_num = n->page.pointer_num;
    }

	if(is_most_left){
		parent->page.pointer_num = neighbor_pagenum;
	}

	//file_write_page(neighbor_pagenum, neighbor);
    n->is_dirty = 1;
    neighbor->is_dirty = 1;
    parent->is_dirty = 1;

    buf_put_page(n);
    buf_put_page(neighbor);
    buf_put_page(parent);

    delete_entry(parent, parent_pagenum, parent->page.entries[parent_entries_index].key);
    //file_free_page(n_pagenum);
    buf_free_page(n->table_id, n_pagenum);

    return;
}

/* Redistributes entries between two nodes when
 * one has become too small after deletion
 * but its neighbor is too big to append the
 * small node's entries without exceeding the
 * maximum
 */
void redistribute_nodes( buffer_t * n, pagenum_t n_pagenum, buffer_t * neighbor, pagenum_t neighbor_pagenum, buffer_t * parent, pagenum_t parent_pagenum, bool is_most_left, int entries_index) {

    int i, num_col = get_num_col(n->table_id);
    n = buf_get_page(n->table_id, n->pagenum);
    neighbor = buf_get_page(neighbor->table_id, neighbor->pagenum);
    parent = buf_get_page(parent->table_id, parent->pagenum);

    /* Case: n has a neighbor to the left. 
     * Pull the neighbor's last key-pointer pair over
     * from the neighbor's right end to n's left end.
     */

    if(!is_most_left) {
        if(!n->page.is_leaf){
            for(i = n->page.num_keys; i > 0; i--){
                n->page.entries[i].key = n->page.entries[i - 1].key;
                n->page.entries[i].child_page_offset_num = n->page.entries[i - 1].child_page_offset_num;
            }
            n->page.entries[0].child_page_offset_num = n->page.pointer_num;
            n->page.entries[0].key = parent->page.entries[entries_index].key;
            n->page.pointer_num = neighbor->page.entries[neighbor->page.num_keys - 1].child_page_offset_num;
            
            //page_t * temp_page = (page_t*)calloc(1, PAGESIZE);
            //file_read_page(n->pointer_num, temp_page);
            buffer_t * temp_buf = buf_get_page(n->table_id, n->page.pointer_num);
            temp_buf->page.parent_page_offset_num = n_pagenum;
            //file_write_page(n->pointer_num, temp_page);
            temp_buf->is_dirty = 1;
            buf_put_page(temp_buf);

            parent->page.entries[entries_index].key = neighbor->page.entries[neighbor->page.num_keys - 1].key;
            neighbor->page.entries[neighbor->page.num_keys - 1].key = 0;
            neighbor->page.entries[neighbor->page.num_keys - 1].child_page_offset_num = 0;
        }
        else{
            for(i = n->page.num_keys; i > 0; i--){
                n->page.records[i].key = n->page.entries[i - 1].key;
                //strcpy(n->page.records[i].value, n->page.records[i - 1].value);
                for(int j = 0; j < num_col - 1; j++){
                    n->page.records[i].values[j] = n->page.records[i - 1].values[j];
                }
            }
            n->page.records[0].key = neighbor->page.records[neighbor->page.num_keys - 1].key;
            //strcpy(n->page.records[0].value, neighbor->page.records[neighbor->page.num_keys - 1].value);
            for(i = 0; i < num_col - 1; i++){
                n->page.records[0].values[i] = neighbor->page.records[neighbor->page.num_keys - 1].values[i];
            }
            parent->page.entries[entries_index].key = n->page.records[0].key;
            neighbor->page.records[neighbor->page.num_keys - 1].key = 0;
            memset(neighbor->page.records[neighbor->page.num_keys - 1].values, 0, sizeof(neighbor->page.records[neighbor->page.num_keys - 1].values));
        }
    }

    /* Case: n is the leftmost child.
     * Take a key-pointer pair from the neighbor to the right.
     * Move the neighbor's leftmost key-pointer pair
     * to n's rightmost position.
     */

    else {  
        if(!n->page.is_leaf){
            n->page.entries[n->page.num_keys].key = parent->page.entries[entries_index].key;
            n->page.entries[n->page.num_keys].child_page_offset_num = neighbor->page.pointer_num;
            parent->page.entries[entries_index].key = neighbor->page.entries[0].key;

            //page_t * temp_page = (page_t*)calloc(1, PAGESIZE);
            //file_read_page(n->entries[n->num_keys].child_page_offset_num, temp_page);
            buffer_t * temp_buf = buf_get_page(n->table_id, n->page.entries[n->page.num_keys].child_page_offset_num);
            temp_buf->page.parent_page_offset_num = n_pagenum;
            //file_write_page(n->entries[n->num_keys].child_page_offset_num, temp_page);
            buf_put_page(temp_buf);

            neighbor->page.pointer_num = neighbor->page.entries[0].child_page_offset_num;
            for(i = 0; i < neighbor->page.num_keys - 1; i++){
                neighbor->page.entries[i].key = neighbor->page.entries[i + 1].key;
                neighbor->page.entries[i].child_page_offset_num = neighbor->page.entries[i + 1].child_page_offset_num;
            }
            neighbor->page.entries[neighbor->page.num_keys - 1].key = 0;
            neighbor->page.entries[neighbor->page.num_keys - 1].child_page_offset_num = 0;
        }
        else{
            n->page.records[n->page.num_keys].key = neighbor->page.records[0].key;
            //strcpy(n->page.records[n->page.num_keys].value, neighbor->page.records[0].value);
            for(i = 0; i < num_col - 1; i++){
                n->page.records[n->page.num_keys].values[i] = neighbor->page.records[0].values[i];
            }
            for(i = 0; i < neighbor->page.num_keys - 1; i++){
                neighbor->page.records[i].key = neighbor->page.records[i + 1].key;
                //strcpy(neighbor->page.records[i].value, neighbor->page.records[i + 1].value);
                for(int j = 0; j < num_col - 1; j++){
                    neighbor->page.records[i].values[j] = neighbor->page.records[i + 1].values[j];
                }
            }
            parent->page.entries[entries_index].key = neighbor->page.records[0].key;
            neighbor->page.records[neighbor->page.num_keys - 1].key = 0;
            memset(neighbor->page.records[neighbor->page.num_keys - 1].values, 0, sizeof(neighbor->page.records[neighbor->page.num_keys - 1].values));
        }       
    }

    /* n now has one more key and one more pointer;
     * the neighbor has one fewer of each.
     */

    n->page.num_keys++;
    neighbor->page.num_keys--;

    n->is_dirty = 1;
    neighbor->is_dirty = 1;
    parent->is_dirty = 1;

    buf_put_page(n);
    buf_put_page(neighbor);
    buf_put_page(parent);

    return;
}


/* Deletes an entry from the B+ tree.
 * Removes the record and its key and pointer
 * from the leaf, and then makes all appropriate
 * changes to preserve the B+ tree properties.
 */
void delete_entry( buffer_t * n, pagenum_t n_pagenum, int64_t key ) {
    //printf("\ndelete entry\n");
    int min_keys;
    buffer_t * neighbor,  * parent;
    pagenum_t neighbor_pagenum, parent_pagenum;
    int capacity;

    // Remove key and pointer from node.

    remove_entry_from_node(n, n_pagenum, key);

    /* Case:  deletion from the root. 
     */

	if(n->page.parent_page_offset_num == 0) {
		//root_page = n;
        adjust_root(n->table_id);
        return;
    }


    /* If the page has no keys (empty)
     * add to free page list
     * Delayed Merge
     */
    if(n->page.num_keys > 0) {
        return;
    }

    /* Case:  deletion from a node below the root.
     * (Rest of function body.)
     */

    /* Determine minimum allowable sizae of node,
     * to be preserved after deletion.
     */

    /* Case:  node stays at or above minimum.
     * (The simple case.)
     */

    /* Case:  node falls below minimum.
     * Either coalescence or redistribution
     * is needed.
     */

    /* Find the appropriate neighbor node with which
     * to coalesce.
     * Also find the key (k_prime) in the parent
     * between the pointer to node n and the pointer
     * to the neighbor.
     */

    bool is_most_left;
    int parent_entries_index;

    neighbor_pagenum = get_neighbor_index(n, n_pagenum, &is_most_left, &parent_entries_index);
    parent_pagenum = n->page.parent_page_offset_num;

    neighbor = buf_get_page(n->table_id, neighbor_pagenum);
    parent = buf_get_page(n->table_id, parent_pagenum);
    buf_put_page(neighbor);
    buf_put_page(parent);

    if (neighbor->page.num_keys + n->page.num_keys >= order - 1){
        redistribute_nodes(n, n_pagenum, neighbor, neighbor_pagenum, parent, parent_pagenum, is_most_left, parent_entries_index);
    }
    else{
        coalesce_nodes(n, n_pagenum, neighbor, neighbor_pagenum, parent, parent_pagenum, is_most_left, parent_entries_index);
    }

    return;
}



/* Master deletion function.
 */
//int delete(int table_id, int64_t key) {
int erase(int table_id, int64_t key){
    //printf("\ndelete\n");
    buffer_t * key_leaf_page;
    int64_t * key_record_value;
    pagenum_t key_leaf_page_pagenum;

    key_record_value = find(table_id, key);
    key_leaf_page = find_leaf(table_id, key, &key_leaf_page_pagenum);

    //printf("%s", key_record_value);
    if (key_record_value != NULL && key_leaf_page != NULL) {
        delete_entry(key_leaf_page, key_leaf_page_pagenum, key);
        return 0;
    }

    return -1;
}
