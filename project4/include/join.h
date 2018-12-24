#ifndef __JOIN_H__
#define __JOIN_H__

#include "bpt.h"
#include <string>
#include <vector>
#include <utility>
using namespace std;

typedef struct join_info{
	int table_id;
	int column_id;
} join_info;

bool cmp(const pair<join_info, join_info> &p1, const pair<join_info, join_info> &p2);

typedef struct join_table{
	int table_id;
	int num_col;
	vector<vector<int64_t> > table;	
} join_table;

vector<string> parse_qurey(const char * query);
pair<join_info, join_info> parse_join(string str);
vector<join_table> get_all_table();

int64_t join(const char * query);

#endif /* __JOIN_H__*/
