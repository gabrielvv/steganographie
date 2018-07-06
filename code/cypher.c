#include "cypher.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

const int KEY_SIZE_BYTES = 8; // la clé doit faire 64 bits
const int CHUNK_LENGTH = 8 / sizeof(char);

/**
* @param {char*} key_file_name - fichier stockant la clé
*
*/
char* set_key(char *key_file_name){
  FILE *key_file;
  char* key = malloc(KEY_SIZE_BYTES);

  key_file = fopen(key_file_name, "rb");
  int read;
  if( !(read = fread( key, /*byte*/1, KEY_SIZE_BYTES, key_file)) ){
    printf("trouble with key read=%d\n", read);
    exit(0);
  }

  if(read < KEY_SIZE_BYTES){
    padding(key, read, KEY_SIZE_BYTES);
  }

  fclose(key_file);
  return key;
}

/**
* @param {char*} output    - chaine reçevant le résultat du xor
* @param {char*} input - chaine sur laquelle appliquer le xor
* @param {char*} vector    - vecteur pour le xor
*/
void array_xor(char* input, char* vector, char* output){

  int i;
  for(i=0; i < CHUNK_LENGTH; ++i) {
    output[i] = (char)(input[i] ^ vector[i]);
  }
}

/**
* @desc Application du Cipher block chaining avec algo de chiffrement personnalisé
*
* @param {char*} output     - chaine reçevant le résultat du chiffrement
* @param {char*} xor_vector - vecteur pour le xor
* @param {char*} input      - chaine sur laquelle appliquer le chiffrement
* @param {char*} key        - clé de chiffrement
*
*/
void msg_enc(char* input, char* xor_vector, char* key, char* output){
  array_xor(input, xor_vector, output);

  int i;
  for(i=0; i < CHUNK_LENGTH; ++i) {
    output[i] = output[i] ^ key[i];
  }

  memcpy(xor_vector, output, CHUNK_LENGTH);
}

/**
* @desc Application du Cipher block chaining avec algo de déchiffrement personnalisé
*
* @param {char*} output  - chaine reçevant le résultat du déchiffrement
* @param {char*} xor_vector
* @param {char*} input  - chaine sur laquelle appliquer le déchiffrement
* @param {char*} key    - clé de chiffrement/déchiffrement
*
*/
void msg_dec(char* input, char* xor_vector, char* key, char* output){
  memset(output, 0, CHUNK_LENGTH);

  int i;
  for(i=0; i < CHUNK_LENGTH; ++i) {
    output[i] = input[i] ^ key[i];
  }

  array_xor(output, xor_vector, output);
  memcpy( xor_vector, input, CHUNK_LENGTH );
}

void padding(char* input, int start_byte, int len_byte){
  (*input) = (*input) << (8*(len_byte-start_byte));
}

/**
*
* @param {int*} input - à permuter
* @param {int*} output - input permutée
* @param {int} inverse - s'il faut appliquer la permutation inverse
*/
void permutation(char* input, char* output, int inverse){
  memset(output, 0, CHUNK_LENGTH);
  /**
  * Table de permutation initial
  * d'un bloc, issu de DES
  *
  */
  static int permutation_table[] = {
    58, 50, 42, 34, 26, 18, 10, 2,
    60, 52, 44, 36, 28, 20, 12, 4,
    62, 54, 46, 38, 30, 22, 14, 6,
    64, 56, 48, 40, 32, 24, 16, 8,
    57, 49, 41, 33, 25, 17, 9, 1,
    59, 51, 43, 35, 27, 19, 11, 3,
    61, 53, 45, 37, 29, 21, 13, 5,
    63, 55, 47, 39, 31, 23, 15, 7
  };

  int n;

  /**
  *
  * https://stackoverflow.com/questions/47981/how-do-you-set-clear-and-toggle-a-single-bit
  */
  for(n = 0; n < KEY_SIZE_BYTES*8; n++){
    int nth = inverse ? permutation_table[n] : n;
    char x = ((*input) & ( 1 << nth )) >> nth; // get the n-th bit of key

    int output_pos = inverse ? n : permutation_table[n];
    // set the n-th bit of permuted key with permuted bit
    (*output) ^= (-x ^ (*output)) & (1UL << output_pos);
  }
}
