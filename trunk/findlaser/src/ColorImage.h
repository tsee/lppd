#ifndef __FindLaser__ColorImage_h
#define __FindLaser__ColorImage_h


#include "Image.h"

#include <string>

namespace FindLaser {
  class GreyImage;
  class Color;

  class ColorImage : public Image {
    friend class GreyImage;
    public:
      ColorImage();
      virtual ~ColorImage();

      bool ReadFromFile (const std::string filename);
      int GetSamples() { return 3; }

      bool ReverseCopyFromMemory (unsigned int width, unsigned int height, unsigned char* ptr);

      GreyImage* ColorProjection(const Color& color);

      // at least twice as fast as the generic ColorProjection
      GreyImage* RedProjection();
      GreyImage* GreenProjection();
      GreyImage* BlueProjection();

      GreyImage* NormalizedColorProjection(const Color& color, const unsigned char colorThreshold = 0, const unsigned char colorDefault = 0);

      GreyImage* ToGreyscale(const unsigned char brightnessThreshold = 0, const unsigned char brightnessDefault = 0);


      void FindLaserCentroid(
        double& centroidx, double& centroidy, const Color& color, 
        const double colorThreshold, const double lightThreshold
      );

      void PtrFindLaserCentroid(
        const unsigned char* ptr, const unsigned int width, const unsigned int height,
        bool reverse,
        double& centroidx, double& centroidy, const Color& color, 
        const double colorThreshold, const double lightThreshold
      );

      void SaveAsJPEG(std::string filename, int quality);

    private:
      GreyImage* FixedColorProjection(unsigned int colorOffset);
  };


} // end namespace FindLaser

#endif

