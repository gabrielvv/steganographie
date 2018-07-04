#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include "bmp_utils.h"
#include "cypher.h"

extern int CHUNK_LENGTH;
extern int KEY_SIZE_BYTES;

static int decode_flag;

void handle_file(char*, char*, int*, void (*functionPtr)(int*, int*, int*, int*));

// https://stackoverflow.com/questions/840501/how-do-function-pointers-in-c-work
void (*encode_decode)(int*, int*, int*, int*);

int main(int argc,char* argv[]) {

  int c;
  char *file_name = NULL;
  char *message = NULL;
  char *key_file_name = NULL;
  int* key;

  while (1)
  {
    static struct option long_options[] =
      {
        {"decode",  no_argument,       &decode_flag, 1},
        {"image",   required_argument, 0, 'i'},
        {"key",     required_argument, 0, 'k'},
        {"message", required_argument, 0, 'm'},
        {0, 0, 0, 0}
      };

    /* getopt_long stores the option index here. */
    int option_index = 0;

    /**
    * optstring is a string containing the legitimate option characters.
    * If such a character is followed by a colon,
    * the option requires an argument
    */
    c = getopt_long (argc, argv, "di:k:m:",
                     long_options, &option_index);

    /* Detect the end of the options. */
    if (c == -1)
      break;

    switch (c)
      {
      case 0:
        /* If this option set a flag, do nothing else now. */
        if (long_options[option_index].flag != 0)
          break;
        printf ("option %s", long_options[option_index].name);
        if (optarg)
          printf (" with arg %s", optarg);
        printf ("\n");
        break;

      case 'd':
        break;

      case 'i':
        file_name = optarg;
        break;

      case 'k':
        key_file_name = optarg;
        break;

      case 'm':
        message = optarg;
        break;

      default:
        printf("unknown argument\n");
        exit (0);
      }
  }

  if(file_name && key_file_name)
  {
    key = set_key(key_file_name);
    handle_file(file_name, message, key, message ? &img_enc : &img_dec);
  }

  free(file_name);
  free(key);
  free(key_file_name);
  free(message);
  exit (0);
} //main

/**
* @param {char*} file_name    - nom de l'image à encoder
* @param {char*} key          - clé de chiffrement symétrique
* @param {void (*encode_decode)(char*, char*, char*, char*)} - fonction de chiffrement/déchiffrement à appliquer
*
*/
void handle_file(
  char *file_name,
  char *message,
  int* key,
  void (*encode_decode)(int*, int*, int*, int*)) {

  int xor_vector[CHUNK_LENGTH];
  memcpy(xor_vector, key, CHUNK_LENGTH);
  int input[CHUNK_LENGTH];
  int output[CHUNK_LENGTH];

  BITMAPINFOHEADER bitmapInfoHeader;
  manipBitmapFile(file_name, &bitmapInfoHeader, message);

  printf("img bits=%d", bitmapInfoHeader.biBitCount);

  // while(1)
  // {
  //   /**
  //   * Reset input buffer
  //   *
  //   */
  //   int j = 0;
  //   for(j; j < CHUNK_LENGTH; j++) {
  //     input[j] = 0;
  //   }
  //
  //   int read;
  //   if( !(read = fread(input, /*byte*/1, KEY_SIZE_BYTES, source)) ){
  //     break;
  //   }
  //
  //   // if(read < KEY_SIZE_BYTES) {
  //   //   padding(input, read, KEY_SIZE_BYTES);
  //   // }
  //   (*encode_decode)(input, xor_vector, key, output);
  //   fwrite(output, /*byte*/1, KEY_SIZE_BYTES, target);
  // }
}
