#include "bmp_utils.h"
#include <stdlib.h>
#include <stdio.h>
#include "string.h"

/**
* https://stackoverflow.com/questions/14279242/read-bitmap-file-into-structure
*
* Actions disponibles
* - ecriture du message si message != NULL et decode_flag = 0
* - lecture du message si decode_flag = 1
*
* @param {char*} filename - nom du fichier bitmap cible
* @param {BITMAPINFOHEADER*} bitmapInfoHeader
* @param {MSG*} msg
* @param {int} decode_flag
*
*
*/
MSG* manipBitmapFile(char *filename, BITMAPINFOHEADER *bitmapInfoHeader, MSG* msg, int decode_flag)
{

    FILE *filePtr; //our file pointer
    BITMAPFILEHEADER bitmapFileHeader; //our bitmap file header

    //open filename in read binary mode
    filePtr = fopen(filename,"r+b");
    if (filePtr == NULL){
      printf("ne peut pas ouvrir le fichier\n");
      return NULL;
    }

    //read the bitmap file header
    fread(&bitmapFileHeader, sizeof(BITMAPFILEHEADER),1,filePtr);

    //verify that this is a bmp file by check bitmap id
    if (bitmapFileHeader.bfType !=0x4D42)
    {
        printf("n'est pas un fichier bmp\n");
        fclose(filePtr);
        return NULL;
    }

    //read the bitmap info header
    fread(bitmapInfoHeader, sizeof(BITMAPINFOHEADER),1,filePtr);

    printf("biBitCount=%d\n", bitmapInfoHeader->biBitCount);
    printf("biClrUsed=%d\n", bitmapInfoHeader->biClrUsed);

    //move file point to the begging of bitmap data
    fseek(filePtr, bitmapFileHeader.bfOffBits, SEEK_SET);

    int imgSize = bitmapInfoHeader->biSizeImage
    ? bitmapInfoHeader->biSizeImage
    : bitmapInfoHeader->biWidth * bitmapInfoHeader->biHeight;

    if(bitmapInfoHeader->biBitCount == 8) {
      msg = process8(filePtr, &bitmapFileHeader, msg, imgSize, decode_flag);
    } else if(bitmapInfoHeader->biBitCount == 24) {
      msg = process24(filePtr, &bitmapFileHeader, msg, imgSize, decode_flag);
    }

    //close file and return bitmap iamge data
    fclose(filePtr);
    // printf("manipBitmapFile output message=%s len=%d\n", msg->message, msg->len);
    return msg;
}

/**
* @desc Traite les fichiers bmp 8 bits (avec palette de couleurs)
*
* @param {FILE*} filePtr - fichier bitmap cible
* @param {BITMAPFILEHEADER*} bitmapFileHeader
* @param {MSG*} msg
* @param {int} imgSize
* @param {int} decode_flag
*
*/
MSG* process8(
  FILE* filePtr,
  BITMAPFILEHEADER* bitmapFileHeader,
  MSG* msg,
  int imgSize,
  int decode_flag){

  int colorNbr = 256;
  int paletteIdx=0;  //image index counter
  char* message = msg->message;
  int msgLen = msg->len;

  // printf("process8 input message=%s len=%d\n", message, msgLen);

  RGBQUAD* bitmapPalette;  //store image color palette

  //allocate enough memory for the bitmap image data
  bitmapPalette = (RGBQUAD*)malloc(colorNbr*sizeof(RGBQUAD));

  /** lecture de la palette
  *
  * A ce moment là le curseur est placé au niveau de la donnée utile
  *
  */
  fread(bitmapPalette,colorNbr*sizeof(RGBQUAD),1,filePtr);

  //verify memory allocation
  if (!bitmapPalette)
  {
    free(bitmapPalette);
    fclose(filePtr);
    return NULL;
  }

  //make sure bitmap image data was read
  if (bitmapPalette == NULL)
  {
    free(bitmapPalette);
    fclose(filePtr);
    return NULL;
  }

  if(!decode_flag) {

    // On ajoute un byte pour stocker la longueur du message
    msgLen = msgLen + 1;
    if(colorNbr < msgLen){
      printf("Pas assez de place disponible, le message sera tronqué\n");
      msgLen = colorNbr;
    }

    bitmapPalette[paletteIdx].rgbReserved = msgLen-1;
    for (paletteIdx = 1; paletteIdx < msgLen; paletteIdx++)
    {
      bitmapPalette[paletteIdx].rgbReserved = message[paletteIdx-1];
      // printf("assigning %c to %c\n", message[paletteIdx], bitmapPalette[paletteIdx].rgbReserved);
    }
    fseek(filePtr, bitmapFileHeader->bfOffBits, SEEK_SET);
    // printf("after seek\n");
    fwrite(bitmapPalette, colorNbr*sizeof(RGBQUAD), 1, filePtr);
    // printf("after write\n");
  } else {
    msgLen = bitmapPalette[0].rgbReserved;
    message = (char*) malloc(msgLen+1);
    for (paletteIdx = 1; paletteIdx < msgLen+1; paletteIdx++)
    {
      message[paletteIdx-1] = bitmapPalette[paletteIdx].rgbReserved;
    }
    message[paletteIdx] = '\0';

  }

  free(bitmapPalette);
  msg->message = message;
  msg->len = decode_flag ? msgLen : msgLen-1;
  // printf("process8 output message=%s len=%d\n", msg->message, msg->len);
  return msg;
}

/**
* @desc Traite les fichiers bmp 24 bits
*
* @param {FILE*} filePtr - fichier bitmap cible
* @param {BITMAPFILEHEADER*} bitmapFileHeader
* @param {MSG*} msg
* @param {int} imgSize
* @param {int} decode_flag
*
*/
MSG* process24(
  FILE* filePtr,
  BITMAPFILEHEADER* bitmapFileHeader,
  MSG* msg,
  int imgSize,
  int decode_flag){

  printf("process24\n");

  char* bitmap;
  char* message = msg->message;
  int len = msg->len;

  // Place disponible en octets pour encoder le message
  int availableBytes = (imgSize*3)/8;

  if(!decode_flag){
    // +1 byte pour coder la longeur du message
    len = msg->len + 1;
    if(availableBytes < len){
      len = availableBytes;
      printf("Pas assez de place disponible, le message sera tronqué\n");
    }

    int bitmapLen = len*8;
    bitmap = (char*)malloc(bitmapLen);
    fread(bitmap, bitmapLen, 1, filePtr);

    //verify memory allocation
    if (!bitmap)
    {
      printf("verify memory allocation\n");
      free(bitmap);
      fclose(filePtr);
      return NULL;
    }
    //make sure bitmap image data was read
    if (bitmap == NULL)
    {
      printf("make sure bitmap image data was read\n");
      free(bitmap);
      fclose(filePtr);
      return NULL;
    }

    /*** On code la longueur du message dans les 8 premiers octets ***/

    char c = len-1;
    int i, nth, x;
    for(nth = 0; nth < 8; nth++){
      x = (c & ( 1 << nth )) >> nth; // get the n-th bit of input
      bitmap[nth] ^= (-x ^ bitmap[nth]) & (1UL << 0);
    }

    /*********************************/

    for(i = 1; i < len; i++) {
      char c = message[i-1];
      for(nth = 0; nth < 8; nth++){
        x = (c & ( 1 << nth )) >> nth; // get the n-th bit of input
        bitmap[nth+i*8] ^= (-x ^ bitmap[nth+i*8]) & (1UL << 0);
      }
    }
    fseek(filePtr, bitmapFileHeader->bfOffBits, SEEK_SET);
    fwrite(bitmap, bitmapLen, 1, filePtr);

  }else{

    /******* On décode la longueur du message ********/

    bitmap = (char*)malloc(8);
    fread(bitmap, 8, 1, filePtr);

    len = 0;
    int x, nth, i;
    for(nth = 0; nth < 8; nth++) {
      x = (bitmap[nth] & ( 1 << 0 )) >> 0; // get the n-th bit of input
      len ^= (-x ^ len) & (1UL << nth);
    }
    free(bitmap);

    /***********************************************/

    message = (char*) malloc(len+1);
    message[len] = '\0';

    int bmpLen = len*8;
    bitmap = (char*)malloc(bmpLen);
    fread(bitmap, bmpLen, 1, filePtr);

    //verify memory allocation
    if (!bitmap)
    {
      printf("verify memory allocation\n");
      free(bitmap);
      fclose(filePtr);
      return NULL;
    }
    //make sure bitmap image data was read
    if (bitmap == NULL)
    {
      printf("make sure bitmap image data was read\n");
      free(bitmap);
      fclose(filePtr);
      return NULL;
    }

    for(i = 0; i < len; i++) {
      char c = '\0';
      for(nth = 0; nth < 8; nth++) {
        x = (bitmap[nth+i*8] & ( 1 << 0 )) >> 0; // get the n-th bit of input
        c ^= (-x ^ c) & (1UL << nth);
      }
      message[i] = c;
    }
  }

  free(bitmap);
  fclose(filePtr);
  msg->message = message;
  msg->len = len;
  return msg;
}
