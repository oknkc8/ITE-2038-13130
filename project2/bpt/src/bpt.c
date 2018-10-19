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
//bool verbose_output = false;


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
    "\t? -- Print this help message.\n");
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
//void enqueue( node * new_node ) {
void enqueue( pagenum_t new_pagenum ) {
    //node * c;
    if (queue == NULL) {
        //queue = new_node;
		//printf("queue empty!\n");
        queue = (queue_node*)calloc(1, sizeof(queue_node));
        queue->num = new_pagenum;
        queue->next = NULL;
    }
    else {
        //c = queue;
        queue_node * c = queue;
        queue_node * end_c = (queue_node*)calloc(1, sizeof(queue_node));
        while(c->next != NULL) {
            c = c->next;
        }
        //c->next = new_node;
        end_c->num = new_pagenum;
        end_c->next = NULL;
        //new_node->next = NULL;
        c->next = end_c;
    }
}


/* Helper function for printing the
 * tree out.  See print_tree.
 */
//node * dequeue( void ) {
pagenum_t dequeue( void ){
    pagenum_t ret = queue->num;
	//printf("queue now = %lld\n",ret);
    queue_node * n = queue;
    queue = queue->next;
//	if(queue!=NULL)
//		printf("queue next = %lld\n",queue->num);
    n->next = NULL;
    free(n);
    //node * n = queue;
    //queue = queue->next;
    //n->next = NULL;
    //return n;
    return ret;
}


/* Prints the bottom row of keys
 * of the tree (with their respective
 * pointers, if the verbose_output flag is set.
 */
//void print_leaves( node * root ) {
void print_leaves( void ) {
    int i;
    //node * c = root;
    page_t * c = (page_t*)calloc(1, PAGESIZE);
    pagenum_t c_pagenum = ROOTPAGENUM;
    file_read_page(c_pagenum, c);
    if (root_page == NULL) {
        printf("Empty tree.\n");
        return;
    }
    while (!c->is_leaf){
        //c = c->pointers[0];
        c_pagenum = c->pointer_num;
        file_read_page(c_pagenum, c);
    }
    while (true) {
        for (i = 0; i < c->num_keys; i++) {
            //if (verbose_output)
            //    printf("%lx ", (unsigned long)c->pointers[i]);
            //printf("%d ", c->keys[i]);
            printf("%lld ", c->records[i].key);
        }
        //if (verbose_output)
        //    printf("%lx ", (unsigned long)c->pointers[order - 1]);
        //if (c->pointers[order - 1] != NULL) {
        if(c->pointer_num != 0) {
            printf(" | ");
            //c = c->pointers[order - 1];
            c_pagenum = c->pointer_num;
            file_read_page(c_pagenum, c);
        }
        else
            break;
    }
    free(c);
    printf("\n");
}


/* Utility function to give the height
 * of the tree, which length in number of edges
 * of the path from the root to any leaf.
 */
/*
int height( node * root ) {
    int h = 0;
    node * c = root;
    while (!c->is_leaf) {
        c = c->pointers[0];
        h++;
    }
    return h;
}
*/

/* Utility function to give the length in edges
 * of the path from any node to the root.
 */
//int path_to_root( node * root, node * child ) {
int path_to_root( pagenum_t child_pagenum ) {
    int length = 0;
    //node * c = child;
    page_t * child = (page_t*)calloc(1, PAGESIZE);
    file_read_page(child_pagenum, child);
    //while (c != root) {
//    while(child != root_page) {
	while(child->parent_page_offset_num != 0){
		//c = c->parent;
        child_pagenum = child->parent_page_offset_num;
        file_read_page(child_pagenum, child);
        length++;
    }
    free(child);
    return length;
}

void print_free_page_list(){
	if(header_page->free_page_offset_num == 0){
		printf("\nEmpty Free Page List.\n");
		return;
	}
	
	printf("\nFree Page List\n");

	page_t * fpl = (page_t*)calloc(1, PAGESIZE);
	pagenum_t fpl_pagenum = header_page->free_page_offset_num;
	file_read_page(fpl_pagenum, fpl);
	printf("[ %lld", fpl_pagenum);
	while(fpl->pointer_num != 0){
		fpl_pagenum = fpl->pointer_num;
		file_read_page(fpl_pagenum, fpl);
		printf(" -> %lld", fpl_pagenum);
	}
	printf(" ]\n");
	return;
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
//void print_tree( node * root ) {
void print_tree( void ) {

    //node * n = NULL;
    page_t * n = (page_t*)calloc(1, PAGESIZE);
    int i = 0;
    int rank = 0;
    int new_rank = 0;

    //if (root == NULL) {
    if(root_page == NULL) {
        printf("Empty tree.\n");
		print_free_page_list();
        return;
    }
    queue = NULL;
	printf("B+ Tree\n");
    //enqueue(root);
    enqueue(ROOTPAGENUM);
    while( queue != NULL ) {
        pagenum_t n_pagenum = dequeue();
        file_read_page(n_pagenum, n);
        pagenum_t parent_pagenum = n->parent_page_offset_num;
        page_t * parent = (page_t*)calloc(1, PAGESIZE);
        file_read_page(parent_pagenum, parent);
        //if (n->parent != NULL && n == n->parent->pointers[0]) {
        if(n->parent_page_offset_num != 0 && n_pagenum == parent->pointer_num) {
            //new_rank = path_to_root( root, n );
            new_rank = path_to_root(n_pagenum);
			//printf("new_rank = %d\n", new_rank);
            if (new_rank != rank) {
                rank = new_rank;
                printf("\n");
            }
        }
        free(parent);
        //if (verbose_output) 
        //    printf("(%lx)", (unsigned long)n);
		printf("(%lld, %lld) ", n_pagenum, n->parent_page_offset_num);
        for (i = 0; i < n->num_keys; i++) {
            //if (verbose_output)
            //    printf("%lx ", (unsigned long)n->pointers[i]);
            //printf("%d ", n->keys[i]);
            if(!n->is_leaf) printf("%lld ", n->entries[i].key);
            else printf("%lld ", n->records[i].key);
        }
        if (!n->is_leaf){
            enqueue(n->pointer_num);
            for (i = 0; i < n->num_keys; i++)
                //enqueue(n->pointers[i]);
				enqueue(n->entries[i].child_page_offset_num);
        }
        /*if (verbose_output) {
            if (n->is_leaf) 
                printf("%lx ", (unsigned long)n->pointers[order - 1]);
            else
                printf("%lx ", (unsigned long)n->pointers[n->num_keys]);
        }
        */
        printf("| ");
    }
    printf("\n");
	print_free_page_list();
    free(n);
}


/* Finds the record under a given key and prints an
 * appropriate message to stdout.
 */
//void find_and_print(node * root, int key, bool verbose) {
void find_and_print( int64_t key, bool is_p ) {
    verbose = is_p;
    //record * r = find(root, key, verbose);
    char * r = find(key);
    pagenum_t pagenum;
    page_t * c = find_leaf(key, &pagenum);
    if (r == NULL)
        printf("Record not found under key %lld.\n", key);
    else 
        printf("Record at %lx(pagenum) -- key %lld, value %s.\n",
                (unsigned long)pagenum, key, r);
    verbose = false;
}



/* Traces the path from the root to a leaf, searching
 * by key.  Displays information about the path
 * if the verbose flag is set.
 * Returns the leaf containing the given key.
 */
//node * find_leaf( node * root, int key, bool verbose ) {
page_t * find_leaf( int64_t key, pagenum_t * index) {
    int i = 0;

	//if(root_page!=NULL && !root_page->is_leaf) printf("%lld\n" ,root_page->entries[0].key);

    if (root_page == NULL) {
        if (verbose) 
            printf("Empty tree.\n");
        return root_page;
    }

    //node * c = root;
    page_t* c = (page_t*)calloc(1, PAGESIZE);
    *index = ROOTPAGENUM;
	file_read_page(*index, c);

//	printf("c is leaf = %d\n",c->is_leaf);
    while (!c->is_leaf) {
//		printf("c first key = %lld\n", c->entries[0].key);
        if (verbose) {
            printf("[");
            for (i = 0; i < c->num_keys - 1; i++)
                //printf("%d ", c->keys[i]);
                printf("%lld ", c->entries[i].key);
            //printf("%d] ", c->keys[i]);
            printf("%lld] ", c->entries[i].key);

        }
        i = 0;
//		printf("c keys\n");
        while (i < c->num_keys) {
            //if (key >= c->keys[i]) i++;
//			printf("%lld ", c->entries[i].key);
            if(key >= c->entries[i].key) i++;
            else break;
        }
//		printf("\n");
        if (verbose)
            printf("%d ->\n", i);
        //c = (node *)c->pointers[i];
//		printf("c pagenum = %lld\n", *index);
		if(i == 0) *index = c->pointer_num;
		else *index = c->entries[i - 1].child_page_offset_num;

        //*index = c->entries[i].child_page_offset_num;
//		printf("c child pagenum = %lld\n", *index);
        file_read_page(*index, c);
    }
    if (verbose) {
        printf("Leaf [");
        for (i = 0; i < c->num_keys - 1; i++)
            //printf("%d ", c->keys[i]);
            printf("%lld ", c->records[i].key);
        //printf("%d] ->\n", c->keys[i]);
        printf("%lld] ->\n", c->records[i].key);
    }
//	printf("find leaf pagenum = %lld, key = %lld\n", *index, key);
    return c;
}


/* Finds and returns the record to which
 * a key refers.
 */
//record * find( node * root, int key, bool verbose ) {
char * find( int64_t key ) {
    int i = 0;
    pagenum_t pagenum;
    //node * c = find_leaf( root, key, verbose );
    page_t* c = find_leaf(key, &pagenum);

    if (c == NULL) return NULL;
//	printf("key is in the leaf!\n");
//	printf("c key num = %d\n", c->num_keys);
    for (i = 0; i < c->num_keys; i++){
		//if (c->keys[i] == key) break;
		//printf("%lld ",c->records[i].key);
        if(c->records[i].key == key) break;
	}
//	printf("chekc!\n");
    if (i == c->num_keys) {
		//printf("key is not in tree!\n");
        free(c);
        return NULL;
    }
    else {
        char * ret = (char*)malloc(sizeof(char)*120);
        strcpy(ret, c->records[i].value);
        free(c);
        //return (record *)c->pointers[i];
//		printf("find the node pagenum = %lld key= %lld value=%s\n", pagenum, key, ret);
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
//record * make_record(int value) {
record_t * make_record( int64_t key, char * value ) {
    //record * new_record = (record *)malloc(sizeof(record));
    record_t * new_record = (record_t*)malloc(sizeof(record_t));
    if (new_record == NULL) {
        perror("Record creation.");
        exit(EXIT_FAILURE);
    }
    else {
        //new_record->value = value;
        new_record->key = key;
        //new_record->value = value;
        strcpy(new_record->value, value);
    }
    return new_record;
}


/* Creates a new general node, which can be adapted
 * to serve as either a leaf or an internal node.
 */
//node * make_node( void ) {
page_t * make_node( void ) {
    //node * new_node;
    page_t * new_page = (page_t*)calloc(1, PAGESIZE);
	header_page->num_pages++;
	//printf("total page number = %d\n",header_page->num_pages);
	file_write_page(HEADERPAGENUM, header_page);
    //new_node = malloc(sizeof(node));
    //if (new_node == NULL) {
    if(new_page == NULL) {
        perror("Node creation.");
        exit(EXIT_FAILURE);
    }
    /*
    new_node->keys = malloc( (order - 1) * sizeof(int) );
    if (new_node->keys == NULL) {
        perror("New node keys array.");
        exit(EXIT_FAILURE);
    }
    new_node->pointers = malloc( order * sizeof(void *) );
    if (new_node->pointers == NULL) {
        perror("New node pointers array.");
        exit(EXIT_FAILURE);
    }
    new_node->is_leaf = false;
    new_node->num_keys = 0;
    new_node->parent = NULL;
    new_node->next = NULL;
    */
    return new_page;
}

/* Creates a new leaf by creating a node
 * and then adapting it appropriately.
 */
//node * make_leaf( void ) {
page_t * make_leaf( void ) {
    //node * leaf = make_node();
    page_t * leaf = make_node();
    leaf->is_leaf = true;
    return leaf;
}


/* Helper function used in insert_into_parent
 * to find the index of the parent's pointer to 
 * the node to the left of the key to be inserted.
 */
//int get_left_index(node * parent, node * left) {
int get_left_index( page_t * parent, pagenum_t left_pagenum) {
    int left_index = 0;
    //while (left_index <= parent->num_keys && 
            //parent->pointers[left_index] != left)
    if(parent->pointer_num == left_pagenum)
        return left_index;

    while (left_index <= parent->num_keys && 
        parent->entries[left_index].child_page_offset_num != left_pagenum)
        left_index++;
    return left_index + 1;
}

/* Inserts a new pointer to a record and its corresponding
 * key into a leaf.
 * Returns the altered leaf.
 */
//node * insert_into_leaf( node * leaf, int key, record * pointer ) {
void insert_into_leaf( page_t * leaf, record_t * record ) {
//	printf("\ninsert_into_leaf function!\n");
    int i, insertion_point;

    insertion_point = 0;
    //while (insertion_point < leaf->num_keys && leaf->keys[insertion_point] < key)
    while(insertion_point < leaf->num_keys && leaf->records[insertion_point].key < record->key)
        insertion_point++;

//	printf("insertion point = %d\n", insertion_point);
//	printf("leaf->num_keys = %d\n", leaf->num_keys);
//	printf("record key = %lld record = %s\n",record->key, record->value);

    for (i = leaf->num_keys; i > insertion_point; i--) {
        //leaf->keys[i] = leaf->keys[i - 1];
        //leaf->records[i] = leaf->records[i - 1];
        //leaf->pointers[i] = leaf->pointers[i - 1];
		leaf->records[i].key = leaf->records[i - 1].key;
        strcpy(leaf->records[i].value, leaf->records[i - 1].value);
    }
    //leaf->keys[insertion_point] = key;
    //leaf->pointers[insertion_point] = pointer;
    //leaf->records[insertion_point] = record;
    leaf->records[insertion_point].key = record->key;
    strcpy(leaf->records[insertion_point].value, record->value);
    leaf->num_keys++;
//	printf("insertion into leaf successful!!\n");
    //return leaf;
}


/* Inserts a new key and pointer
 * to a new record into a leaf so as to exceed
 * the tree's order, causing the leaf to be split
 * in half.
 */
//node * insert_into_leaf_after_splitting(node * root, node * leaf, int key, record * pointer) {
void insert_into_leaf_after_splitting( page_t * leaf, record_t * record, pagenum_t pagenum ) {
    //node * new_leaf;
    page_t * new_leaf;
    pagenum_t new_leaf_pagenum = header_page->num_pages;
//	printf("insert_into_leaf_after_splitting function\n");

    /* Check the free_page_num from header_page
     * get the first free page on free page list
    */
    if(header_page->free_page_offset_num != 0){
//		printf("free page list exist!\n");
        new_leaf = (page_t*)calloc(1, PAGESIZE);
        //pagenum_t new_free_page_num = header_page->free_page_offset_num;
        pagenum_t new_free_page_num = file_alloc_page();
        new_leaf_pagenum = new_free_page_num;
        file_read_page(new_free_page_num, new_leaf);
        header_page->free_page_offset_num = new_leaf->pointer_num;
        new_leaf->pointer_num = 0;
    }
    else{
//		printf("free page list empty!\n");
		new_leaf = make_leaf();
	}

    //int * temp_keys;
    //void ** temp_pointers;
    page_t * temp_page = (page_t*)calloc(1, PAGESIZE);
    int insertion_index, split, i, j;
    int64_t new_key;

    //new_leaf = make_leaf();

/*
    temp_keys = malloc( order * sizeof(int) );
    if (temp_keys == NULL) {
        perror("Temporary keys array.");
        exit(EXIT_FAILURE);
    }

    temp_pointers = malloc( order * sizeof(void *) );
    if (temp_pointers == NULL) {
        perror("Temporary pointers array.");
        exit(EXIT_FAILURE);
    }
*/

    insertion_index = 0;
    //while (insertion_index < order - 1 && leaf->keys[insertion_index] < key)
    while (insertion_index < order - 1 && leaf->records[insertion_index].key < record->key)
        insertion_index++;

    for (i = 0, j = 0; i < leaf->num_keys; i++, j++) {
        if (j == insertion_index) j++;
        //temp_keys[j] = leaf->keys[i];
        //temp_pointers[j] = leaf->pointers[i];
        temp_page->records[j].key = leaf->records[i].key;
        strcpy(temp_page->records[j].value, leaf->records[i].value);
    }

    //temp_keys[insertion_index] = key;
    //temp_pointers[insertion_index] = pointer;
    temp_page->records[insertion_index].key = record->key;
    strcpy(temp_page->records[insertion_index].value, record->value);

    leaf->num_keys = 0;

    split = cut(order - 1);

//	printf("\ntemp page key & value\n");
//	printf("* copy into leaf page\n");
    for (i = 0; i < split; i++) {
        //leaf->pointers[i] = temp_pointers[i];
        //leaf->keys[i] = temp_keys[i];
        leaf->records[i].key = temp_page->records[i].key;
        strcpy(leaf->records[i].value, temp_page->records[i].value);
//		printf("key = %lld value = %s\n",temp_page->records[i].key, temp_page->records[i].value);
        leaf->num_keys++;
    }

//	printf("* copy into new_leaf page\n");
    for (i = split, j = 0; i < order; i++, j++) {
        //new_leaf->pointers[j] = temp_pointers[i];
        //new_leaf->keys[j] = temp_keys[i];
        //new_leaf->num_keys++;
        new_leaf->records[j].key = temp_page->records[i].key;
        strcpy(new_leaf->records[j].value, temp_page->records[i].value);
//		printf("key = %lld value = %s\n",temp_page->records[i].key, temp_page->records[i].value);
        new_leaf->num_keys++;
    }

    //free(temp_pointers);
    //free(temp_keys);
    free(temp_page);

    //new_leaf->pointers[order - 1] = leaf->pointers[order - 1];
    //leaf->pointers[order - 1] = new_leaf;
    new_leaf->pointer_num = leaf->pointer_num;
    leaf->pointer_num = new_leaf_pagenum;
    strcpy(new_leaf->records[order-1].value, leaf->records[order-1].value);

    for (i = leaf->num_keys; i < order - 1; i++) {
        //leaf->pointers[i] = NULL;
        leaf->records[i].key = 0;
        memset(leaf->records[i].value, 0, sizeof(leaf->records[i].value));
    }
    for (i = new_leaf->num_keys; i < order - 1; i++) {
        //new_leaf->pointers[i] = NULL;
        new_leaf->records[i].key = 0;
        memset(new_leaf->records[i].value, 0, sizeof(new_leaf->records[i].value));
    }

    //new_leaf->parent = leaf->parent;
    new_leaf->parent_page_offset_num = leaf->parent_page_offset_num;
    //new_key = new_leaf->keys[0];
    new_key = new_leaf->records[0].key;

    file_write_page(pagenum, leaf);
    file_write_page(new_leaf_pagenum, new_leaf);

    //return insert_into_parent(root, leaf, new_key, new_leaf);
    insert_into_parent(leaf, &pagenum, new_key, new_leaf, &new_leaf_pagenum);
    free(new_leaf);
    return;
}


/* Inserts a new key and pointer to a node
 * into a node into which these can fit
 * without violating the B+ tree properties.
 */
//node * insert_into_node(node * root, node * n, 
//        int left_index, int key, node * right) {
void insert_into_node( page_t * n, pagenum_t n_pagenum, int left_index, 
    int64_t key, pagenum_t right_pagenum ){
//	printf("\ninsert_into_node function\n");
    int i;

//	printf("right pn %d\nkey %lld\nn pagenum %lld\n",right_pagenum, key, n_pagenum);
    for (i = n->num_keys; i > left_index; i--) {
        //n->pointers[i + 1] = n->pointers[i];
        //n->keys[i] = n->keys[i - 1];
        n->entries[i].key = n->entries[i - 1].key;
        n->entries[i].child_page_offset_num = n->entries[i - 1].child_page_offset_num;
    }
    //n->pointers[left_index + 1] = right;
    //n->keys[left_index] = key;
    n->entries[left_index].key = key;
    n->entries[left_index].child_page_offset_num = right_pagenum;
    n->num_keys++;

    file_write_page(n_pagenum, n);

    //return root;
    return;
}


/* Inserts a new key and pointer to a node
 * into a node, causing the node's size to exceed
 * the order, and causing the node to split into two.
 */
//node * insert_into_node_after_splitting(node * root, node * old_node, int left_index, 
//        int key, node * right) {
void insert_into_node_after_splitting( page_t * old_page, pagenum_t * old_page_pagenum, int left_index, 
    int64_t key, page_t * right, pagenum_t * right_pagenum){

//	printf("insert_into_node_after_splitting function\n");
    int i, j, split;
    int64_t k_prime;
    //node * new_node, * child;
    //int * temp_keys;
    //node ** temp_pointers;
    page_t * new_page, * child;
	pagenum_t new_page_pagenum = header_page->num_pages, child_page_pagenum;

    /* First create a temporary set of keys and pointers
     * to hold everything in order, including
     * the new key and pointer, inserted in their
     * correct places. 
     * Then create a new node and copy half of the 
     * keys and pointers to the old node and
     * the other half to the new.
     */

    /* Check the free_page_num from header_page
     * get the first free page on free page list
    */
    if(header_page->free_page_offset_num != 0){
        new_page = (page_t*)calloc(1, PAGESIZE);
        //pagenum_t new_free_page_num = header_page->free_page_offset_num;
        pagenum_t new_free_page_num = file_alloc_page();
        new_page_pagenum = new_free_page_num;
        file_read_page(new_free_page_num, new_page);
        header_page->free_page_offset_num = new_page->pointer_num;
        new_page->pointer_num = 0;
    }
    else new_page = make_node();

/*
    temp_pointers = malloc( (order + 1) * sizeof(node *) );
    if (temp_pointers == NULL) {
        perror("Temporary pointers array for splitting nodes.");
        exit(EXIT_FAILURE);
    }
    temp_keys = malloc( order * sizeof(int) );
    if (temp_keys == NULL) {
        perror("Temporary keys array for splitting nodes.");
        exit(EXIT_FAILURE);
    }
*/
    page_t * temp_page = (page_t*)calloc(1,PAGESIZE);

/*
    for (i = 0, j = 0; i < old_node->num_keys + 1; i++, j++) {
        if (j == left_index + 1) j++;
        temp_pointers[j] = old_node->pointers[i];
    }

    for (i = 0, j = 0; i < old_node->num_keys; i++, j++) {
        if (j == left_index) j++;
        temp_keys[j] = old_node->keys[i];
    }
*/
    temp_page->pointer_num = old_page->pointer_num;
    for(i = 0, j = 0; i < old_page->num_keys; i++, j++) {
        if(j == left_index) j++;
        temp_page->entries[j].key = old_page->entries[i].key;
        temp_page->entries[j].child_page_offset_num = old_page->entries[i].child_page_offset_num;
    }

    //temp_pointers[left_index + 1] = right;
    //temp_keys[left_index] = key;
    temp_page->entries[left_index].key = key;
    temp_page->entries[left_index].child_page_offset_num = *right_pagenum;

    /* Create the new node and copy
     * half the keys and pointers to the
     * old and half to the new.
     */  
    split = cut(order);
    //new_node = make_node();
    //old_node->num_keys = 0;
    old_page->num_keys = 0;
    for (i = 0; i < split - 1; i++) {
        //old_node->pointers[i] = temp_pointers[i];
        //old_node->keys[i] = temp_keys[i];
        old_page->entries[i].key = temp_page->entries[i].key;
        old_page->entries[i].child_page_offset_num = temp_page->entries[i].child_page_offset_num;
        //old_node->num_keys++;
        old_page->num_keys++;
    }
    //old_node->pointers[i] = temp_pointers[i];
    //k_prime = temp_keys[split - 1];
    k_prime = temp_page->entries[split - 1].key;
//	printf("k_prime %lld\n", k_prime);
    new_page->pointer_num = temp_page->entries[split - 1].child_page_offset_num;
    for (++i, j = 0; i < order; i++, j++) {
        //new_node->pointers[j] = temp_pointers[i];
        //new_node->keys[j] = temp_keys[i];
        new_page->entries[j].key = temp_page->entries[i].key;
        new_page->entries[j].child_page_offset_num = temp_page->entries[i].child_page_offset_num;
        //new_node->num_keys++;
        new_page->num_keys++;
    }
    //new_node->pointers[j] = temp_pointers[i];
    //free(temp_pointers);
    //free(temp_keys);
    free(temp_page);
    //new_node->parent = old_node->parent;
    new_page->parent_page_offset_num = old_page->parent_page_offset_num;
	
	file_write_page(*old_page_pagenum, old_page);
    file_write_page(new_page_pagenum, new_page);

    /* Insert a new key into the parent of the two
     * nodes resulting from the split, with
     * the old node to the left and the new to the right.
     */

    //return insert_into_parent(root, old_node, k_prime, new_node);
    insert_into_parent(old_page, old_page_pagenum, k_prime, new_page, &new_page_pagenum);

    child = (page_t*)calloc(1, PAGESIZE);

	file_read_page(*old_page_pagenum, old_page);
	file_read_page(new_page_pagenum, new_page);

//	printf("old_page pagenum = %lld\n", *old_page_pagenum);

	child_page_pagenum = old_page->pointer_num;
	file_read_page(child_page_pagenum, child);
	child->parent_page_offset_num = *old_page_pagenum;
	file_write_page(child_page_pagenum, child);
	for (i = 0; i< old_page->num_keys; i++){
		child_page_pagenum = old_page->entries[i].child_page_offset_num;
		file_read_page(child_page_pagenum, child);
		child->parent_page_offset_num = *old_page_pagenum;
		file_write_page(child_page_pagenum, child);
	}

	child_page_pagenum = new_page->pointer_num;
	file_read_page(child_page_pagenum, child);
	child->parent_page_offset_num = new_page_pagenum;
	file_write_page(child_page_pagenum, child);
    for (i = 0; i < new_page->num_keys; i++) {
        //child = new_node->pointers[i];
        child_page_pagenum = new_page->entries[i].child_page_offset_num;
        file_read_page(child_page_pagenum, child);
        //child->parent = new_node;
        child->parent_page_offset_num = new_page_pagenum;
        file_write_page(child_page_pagenum, child);
    }
	free(child);
    free(new_page);
    return;
}



/* Inserts a new node (leaf or internal node) into the B+ tree.
 * Returns the root of the tree after insertion.
 */
//node * insert_into_parent(node * root, node * left, int key, node * right) {
void insert_into_parent(page_t * left, pagenum_t * left_pagenum, int64_t key, 
    page_t * right, pagenum_t * right_pagenum) {

//	printf("\ninsert_into_parent function\n");

    int left_index;
    //node * parent;

    //parent = left->parent;

    /* Case: new root. */

    //if (parent == NULL)
    if(left->parent_page_offset_num == 0) {
        //return insert_into_new_root(left, key, right);
//		printf("page to insert is root!\n");
        insert_into_new_root(left, left_pagenum, key, right, right_pagenum);
        return;
    }

    /* Case: leaf or node. (Remainder of
     * function body.)  
     */

    /* Find the parent's pointer to the left 
     * node.
     */

    page_t * parent = (page_t*)calloc(1, PAGESIZE);
    pagenum_t parent_pagenum = left->parent_page_offset_num;
    file_read_page(parent_pagenum, parent);
    //left_index = get_left_index(parent, left);
    left_index = get_left_index(parent, *left_pagenum);
//	printf("left_index %d\n", left_index);
//	printf("rpn %lld\n", *right_pagenum);

    /* Simple case: the new key fits into the node. 
     */

    if (parent->num_keys < order - 1) {
        //return insert_into_node(root, parent, left_index, key, right);
        insert_into_node(parent, parent_pagenum, left_index, key, *right_pagenum);
        free(parent);
        return;
    }

    /* Harder case:  split a node in order 
     * to preserve the B+ tree properties.
     */

    //return insert_into_node_after_splitting(root, parent, left_index, key, right);
    insert_into_node_after_splitting(parent, &parent_pagenum, left_index, key, right, right_pagenum);
    free(parent);
    return;
}


/* Creates a new root for two subtrees
 * and inserts the appropriate key into
 * the new root.
 */
//node * insert_into_new_root(node * left, int key, node * right) {
void insert_into_new_root(page_t * left, pagenum_t * left_pagenum, int64_t key, 
    page_t * right, pagenum_t * right_pagenum) {
	
//	printf("\ninsert_into_new_root function\n");

    //node * root = make_node();
    page_t * new_root;
    pagenum_t new_root_pagenum = header_page->num_pages;

	int i;
//	printf("* left child keys\n");
//	for(i = 0; i < left->num_keys; i++) printf("%lld ",left->records[i].key);
//	printf("\n* right child keys\n");
//	for(i = 0; i < right->num_keys; i++) printf("%lld ", right->records[i].key);
//	printf("\n");

    if(header_page->free_page_offset_num != 0){
//		printf("free page list exist!\n");
        new_root = (page_t*)calloc(1, PAGESIZE);
        //pagenum_t new_free_page_num = header_page->free_page_offset_num;
        pagenum_t new_free_page_num = file_alloc_page();
        new_root_pagenum = new_free_page_num;
        file_read_page(new_free_page_num, new_root);
        header_page->free_page_offset_num = new_root->pointer_num;
        new_root->pointer_num = 0;
    }
    else{
//		printf("free page list empty!\n");
		new_root = make_node();
	}

    /*
    root->keys[0] = key;
    root->pointers[0] = left;
    root->pointers[1] = right;
	root->num_keys++;
    root->parent = NULL;
    left->parent = root;
    right->parent = root;
    */
	
//	printf("checking sf\n");

//	printf("Before left pagenum = %lld right pagenum = %lld\n", *left_pagenum, *right_pagenum);
    new_root->pointer_num = new_root_pagenum;
    new_root->entries[0].key = key;
    new_root->entries[0].child_page_offset_num = *right_pagenum;
    new_root->num_keys++;
	left->parent_page_offset_num = *left_pagenum;
	right->parent_page_offset_num = *left_pagenum;
	new_root->parent_page_offset_num = 0;

//	printf("after insert into root\n");
//	printf("* left child keys\n");
//	for(i=0;i<left->num_keys;i++) printf("%lld ",left->records[i].key);
//	printf("\n* right child keys\n");
//	for(i=0;i<right->num_keys;i++) printf("%lld ",right->records[i].key);
//	printf("\n");
//	printf("left pagenum = %lld right pagenum = %lld\n", new_root_pagenum, *right_pagenum);

    file_write_page(*left_pagenum, new_root);
    file_write_page(new_root_pagenum, left);
    file_write_page(*right_pagenum, right);

    file_read_page(*left_pagenum, root_page);
	*left_pagenum = new_root_pagenum;
//	printf("r l p = %lld r r p = %lld\n r first key = %lld\n", root_page->pointer_num, root_page->entries[0].child_page_offset_num, root_page->entries[0].key);

    free(new_root);
 
	//return root;
    return;
}



/* First insertion:
 * start a new tree.
 */
//node * start_new_tree(int key, record * pointer) {
void start_new_tree( record_t * record ) {
    //node * root = make_leaf();
    root_page = make_leaf();
    //root->keys[0] = key;
    //root->pointers[0] = pointer;
    //root->pointers[order - 1] = NULL;
    //root->parent = NULL;
    root_page->parent_page_offset_num = 0;
    root_page->records[0].key = record->key;
    strcpy(root_page->records[0].value, record->value);
    root_page->num_keys++;
    //return root;
}



/* Master insertion function.
 * Inserts a key and an associated value into
 * the B+ tree, causing the tree to be adjusted
 * however necessary to maintain the B+ tree
 * properties.
 */
//node * insert( node * root, int key, int value ) {
int insert ( int64_t key, char * value ) {
    //record * pointer;
    record_t * record;
    //node * leaf;
    //page_t * leaf;
    pagenum_t pagenum;

    /* The current implementation ignores
     * duplicates.
     */

    //if (find(root, key, false) != NULL)
	char * find_ret = find(key);
//	printf("%s\n", find_ret==NULL?"null":"not null");
    //if(find(key) != NULL)
	if(find_ret != NULL)
        //return root;
        return 1;   //Already, tree has the record

    /* Create a new record for the
     * value.
     */
    //pointer = make_record(value);
    record = make_record(key, value);

//    printf("make record (key = %lld)\n", key);
    /* Case: the tree does not exist yet.
     * Start a new tree.
     */

    //if (root == NULL) 
    if(root_page == NULL) {
        //return start_new_tree(key, pointer);
        start_new_tree(record);
        header_page->root_page_offset_num = ROOTPAGENUM;
        //header_page->num_pages++;
        file_write_page(ROOTPAGENUM, root_page);
		file_write_page(HEADERPAGENUM, header_page);
        free(record);
//		printf("start new tree!\n");
        return 0;
    }


    /* Case: the tree already exists.
     * (Rest of function body.)
     */

//	if(!root_page->is_leaf)
//		printf("root page first key = %lld\n", root_page->entries[0].key);
    //leaf = find_leaf(root, key, false);
    page_t * leaf = find_leaf(key, &pagenum);

//	printf("find the leaf page pagenum = %lld key = %lld\n", pagenum, key);

    /* Case: leaf has room for key and pointer.
     */

    if (leaf->num_keys < order - 1) {
        //insert_into_leaf(leaf, key, pointer);
		insert_into_leaf(leaf, record);
        file_write_page(pagenum, leaf);
        //free(leaf);
//		printf("\nfree leaf variable succesful!\n");
        free(record);
//		printf("free record variable succesful!\n");
        return 0;
    }


    /* Case:  leaf must be split.
     */

    //insert_into_leaf_after_splitting(root, leaf, key, pointer);
    insert_into_leaf_after_splitting(leaf, record, pagenum);
    free(leaf);
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
//int get_neighbor_index( node * n ) {
pagenum_t get_neighbor_index( page_t * n, pagenum_t n_pagenum, bool * is_most_left ,
    int * parent_entries_index) {

    int i;
    page_t * parent_page = (page_t*)calloc(1,PAGESIZE);

    file_read_page(n->parent_page_offset_num, parent_page);

    /* Return the index of the key to the left
     * of the pointer in the parent pointing
     * to n.  
     * If n is the leftmost child, this means
     * return -1.
     */
/*
    for (i = 0; i <= n->parent->num_keys; i++)
        if (n->parent->pointers[i] == n)
            return i - 1;
*/
    if(parent_page->pointer_num == n_pagenum){
        pagenum_t ret = parent_page->entries[0].child_page_offset_num;
        free(parent_page);
        *is_most_left = true;
        *parent_entries_index = 0;
        return ret;
    }
    for(i = 0; i <= parent_page->num_keys; i++){
        if(parent_page->entries[i].child_page_offset_num == n_pagenum){
            pagenum_t ret;
            if(i == 0)
                ret = parent_page->pointer_num;
            else
                ret = parent_page->entries[i-1].child_page_offset_num;
            free(parent_page);
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


//node * remove_entry_from_node(node * n, int key, node * pointer) {
page_t * remove_entry_from_node( page_t * n, pagenum_t n_pagenum, int64_t key ) {

	printf("\nremove_entry_from_node function\n");

    //int i, num_pointers;
    int i;

    // Remove the key and shift other keys accordingly.
    i = 0;
/*
    while (n->keys[i] != key)
        i++;
    for (++i; i < n->num_keys; i++)
        n->keys[i - 1] = n->keys[i];
*/
    i = 0;
    if(n->is_leaf){
        while(n->records[i].key != key)
            i++;
        for(++i; i < n->num_keys; i++){
            n->records[i - 1].key = n->records[i].key;
            strcpy(n->records[i - 1].value, n->records[i].value);
        }
    }
    else{
        while(n->entries[i].key != key)
            i++;
        for(++i; i < n->num_keys; i++){
            n->entries[i - 1].key = n->entries[i].key;
            n->entries[i - 1].child_page_offset_num = n->entries[i].child_page_offset_num;
        }
    }

    // Remove the pointer and shift other pointers accordingly.
    // First determine number of pointers.
    //num_pointers = n->is_leaf ? n->num_keys : n->num_keys + 1;
/*
    i = 0;
    while (n->pointers[i] != pointer)
        i++;
    for (++i; i < num_pointers; i++)
        n->pointers[i - 1] = n->pointers[i];
*/


    // One key fewer.
    n->num_keys--;

    // Set the other pointers to NULL for tidiness.
    // A leaf uses the last pointer to point to the next leaf.
    if (n->is_leaf){
        for (i = n->num_keys; i < order; i++){
            //n->pointers[i] = NULL;
            n->records[i].key = 0;
            memset(n->records[i].value, 0, sizeof(n->records[i].value));
        }
    }
    else{
        for (i = n->num_keys; i < order; i++){
            //n->pointers[i] = NULL;
            n->entries[i].key = 0;
            n->entries[i].child_page_offset_num = 0;
        }
    }

    file_write_page(n_pagenum, n);

    return n;
}


//node * adjust_root(node * root) {
void adjust_root(void) {
	printf("adjust root function\n");
    printf("root keys num %d\n", root_page->num_keys);
	//node * new_root;

    /* Case: nonempty root.
     * Key and pointer have already been deleted,
     * so nothing to be done.
     */

    //if (root->num_keys > 0)
    if(root_page->num_keys > 0)
        //return root;
        return;

    /* Case: empty root. 
     */

    // If it has a child, promote 
    // the first (only) child
    // as the new root.

    //if (!root->is_leaf) {
    if(!root_page->is_leaf) {
        //new_root = root->pointers[0];
        //new_root->parent = NULL;
        page_t * new_root_page = (page_t*)calloc(1, PAGESIZE);
        pagenum_t new_root_page_pagenum = root_page->pointer_num;
        file_read_page(new_root_page_pagenum, new_root_page);
        root_page = new_root_page;
        file_write_page(ROOTPAGENUM, root_page);
        file_free_page(new_root_page_pagenum);
        free(new_root_page);
    }

    // If it is a leaf (has no children),
    // then the whole tree is empty.

    else{
		printf("make root page null\n");
        //new_root = NULL;
        root_page = NULL;
	}

/*
    free(root->keys);
    free(root->pointers);
    free(root);

    return new_root;
*/
    return;
}

/* Redistributes entries between two nodes when
 * one has become too small after deletion
 * but its neighbor is too big to append the
 * small node's entries without exceeding the
 * maximum
 */
//node * redistribute_nodes(node * root, node * n, node * neighbor, int neighbor_index, 
//        int k_prime_index, int k_prime) {  
void redistribute_nodes( page_t * n, pagenum_t n_pagenum, page_t * neighbor,
    pagenum_t neighbor_pagenum, page_t * parent, pagenum_t parent_pagenum,
    bool is_most_left, int entries_index) {

    int i;
    //node * tmp;

    /* Case: n has a neighbor to the left. 
     * Pull the neighbor's last key-pointer pair over
     * from the neighbor's right end to n's left end.
     */

    //if (neighbor_index != -1) {
    if(!is_most_left) {
/*
        if (!n->is_leaf)
            n->pointers[n->num_keys + 1] = n->pointers[n->num_keys];
        for (i = n->num_keys; i > 0; i--) {
            n->keys[i] = n->keys[i - 1];
            n->pointers[i] = n->pointers[i - 1];
        }
        if (!n->is_leaf) {
            n->pointers[0] = neighbor->pointers[neighbor->num_keys];
            tmp = (node *)n->pointers[0];
            tmp->parent = n;
            neighbor->pointers[neighbor->num_keys] = NULL;
            n->keys[0] = k_prime;
            n->parent->keys[k_prime_index] = neighbor->keys[neighbor->num_keys - 1];
        }
        else {
            n->pointers[0] = neighbor->pointers[neighbor->num_keys - 1];
            neighbor->pointers[neighbor->num_keys - 1] = NULL;
            n->keys[0] = neighbor->keys[neighbor->num_keys - 1];
            n->parent->keys[k_prime_index] = n->keys[0];
        }
*/
        if(!n->is_leaf){
            for(i = n->num_keys; i > 0; i--){
                n->entries[i].key = n->entries[i - 1].key;
                n->entries[i].child_page_offset_num = n->entries[i - 1].child_page_offset_num;
            }
            n->entries[0].child_page_offset_num = n->pointer_num;
            n->entries[0].key = parent->entries[entries_index].key;
            n->pointer_num = neighbor->entries[neighbor->num_keys - 1].child_page_offset_num;
            
            page_t * temp_page = (page_t*)calloc(1, PAGESIZE);
            file_read_page(n->pointer_num, temp_page);
            temp_page->parent_page_offset_num = n_pagenum;
            file_write_page(n->pointer_num, temp_page);
            free(temp_page);

            parent->entries[entries_index].key = neighbor->entries[neighbor->num_keys - 1].key;
            neighbor->entries[neighbor->num_keys - 1].key = 0;
            neighbor->entries[neighbor->num_keys - 1].child_page_offset_num = 0;
        }
        else{
            for(i = n->num_keys; i > 0; i--){
                n->records[i].key = n->entries[i - 1].key;
                strcpy(n->records[i].value, n->records[i - 1].value);
            }
            n->records[0].key = neighbor->records[neighbor->num_keys - 1].key;
            strcpy(n->records[0].value, neighbor->records[neighbor->num_keys - 1].value);
            parent->entries[entries_index].key = n->records[0].key;
            neighbor->records[neighbor->num_keys - 1].key = 0;
            memset(neighbor->records[neighbor->num_keys - 1].value, 0, sizeof(neighbor->records[neighbor->num_keys - 1].value));
        }
    }

    /* Case: n is the leftmost child.
     * Take a key-pointer pair from the neighbor to the right.
     * Move the neighbor's leftmost key-pointer pair
     * to n's rightmost position.
     */

    else {  
/*
        if (n->is_leaf) {
            n->keys[n->num_keys] = neighbor->keys[0];
            n->pointers[n->num_keys] = neighbor->pointers[0];
            n->parent->keys[k_prime_index] = neighbor->keys[1];
        }
        else {
            n->keys[n->num_keys] = k_prime;
            n->pointers[n->num_keys + 1] = neighbor->pointers[0];
            tmp = (node *)n->pointers[n->num_keys + 1];
            tmp->parent = n;
            n->parent->keys[k_prime_index] = neighbor->keys[0];
        }
        for (i = 0; i < neighbor->num_keys - 1; i++) {
            neighbor->keys[i] = neighbor->keys[i + 1];
            neighbor->pointers[i] = neighbor->pointers[i + 1];
        }
        if (!n->is_leaf)
            neighbor->pointers[i] = neighbor->pointers[i + 1];
*/
        if(!n->is_leaf){
            n->entries[n->num_keys].key = parent->entries[entries_index].key;
            n->entries[n->num_keys].child_page_offset_num = neighbor->pointer_num;
            parent->entries[entries_index].key = neighbor->entries[0].key;

            page_t * temp_page = (page_t*)calloc(1, PAGESIZE);
            file_read_page(n->entries[n->num_keys].child_page_offset_num, temp_page);
            temp_page->parent_page_offset_num = n_pagenum;
            file_write_page(n->entries[n->num_keys].child_page_offset_num, temp_page);
            free(temp_page);

            neighbor->pointer_num = neighbor->entries[0].child_page_offset_num;
            for(i = 0; i < neighbor->num_keys - 1; i++){
                neighbor->entries[i].key = neighbor->entries[i + 1].key;
                neighbor->entries[i].child_page_offset_num = neighbor->entries[i + 1].child_page_offset_num;
            }
            neighbor->entries[neighbor->num_keys - 1].key = 0;
            neighbor->entries[neighbor->num_keys - 1].child_page_offset_num = 0;
        }
        else{
            n->records[n->num_keys].key = neighbor->records[0].key;
            strcpy(n->records[n->num_keys].value, neighbor->records[0].value);
            for(i = 0; i < neighbor->num_keys - 1; i++){
                neighbor->records[i].key = neighbor->records[i + 1].key;
                strcpy(neighbor->records[i].value, neighbor->records[i + 1].value);
            }
            parent->entries[entries_index].key = neighbor->records[0].key;
            neighbor->records[neighbor->num_keys - 1].key = 0;
            memset(neighbor->records[neighbor->num_keys - 1].value, 0, sizeof(neighbor->records[neighbor->num_keys - 1].value));
        }       
    }

    /* n now has one more key and one more pointer;
     * the neighbor has one fewer of each.
     */

    n->num_keys++;
    neighbor->num_keys--;

    file_write_page(n_pagenum, n);
    file_write_page(neighbor_pagenum, neighbor);
    file_write_page(parent_pagenum, parent);

    return;
}


/* Deletes an entry from the B+ tree.
 * Removes the record and its key and pointer
 * from the leaf, and then makes all appropriate
 * changes to preserve the B+ tree properties.
 */
//node * delete_entry( node * root, node * n, int key, void * pointer ) {
void delete_entry( page_t * n, pagenum_t n_pagenum, int64_t key, 
    char * key_record_value) {

	printf("\ndelete_entry function\n");

    int min_keys;
    //node * neighbor;
    page_t * neighbor,  * parent;
    //int neighbor_index;
    pagenum_t neighbor_pagenum, parent_pagenum;
    int capacity;

    // Remove key and pointer from node.

    //n = remove_entry_from_node(n, key, pointer);
    remove_entry_from_node(n, n_pagenum, key);
	printf("n keys %d\n", n->num_keys);

    /* Case:  deletion from the root. 
     */

    //if (n == root) 
//    if(n == root_page){
	if(n->parent_page_offset_num == 0) {
        //return adjust_root(root);
		root_page = n;
        adjust_root();
        return;
    }


    /* If the page has no keys (empty)
     * add to free page list
     * Delayed Merge
     */
    if(n->num_keys == 0) {
		int i, index = 0;
		parent_pagenum = n->parent_page_offset_num;
		parent = (page_t*)calloc(1, PAGESIZE);
		file_read_page(parent_pagenum, parent);

		printf("Delayed merge\n");
		if(parent->pointer_num == n_pagenum){
			parent->pointer_num = parent->entries[0].child_page_offset_num;
			for(i = 0; i < parent->num_keys; i++){
				parent->entries[i].key = parent->entries[i + 1].key;
				parent->entries[i].child_page_offset_num = parent->entries[i + 1].child_page_offset_num;
			}
		}
		else{
			for(i = 0; i < parent->num_keys; i++){
				if(parent->entries[i].child_page_offset_num == n_pagenum){
					index = i;
					break;
				}
			}
			parent->entries[index].child_page_offset_num = parent->entries[index + 1].child_page_offset_num;
			for(i = index+1; i < parent->num_keys - 1; i++){
				parent->entries[i].key = parent->entries[i + 1].key;
				parent->entries[i].child_page_offset_num = parent->entries[i + 1].child_page_offset_num;
			}
		}
		parent->num_keys--;
		for(i = parent->num_keys; i < order; i++){
			parent->entries[i].key = 0;
			parent->entries[i].child_page_offset_num = 0;
		}

		file_write_page(parent_pagenum, parent);
		free(parent);
		file_free_page(n_pagenum);
        return;
    }

    /* Case:  deletion from a node below the root.
     * (Rest of function body.)
     */

    /* Determine minimum allowable sizae of node,
     * to be preserved after deletion.
     */

/*****************/


//    min_keys = n->is_leaf ? cut(order - 1) : cut(order) - 1;

    /* Case:  node stays at or above minimum.
     * (The simple case.)
     */

//    if (n->num_keys >= min_keys || n->parent_page_offset_num == 0)
        //return root;
//        return;

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

    //neighbor_index = get_neighbor_index( n );
//    bool is_most_left;
//    int parent_entries_index;
//    neighbor_pagenum = get_neighbor_index(n, n_pagenum, &is_most_left, &parent_entries_index);
    //k_prime_index = neighbor_index == -1 ? 0 : neighbor_index;
//    neighbor = (page_t*)calloc(1, PAGESIZE);
//    parent = (page_t*)calloc(1, PAGESIZE);
//    parent_pagenum = n->parent_page_offset_num;
//    file_read_page(neighbor_pagenum, neighbor);
//    file_read_page(parent_pagenum, parent);
/*    
    k_prime_pagenum = is_most_left ? 
    k_prime = n->parent->keys[k_prime_index];
    neighbor = neighbor_index == -1 ? n->parent->pointers[1] : 
        n->parent->pointers[neighbor_index];
*/
//    capacity = n->is_leaf ? order : order - 1;

    /* Coalescence. */
/*
    if (neighbor->num_keys + n->num_keys < capacity)
        return coalesce_nodes(root, n, neighbor, neighbor_index, k_prime);
*/
    /* Redistribution. */

    //else
//    if (neighbor->num_keys + n->num_keys < capacity){
        //return redistribute_nodes(root, n, neighbor, neighbor_index, k_prime_index, k_prime);
//        redistribute_nodes(n, n_pagenum, neighbor, neighbor_pagenum, parent, parent_pagenum, is_most_left, parent_entries_index);
//    }
//    free(neighbor);
//    free(parent);
    return;
}



/* Master deletion function.
 */
//node * delete(node * root, int key) {
int delete(int64_t key) {

    //node * key_leaf;
    page_t * key_leaf_page;
    //record * key_record;
    char * key_record_value;
    pagenum_t key_leaf_page_pagenum;

    //key_record = find(root, key, false);
    key_record_value = find(key);
    //key_leaf = find_leaf(root, key, false);
    key_leaf_page = find_leaf(key, &key_leaf_page_pagenum);
    if (key_record_value != NULL && key_leaf_page != NULL) {
        //root = delete_entry(root, key_leaf, key, key_record);
		printf("delete start!\n");
        delete_entry(key_leaf_page, key_leaf_page_pagenum, key, key_record_value);
        //free(key_record);
        return 0;
    }
    return -1;
}
