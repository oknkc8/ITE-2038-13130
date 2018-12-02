#include "join.h"
#include <algorithm>
#include <unordered_map>
#define INF (-10000)

extern int table_fd[10], table_fd_count, table_col[10];
extern char table_id_arr[10][100];

bool cmp(const pair<join_info, join_info> &p1, const pair<join_info, join_info> &p2){
    if(p1.first.table_id < p2.first.table_id) return true;
    else return false;
}

vector<string> parse_query(const char * query) {
    vector<string> ret;
    string q = query;
    // parse the query
    while (1) {
        if (q.find("&") == -1) {
            ret.push_back(q);
            break;
        }
        string tmp = q.substr(0, q.find("&"));
        ret.push_back(tmp);
        q = q.substr(q.find("&")+1);
    }

    return ret;
}

pair<join_info, join_info> parse_join(string str) {
    string s1, s2;
    s1 = str.substr(0, str.find("="));
    s2 = str.substr(str.find("=") + 1);

    join_info j1, j2;
    j1.table_id = stoi(s1.substr(0, s1.find(".")))-1;
    j1.column_id = stoi(s1.substr(s1.find(".") + 1)) -1;
    j2.table_id = stoi(s2.substr(0, s2.find(".")))-1;
    j2.column_id = stoi(s2.substr(s1.find(".") + 1))-1;

    if(j1.table_id > j2.table_id) swap(j1, j2);

    return make_pair(j1, j2);
}

vector<join_table> get_all_table(){
    vector<join_table> ret(table_fd_count);
    for(int i = 0; i < table_fd_count; i++){
        pagenum_t tmp;
        int64_t p = INF;
        ret[i].table_id = i+1, ret[i].num_col = get_num_col(i+1);
        printf("num_col : %d\n",ret[i].num_col);
        printf("check1\n");
        print_buf();
        buffer_t * now = find_leaf(i+1, p, &tmp);
        printf("check2\n");
        while(1){
            printf("page num keys : %d\n", now->page.num_keys);
            for(int j = 0; j < now->page.num_keys; j++){
                printf("check3\n");
                vector<int64_t> col;
                col.push_back(now->page.records[j].key);
                printf("%lld ", now->page.records[j].key);
                for(int k = 0; k < ret[i].num_col - 1; k++){
                    col.push_back(now->page.records[j].values[k]);
                    printf("%lld ", now->page.records[j].values[k]);
                }
                printf("\n");
                ret[i].table.push_back(col);
            }
            if(now->page.pointer_num == 0) break;
            now = buf_get_page(ret[i].table_id, now->page.pointer_num);
            buf_put_page(now);
        }
    }

    return ret;
}

int64_t join(const char* query) {
    // Parse the query
    vector<string> joins = parse_query(query);
    vector< pair<join_info, join_info> > table_info;
    vector<join_table> tables = get_all_table(), result;
    int table_check[10] = {0};
    int64_t ret = 0;

    for (int i = 0; i < joins.size(); i++) {
        table_info.push_back(parse_join(joins[i]));
    }

    sort(table_info.begin(), table_info.end(), cmp);
    result.resize(table_fd_count);

    // Initialize
    // Put the first table
    table_check[table_info[0].first.table_id] = 1;
    result[table_info[0].first.table_id].table_id = table_info[0].first.table_id;
    result[table_info[0].first.table_id].num_col = tables[table_info[0].first.table_id].num_col;
    //printf("%d\n", tables[table_info[0].first.table_id].num_col);
    for(int i = 0; i < tables[table_info[0].first.table_id].table.size(); i++){
        result[table_info[0].first.table_id].table.push_back(tables[table_info[0].first.table_id].table[i]);
        //printf("%d\n",tables[table_info[0].first.table_id].table[i].size());
        //for(int j = 0; j < tables[table_info[0].first.table_id].table[i].size(); j++)
        //    printf("%lld ",tables[table_info[0].first.table_id].table[i][j]);
        //printf("\n");
    }
    //printf("\n");

    // Join tables
    for(int i = 0; i < table_info.size(); i ++){
        join_info left = table_info[i].first, right = table_info[i].second;
        vector<join_table> temp(table_fd_count);

        // Hash the left table
        unordered_map<int64_t, int> t;
        //printf("%d\n",result[left.table_id].table.size());
        for(int j = 0; j < result[left.table_id].table.size(); j++){
            int x_key = result[left.table_id].table[j][left.column_id];
            //printf("%d %d\n", j, x_key);
            t[x_key] = j;
        }

        // Two Cases
        // Case 1 :  right table already in result table
        // Case 2 :  right table not in result table

        // Case 1
        if(table_check[right.table_id]){
            printf("checked\n");
            for(int j = 0; j < min(result[left.table_id].table.size(), result[right.table_id].table.size()); j++){
                if(result[left.table_id].table[j][left.column_id] == result[right.table_id].table[j][right.column_id]){
                    for(int k = 0; k < result.size(); k++){
                        if(result[k].table.size() < j) continue;
                        vector<int64_t> temp_col;
                        for(int l = 0; l < result[k].num_col; l++){
                            temp_col.push_back(result[k].table[j][l]);
                        }
                        temp[k].table.push_back(temp_col);
                    }
                }
            }
        }
        // Case 2
        else{
            printf("not check\n");
            table_check[right.table_id] = 1;
            result[right.table_id].num_col = tables[right.table_id].num_col;
            //printf("%d\n", tables[right.table_id].table.size());
            for(int j = 0; j < tables[right.table_id].table.size(); j++){
                int y_key = tables[right.table_id].table[j][right.column_id];
                printf("%d ", y_key);
                if(t.find(y_key) != t.end()){
                    printf("%d \n", t[y_key]);
                    for(int k = 0; k < result.size(); k++){
                        vector<int64_t> temp_col;
                        if(k == right.table_id){
                            printf("same\n");
                            for(int l = 0; l < tables[right.table_id].num_col; l++){
                                temp_col.push_back(tables[right.table_id].table[j][l]);
                                printf("check1\n");
                            }
                        }
                        else{
                            printf("not same\n");
                            if(result[k].table.size() < j) continue;
                            for(int l = 0; l < result[k].num_col; l++){
                                temp_col.push_back(result[k].table[t[y_key]][l]);
                                printf("check2\n");
                            }
                        }
                        temp[k].table.push_back(temp_col);
                        printf("check3\n");
                    }
                }
            }
        }
        for(int j = 0; j < result.size(); j++){
          temp[i].table_id = result[i].table_id;
          temp[i].num_col = result[i].num_col;
        }
        result.clear();
        result = temp;
        printf("check4\n");
    }

    // Print the result
    printf("%d\n", result.size());
    for(int i = 0; i < result.size(); i++){
        printf("%d\n", result[i].num_col);
        for(int j = 0; j < result[i].num_col; j++){
            printf("%d %d  ", i+1, j+1);
        }
    }
    printf("\n");
    for(int i = 0; i < result.size() * result[i].num_col * 4; i++) printf("-");
    printf("\n");
    for(int j = 0; j < result[0].table.size(); j++){
        for(int i = 0; i < result.size(); i++){
            ret += result[i].table[j][0];
            for(int k = 0; k < result[i].num_col; k++){
                printf("%3lld ", (long long)result[i].table[j][k]);
            }
        }
        printf("\n");
    }
    printf("\n");

    return ret;
}