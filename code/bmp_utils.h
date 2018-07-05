#ifndef BMP_UTILS
#define BMP_UTILS

#define WORD unsigned short
#define DWORD unsigned int
#define LONG int
#define BYTE char

#include <stdio.h>

#pragma pack(push, 1)

typedef struct tagRGBQUAD {
  BYTE rgbBlue;
  BYTE rgbGreen;
  BYTE rgbRed;
  BYTE rgbReserved;
} RGBQUAD;

#pragma pack(pop)

#pragma pack(push, 1)

typedef struct tagRGBTRI {
  BYTE rgbBlue;
  BYTE rgbGreen;
  BYTE rgbRed;
} RGBTRI;

#pragma pack(pop)

#pragma pack(push, 1)

typedef struct tagBITMAPFILEHEADER
{
  WORD bfType;  //specifies the file type
  DWORD bfSize;  //specifies the size in bytes of the bitmap file
  WORD bfReserved1;  //reserved; must be 0
  WORD bfReserved2;  //reserved; must be 0
  DWORD bfOffBits;  //species the offset in bytes from the bitmapfileheader to the bitmap bits
} BITMAPFILEHEADER;

#pragma pack(pop)

#pragma pack(push, 1)

typedef struct tagBITMAPINFOHEADER
{
  DWORD biSize;  //specifies the number of bytes required by the struct
  LONG biWidth;  //specifies width in pixels
  LONG biHeight;  //species height in pixels
  WORD biPlanes; //specifies the number of color planes, must be 1
  WORD biBitCount; //specifies the number of bit per pixel
  DWORD biCompression;//spcifies the type of compression
  DWORD biSizeImage;  //size of image in bytes
  LONG biXPelsPerMeter;  //number of pixels per meter in x axis
  LONG biYPelsPerMeter;  //number of pixels per meter in y axis
  DWORD biClrUsed;  //number of colors used by th ebitmap
  DWORD biClrImportant;  //number of colors that are important
} BITMAPINFOHEADER;

#pragma pack(pop)

unsigned char* process24(FILE*, BITMAPFILEHEADER*, char*, int, int);
unsigned char* process8(FILE*, BITMAPFILEHEADER*, char*, int, int);
unsigned char* manipBitmapFile(char*, BITMAPINFOHEADER*, char*, int);

# endif
