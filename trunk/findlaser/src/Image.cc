#include "Image.h"

namespace FindLaser {
  Image::Image() : fColumns(0), fImage(0), fRows(0) {
  }

  Image::~Image() {
    ClearImage();
  }

  void Image::ClearImage () {
    unsigned char* img = GetImage();
    if (img != NULL)
      delete img;
    img = NULL;
    SetImage(img);
    SetColumns(0);
    SetRows(0);
  }

  unsigned char* Image::AsPNM(unsigned int& datasize) {
    unsigned char* pnm;
    const unsigned char* img = GetImage();
    const unsigned int cols = GetColumns();
    const unsigned int samples = GetSamples();
    const unsigned int rows = GetRows();

    char header[100];
    if (samples == 3) {
      sprintf(header, "P6\n%u %u\n255\n", GetColumns(), GetRows());
    }
    else {
      sprintf(header, "P5\n%u %u\n255\n", GetColumns(), GetRows());
    }
    const unsigned int headerlength = strlen(header);

    const unsigned int rowsize = cols*samples;
    const unsigned int size = rowsize*rows * sizeof(unsigned char);
    
    pnm = (unsigned char*) malloc( size + headerlength*sizeof(char) );
    strcpy((char*)pnm, header);

    unsigned char* ptr = pnm+headerlength;
    memcpy(ptr, img, size);

    datasize = size + headerlength*sizeof(char);
    return pnm;
  }

} // end namespace FindLaser

