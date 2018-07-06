#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include "bmp_utils.h"
#include "cypher.h"

extern int CHUNK_LENGTH;
extern int KEY_SIZE_BYTES;
static int decode_flag;

int main(int argc,char* argv[]) {

  int c;
  char *file_name = NULL;
  char *message = NULL;
  MSG* msg = (MSG*)malloc(sizeof(MSG));
  char *key_file_name = NULL;
  char* key;

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

  if(file_name)
  {
    if(!decode_flag){
      int len = strlen(message);

      /**
      * Si une clé est fournie
      * on encode le message selon le CBC
      * le chiffrement est réalisé avec une permutation de clé et un XOR
      */
      if(key_file_name){

        char xor_vector[CHUNK_LENGTH];
        char chunk[CHUNK_LENGTH+1];
        char encoded_chunk[CHUNK_LENGTH+1];

        key = set_key(key_file_name);
        memcpy(xor_vector, key, CHUNK_LENGTH);
        char* permuted_key = (char*)malloc(KEY_SIZE_BYTES);
        permutation(key, permuted_key, 0);

        int i;
        for(i = 0; i < len/CHUNK_LENGTH; i++){
          memcpy(chunk, message+CHUNK_LENGTH*i, CHUNK_LENGTH);
          msg_enc(chunk, xor_vector, permuted_key, encoded_chunk);
          memcpy(message+CHUNK_LENGTH*i, encoded_chunk, CHUNK_LENGTH);
        }
      }

      BITMAPINFOHEADER bitmapInfoHeader;
      msg->len = len;
      msg->message = message;
      msg = manipBitmapFile(file_name, &bitmapInfoHeader, msg, decode_flag);
      printf("message insere=%s\n", msg->message);
    }

    if(decode_flag){
      BITMAPINFOHEADER bitmapInfoHeader;
      msg->len = -1; // est redéfini par manipBitmapFile
      msg->message = message;
      msg = manipBitmapFile(file_name, &bitmapInfoHeader, msg, decode_flag);
      message = msg->message;
      int len = msg->len;

      /**
      * Si une clé est fournie
      * on décode le message selon le CBC
      * le chiffrement est réalisé avec une permutation de clé et un XOR
      */
      if(key_file_name){
        char xor_vector[CHUNK_LENGTH];
        char chunk[CHUNK_LENGTH];
        char decoded_chunk[CHUNK_LENGTH];

        key = set_key(key_file_name);
        memcpy(xor_vector, key, CHUNK_LENGTH);

        char* permuted_key = (char*)malloc(KEY_SIZE_BYTES);
        permutation(key, permuted_key, 0);

        int i;
        for(i = 0; i < len/CHUNK_LENGTH; i++){
          memcpy(chunk, message+CHUNK_LENGTH*i, CHUNK_LENGTH);
          msg_dec(chunk, xor_vector, permuted_key, decoded_chunk);
          memcpy(message+CHUNK_LENGTH*i, decoded_chunk, CHUNK_LENGTH);
        }
      }
      printf("message recupere=%s\n", message);
    }

  }

  free(file_name);
  free(key);
  free(key_file_name);
  free(msg);
  exit (0);
} //main
