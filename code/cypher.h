# ifndef CYPHER
# define CYPHER

char* set_key(char *key_file_name);
void array_xor(char* input, char* vector, char* output);
void padding(char* input, int start_byte, int len_byte);
void permutation(char* input, char* output, int inverse);

void msg_enc(char* input, char* xor_vector, char* key, char* output);
void msg_dec(char* input, char* xor_vector, char* key, char* output);

# endif
