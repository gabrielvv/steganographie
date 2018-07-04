# ifndef CYPHER
# define CYPHER

int* set_key(char *key_file_name);
void array_xor(int* input, int* vector, int* output);
void padding(int* input, int start_byte, int len_byte);
void permutation(int* input, int* output, int inverse);

void img_enc(int* input, int* xor_vector, int* key, int* output);
void img_dec(int* input, int* xor_vector, int* key, int* output);

# endif
