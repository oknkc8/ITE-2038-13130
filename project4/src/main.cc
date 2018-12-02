#include "join.h"
#include <inttypes.h>

int main( int argc, char ** argv ) {

    char input_file[100];
    int buf_cap = 10;
    //char value[120];
    int64_t values[15];
    int64_t * buf;
    FILE * fp;
    int64_t input, ret;
    char instruction;
    char license_part;
    int result;
    int table_id = 0;

    if (argc > 1) {
        order = atoi(argv[1]);
        if (order < MIN_ORDER || order > MAX_ORDER) {
            fprintf(stderr, "Invalid order: %d .\n\n", order);
            usage_3();
            exit(EXIT_FAILURE);
        }
    }

    license_notice();
    usage_1();  
    usage_2();

    if(argc > 2){
        buf_cap = atoi(argv[2]);
        init_db(buf_cap);
    }
    else{
        //printf("%d",buf_cap);
        init_db(buf_cap);
    }

    int tid, num_column, now_num_col;
    char query[100];
    printf("> ");
    while (scanf("%c", &instruction) != EOF) {
        switch (instruction) {
        case 'o':
            //if(table_id) close_table(table_id);
            scanf("%s %d", input_file, &num_column);
            table_id = open_table(input_file,num_column);
            printf("\nOpen or Create DB file %s Successful!\n", input_file);
            break;
        case 'd':
            if(!table_id){
                printf("Error!\nSelect the DB file!\n\n");
                break;
            }
            scanf("%" PRIu64, &input);
            result = erase(table_id, input);
            if(result){
                printf("delete error!\n");
                break;
            }
            print_tree(table_id);
            print_buf();
            break;
        case 'i':
            if(!table_id){
                printf("Error!\nSelect the DB file!\n\n");
                break;
            }
            scanf("%"PRId64, &input);
            now_num_col = get_num_col(table_id);
            for(int i = 0; i < now_num_col - 1; i++) scanf("%"PRId64, &values[i]);
            result = insert(table_id, input, values);
            if(result){
                printf("same key!\n");
                break;
            }
            print_tree(table_id);
            print_buf();
            break;
        case 'f':
            if(!table_id){
                printf("Error!\nSelect the DB file!\n\n");
                break;
            }
            scanf("%" PRIu64, &input);
            buf = find(table_id, input);
            if(buf==NULL) printf("Not Found!\n");
            else{
                printf("Key : ""%"PRId64 " value : ",input);
                now_num_col = get_num_col(table_id);
                for(int i = 0; i < now_num_col - 1; i++) printf("%"PRId64" ", buf[i]);
                printf("\n");
            }
            break;
        case 'p':
            scanf("%" PRIu64, &input);
            find_and_print(table_id, input, instruction == 'p');
            break;
        case 'l':
            if(!table_id){
                printf("Error!\nSelect the DB file!\n\n");
                break;
            }
            print_leaves(table_id);
            break;
        case 'q':
            shutdown_db();
            while (getchar() != (int)'\n');
            return EXIT_SUCCESS;
            break;
        case 't':
            if(!table_id){
                printf("Error!\nSelect the DB file!\n\n");
                break;
            }
            print_tree(table_id);
            print_buf();
            break;
        case 'c':
            scanf("%d", &tid);
            close_table(tid);
            break;
        case 'j':
            scanf("%s", query);
            ret = join(query);
            printf("%" PRIu64"\n\n", ret);
            break;
        default:
            usage_2();
            break;
        }
        while (getchar() != (int)'\n');
        printf("> ");
    }
    printf("\n");

    return EXIT_SUCCESS;
}
