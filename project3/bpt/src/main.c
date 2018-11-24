/*
// test code
#include "bpt.h"
#include <time.h>
#include <ctype.h>
#include <inttypes.h>
#include <stdbool.h>

int main(int argc, char ** argv) {

    int64_t input;
    char instruction;
	char value[120];
    open_db("test.db");
    while(scanf("%c", &instruction) != EOF){
        switch(instruction){
            case 'i':
                scanf("%"PRId64 "%s", &input, value);
                insert(input, value);
                break;
            case 'f':
                scanf("%" PRIu64, &input);
                char * buf = find(input);
                if (buf != NULL) {
                    printf("Key: ""%"PRId64 ", Value: %s\n", input, buf);
                } else
                    printf("Not Exists\n");

                fflush(stdout);
                break;
            case 'd':
                scanf("%" PRIu64, &input);
                delete(input);
                break;
            case 'q':
                while(getchar() != (int)'\n');
                return EXIT_SUCCESS;
                break;
        }
        while (getchar() != (int)'\n');
    }
    printf("\n");
    return EXIT_SUCCESS;
}
*/

#include "bpt.h"
#include <inttypes.h>

int main( int argc, char ** argv ) {

    char input_file[100];
    int buf_cap = 10;
    char value[120];
    FILE * fp;
    int64_t input;
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


/*    if (argc > 3) {
        input_file = argv[2];
        table_id = open_table(input_file);
        printf("\nOpen or Create file Successful!\n");
    }
    else{
        table_id = open_table(input_file);
        printf("\nOpen or Create file Successful!\n");
    }
    */

    printf("> ");
    while (scanf("%c", &instruction) != EOF) {
        switch (instruction) {
        case 'o':
            if(table_id) close_table(table_id);
            scanf("%s", input_file);
            table_id = open_table(input_file);
            printf("\nOpen or Create DB file %s Successful!\n", input_file);
            break;
        case 'd':
            if(!table_id){
                printf("Error!\nSelect the DB file!\n\n");
                break;
            }
            scanf("%" PRIu64, &input);
            result = delete(table_id, input);
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
            scanf("%"PRId64 "%s", &input, value);
            result = insert(table_id, input, value);
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
			char * buf = find(table_id, input);
			if(buf==NULL) printf("Not Found!\n");
			else printf("Key : ""%"PRId64 " value : %s\n",input, buf);
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