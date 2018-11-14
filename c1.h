typedef struct c0_node
{
    char * key;
    char * value;
    struct c0_node *left,*right;
    int ht;
}c0_node;


void c1_batch_insert(c0_node nodes[], int size);
char * c1_get(char *key);
