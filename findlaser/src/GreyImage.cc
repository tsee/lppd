
#include "GreyImage.h"
#include "Image.h"
#include "jpegstuff.h"

#include <iostream>
#include <string>

using std::cout;
using std::cerr;
using std::endl;
using std::string;

using namespace FindLaser::JPEGStuff;

namespace FindLaser {

  GreyImage::GreyImage() {}

  GreyImage::~GreyImage() {}

  void GreyImage::SaveAsJPEG(std::string filename, int quality) {
    write_JPEG_file(
      (char*)filename.c_str(), quality, GetRows(), GetColumns(), GetImage(), GetSamples()
    );
  }

  GreyImage* GreyImage::CutOnThreshold(unsigned char threshold, unsigned char replacement) {
    
    const unsigned char* oldimg = GetImage();
    unsigned int rows = GetRows();
    unsigned int cols = GetColumns();

    const unsigned int size = cols*rows*1*sizeof(unsigned char);

    GreyImage* grey = new GreyImage();
    unsigned char* newimg = grey->GetImage();
    newimg = (unsigned char*) malloc(size);

    grey->SetColumns(cols);
    grey->SetRows(rows);

    for (unsigned int i = 0; i < size; i++) {
      const unsigned char val = newimg[i];
      if (val < threshold) newimg[i] = replacement;
      else newimg[i] = val;
    }

    grey->SetImage(newimg);

    return grey;
  }

  GreyImage* GreyImage::Difference(GreyImage* img) {
    if (img == NULL)
      return NULL;
    
    unsigned char* thisimg = GetImage();
    const unsigned int rows = GetRows();
    const unsigned int cols = GetColumns();

    const unsigned int sRows = img->GetRows();
    const unsigned int sCols = img->GetColumns();
    unsigned char* thatimg   = img->GetImage();

    if (sRows != rows || sCols != cols)
      return NULL;

    GreyImage* diffI = new GreyImage();
    unsigned char* newimg = diffI->GetImage();
    const unsigned int size = cols*rows*1*sizeof(unsigned char);
    newimg = (unsigned char*) malloc(size);

    diffI->SetColumns(cols);
    diffI->SetRows(rows);

    for (unsigned int i = 0; i < size; i++) {
      const short diff = ((short) thisimg[i]) - ((short) thatimg[i]);
      newimg[i] = (unsigned char) (diff < 0 ? -diff : diff);
    }

    diffI->SetImage(newimg);

    return diffI;
  }

} // end namespace FindLaser

