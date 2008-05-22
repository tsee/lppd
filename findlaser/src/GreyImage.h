#ifndef __FindLaser__GreyImage_h
#define __FindLaser__GreyImage_h

#include "Image.h"

#include <string>

namespace FindLaser {
  class GreyImage;
  class ColorImage;
  class Color;
  class GreyImage : public Image {
    friend class ColorImage;
    public:
      GreyImage();
      virtual ~GreyImage();
      
      GreyImage* CutOnThreshold(unsigned char threshold, unsigned char replacement);

      void SaveAsJPEG(std::string filename, int quality);
      int GetSamples() { return 1; }
  };
} // end namespace FindLaser

#endif
