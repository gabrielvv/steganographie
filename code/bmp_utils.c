#include "bmp_utils.h"
#include <stdlib.h>
#include <stdio.h>
#include "string.h"

/**
* https://stackoverflow.com/questions/14279242/read-bitmap-file-into-structure
*
* Actions disponibles
* - effacement message si message = NULL et decode_flag  = 0
* - ecriture du message si message != NULL et decode_flag = 0
* - lecture du message si decode_flag = 1
*
*/
unsigned char* manipBitmapFile(char *filename, BITMAPINFOHEADER *bitmapInfoHeader, char* message, int decode_flag)
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
      message = process8(filePtr, &bitmapFileHeader, message, imgSize, decode_flag);
    } else if(bitmapInfoHeader->biBitCount == 24) {
      message = process24(filePtr, &bitmapFileHeader, message, imgSize, decode_flag);
    }

    //close file and return bitmap iamge data
    fclose(filePtr);
    return message;
}

unsigned char* process8(
  FILE* filePtr,
  BITMAPFILEHEADER* bitmapFileHeader,
  char* message,
  int imgSize,
  int decode_flag){

  int colorNbr = 256;
  int paletteIdx=0;  //image index counter

  // Si on ne fournit pas de message cela correspond à un effacement du message
  if(!message && !decode_flag){
    printf("creating empty message\n");
    message = (char*)malloc(colorNbr+1);
    for(paletteIdx = 0; paletteIdx < colorNbr; paletteIdx++){
      message[paletteIdx] = 1;
    }
    message[paletteIdx] = '\0';
  }

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
    int msgLen = strlen(message) + 1;
    printf("encoding message of length=%d\n", msgLen);
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

    int msgLen = bitmapPalette[0].rgbReserved;

    message = (char*) malloc(msgLen+1);
    for (paletteIdx = 1; paletteIdx < msgLen+1; paletteIdx++)
    {
      message[paletteIdx-1] = bitmapPalette[paletteIdx].rgbReserved;
    }
    message[paletteIdx] = '\0';

  }

  free(bitmapPalette);
  return message;
}

unsigned char* process24(
  FILE* filePtr,
  BITMAPFILEHEADER* bitmapFileHeader,
  char* message,
  int imgSize,
  int decode_flag){

  printf("process24\n");

  unsigned char* bitmap;

  // Place disponible en octets pour encoder le message
  int availableBytes = (imgSize*3)/8;

  if(!decode_flag){
    printf("encoding\n");

    // +1 byte pour coder la longeur du message
    int len = strlen(message) + 1;
    if(availableBytes < len){
      len = availableBytes;
      printf("Pas assez de place disponible, le message sera tronqué\n");
    }

    printf("len=%d\n", len);

    int bitmapLen = len*8;
    bitmap = (unsigned char*)malloc(bitmapLen);
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

    unsigned char c = len-1;
    printf("c=%d\n", c);
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

    printf("decoding\n");

    /******* On décode la longueur du message ********/

    bitmap = (unsigned char*)malloc(8);
    fread(bitmap, 8, 1, filePtr);

    int len = 0;
    int x, nth, i;
    for(nth = 0; nth < 8; nth++) {
      x = (bitmap[nth] & ( 1 << 0 )) >> 0; // get the n-th bit of input
      len ^= (-x ^ len) & (1UL << nth);
    }
    printf("longueur de message decodee=%d\n", len);
    free(bitmap);

    /***********************************************/

    message = (unsigned char*) malloc(len+1);
    message[len] = '\0';

    int bmpLen = len*8;
    bitmap = (unsigned char*)malloc(bmpLen);
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
      unsigned char c = '\0';
      for(nth = 0; nth < 8; nth++) {
        x = (bitmap[nth+i*8] & ( 1 << 0 )) >> 0; // get the n-th bit of input
        c ^= (-x ^ c) & (1UL << nth);
      }
      message[i] = c;
    }
  }

  free(bitmap);
  fclose(filePtr);
  return message;
}
