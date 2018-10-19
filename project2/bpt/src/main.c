#include "bpt.h"

int main( int argc, char ** argv ) {

    const char * input_file = "default_input";
	char value[120];
    FILE * fp;
    //node * root;
    //int range2;
    int64_t input;
    char instruction;
    char license_part;
    int result;

    //root = NULL;
    //root_page = NULL;
    //verbose_output = false;

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

    if (argc > 2) {
        input_file = argv[2];
        /*fp = fopen(input_file, "r");
        if (fp == NULL) {
            perror("Failure  open input file.");
            exit(EXIT_FAILURE);
        }
        while (!feof(fp)) {
            fscanf(fp, "%d\n", &input);
            root = insert(root, input, input);
        }
        fclose(fp);*/
        //print_tree(root);
        open_db(input_file);
        //print_tree();
        printf("\nOpen or Create file Successful!\n");
    }
	else{
		open_db(input_file);
		printf("\nOpen or Create file Successful!\n");
	}

    printf("> ");
    while (scanf("%c", &instruction) != EOF) {
        switch (instruction) {
        case 'd':
            scanf("%lld", &input);
            //root = delete(root, input);
            result = delete(input);
            if(result){
                printf("delete error!\n");
                return EXIT_FAILURE;
            }
            //print_tree(root);
            print_tree();
            break;
        case 'i':
            scanf("%lld %s", &input, value);
            //root = insert(root, input, input);
            result = insert(input, value);
            if(result){
                printf("same key!\n");
                break;
            }
            //print_tree(root);
            print_tree();
            break;
        case 'f':
        case 'p':
            scanf("%lld", &input);
            //find_and_print(root, input, instruction == 'p');
            find_and_print(input, instruction == 'p');
            break;
        case 'l':
            //print_leaves(root);
            print_leaves();
            break;
        case 'q':
            while (getchar() != (int)'\n');
            return EXIT_SUCCESS;
            break;
        case 't':
            //print_tree(root);
            print_tree();
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
