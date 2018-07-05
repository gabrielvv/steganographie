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
    int colorNbr = 256;
    RGBQUAD* bitmapPalette;  //store image color palette
    int paletteIdx=0;  //image index counter
    unsigned char tempRGB;  //our swap variable

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

    //allocate enough memory for the bitmap image data
    bitmapPalette = (RGBQUAD*)malloc(colorNbr*sizeof(RGBQUAD));

    //verify memory allocation
    if (!bitmapPalette)
    {
      free(bitmapPalette);
      fclose(filePtr);
      return;
    }

    //make sure bitmap image data was read
    if (bitmapPalette == NULL)
    {
        fclose(filePtr);
        return NULL;
    }

    // Si on ne fournit pas de message cela correspond à un effacement du message
    if(!message && !decode_flag){
      printf("creating empty message\n");
      message = (char*)malloc(colorNbr+1);
      for(paletteIdx = 0; paletteIdx < colorNbr; paletteIdx++){
        message[paletteIdx] = 1;
      }
      message[paletteIdx] = '\0';
    }

    if(bitmapInfoHeader->biBitCount == 8) {
      int availableBytes = colorNbr; // nombre de couleurs de la palette
      /** lecture de la palette
      *
      * A ce moment là le curseur est placé au niveau de la donnée utile
      *
      */
      fread(bitmapPalette,colorNbr*sizeof(RGBQUAD),1,filePtr);

      if(!decode_flag) {

        int msgLen = strlen(message);
        printf("encoding message of length=%d\n", msgLen);
        if(availableBytes < msgLen){
          printf("Pas assez de place disponible, le message sera tronqué\n");
          msgLen = availableBytes;
        }

        for (paletteIdx = 0; paletteIdx < msgLen; paletteIdx++)
        {
          bitmapPalette[paletteIdx].rgbReserved = message[paletteIdx];
          // printf("assigning %c to %c\n", message[paletteIdx], bitmapPalette[paletteIdx].rgbReserved);
        }
        fseek(filePtr, bitmapFileHeader.bfOffBits, SEEK_SET);
        fwrite(bitmapPalette, colorNbr*sizeof(RGBQUAD), 1, filePtr);

      } else {

        printf("decoding %d bytes\n", availableBytes);
        message = (char*)malloc(colorNbr+1);
        for (paletteIdx = 0; paletteIdx < colorNbr; paletteIdx++)
        {
          message[paletteIdx] = bitmapPalette[paletteIdx].rgbReserved;
          // printf("assigning %c to %c\n", message[paletteIdx], bitmapPalette[paletteIdx].rgbReserved);
        }
        message[paletteIdx] = '\0';

      }
    } else if(bitmapInfoHeader->biBitCount == 24) {

    }

    //close file and return bitmap iamge data
    fclose(filePtr);
    return message;
}
