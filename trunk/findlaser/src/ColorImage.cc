#include "ColorImage.h"
#include "Image.h"
#include "GreyImage.h"
#include "Color.h"

#include "jpegstuff.h"

#include <iostream>
#include <string>
#include <cmath>

using std::cout;
using std::cerr;
using std::endl;
using std::string;

using namespace FindLaser::JPEGStuff;

namespace FindLaser {

  ColorImage::ColorImage() {
  }

  ColorImage::~ColorImage() {}


  bool ColorImage::ReverseCopyFromMemory (unsigned int width, unsigned int height, unsigned char* ptr) {
    if (ptr == NULL) return false;
    ClearImage();
    SetColumns(width);
    SetRows(height);
    unsigned char* img = GetImage();

    const unsigned int rowsize = width * GetSamples() * sizeof(unsigned char);
    const unsigned int size = height * rowsize;
    img = (unsigned char*) malloc(size);

    cout << width << " " << height << " " << GetSamples() << endl;

    for (unsigned int i = 0; i < size; i+=3) {
      img[i]   = ptr[i+2];
      img[i+1] = ptr[i+1];
      img[i+2] = ptr[i];
    }

    SetImage(img);
    return true;
  }

  bool ColorImage::ReadFromFile (const string filename) {
    ClearImage();
    int samples, columns, rows;
    unsigned char* img = GetImage();
    int result = read_JPEG_file((char*)filename.c_str(), img, samples, columns, rows);
    if (result == 0) {
      ClearImage();
      return false;
    }
    SetImage(img);
    SetRows(rows);
    SetColumns(columns);
    return true;
  }


  GreyImage* ColorImage::ColorProjection(const Color& color) {
    GreyImage* grey = new GreyImage();
    unsigned char* greyData = grey->GetImage();
    unsigned char* colorData = GetImage();

    double projRed   = color.red / 255.;
    double projGreen = color.green / 255.;
    double projBlue  = color.blue / 255.;

    int cols = GetColumns();
    int rows = GetRows();
    const unsigned int greySize = cols*rows*1*sizeof(unsigned char);
    const unsigned int colorSize = greySize*GetSamples();

    greyData = (unsigned char*) malloc(greySize);

    for (unsigned int i = 0; i < greySize; i++) {
      const unsigned int ci = i * 3;
      greyData[i] = (unsigned char) (   projRed * colorData[ci+0]
                                       + projGreen * colorData[ci+1]
                                       + projBlue * colorData[ci+2]  );
    }

    grey->SetColumns(cols);
    grey->SetRows(rows);
    grey->SetImage(greyData);

    return grey;
  }


  inline GreyImage* ColorImage::RedProjection()   { return FixedColorProjection(0); }
  inline GreyImage* ColorImage::GreenProjection() { return FixedColorProjection(1); }
  inline GreyImage* ColorImage::BlueProjection()  { return FixedColorProjection(2); }


  GreyImage* ColorImage::FixedColorProjection(unsigned int colorOffset) {
    GreyImage* grey = new GreyImage();
    unsigned char* greyData = grey->GetImage();
    unsigned char* colorData = GetImage();

    int cols = GetColumns();
    int rows = GetRows();

    const unsigned int greySize = cols*rows*1*sizeof(unsigned char);
    const unsigned int colorSize = greySize*GetSamples();
    greyData = (unsigned char*) malloc(greySize);

    for (unsigned int i = 0; i < greySize; i++) {
      const unsigned int ci = i * 3;
      greyData[i] =  colorData[ci + colorOffset];
    }

    grey->SetColumns(cols);
    grey->SetRows(rows);
    grey->SetImage(greyData);

    return grey;
  }


  GreyImage* ColorImage::NormalizedColorProjection(const Color& color, const unsigned char colorThreshold, const unsigned char colorDefault) {
    GreyImage* grey = new GreyImage();
    unsigned char* greyData = grey->GetImage();
    unsigned char* colorData = GetImage();

    const double projRed   = color.red / 255.;
    const double projGreen = color.green / 255.;
    const double projBlue  = color.blue / 255.;
    
    const double projLen = pow( projRed*projRed + projGreen*projGreen + projBlue*projBlue, 0.5 );

    const double normProjRed = projRed/projLen;
    const double normProjGreen = projGreen/projLen;
    const double normProjBlue = projBlue/projLen;

    int cols = GetColumns();
    int rows = GetRows();

    const unsigned int greySize = cols*rows*1*sizeof(unsigned char);
    const unsigned int colorSize = greySize*GetSamples();

    greyData = (unsigned char*) malloc(greySize);

    for (unsigned int i = 0; i < greySize; i++) {
      const unsigned int ci = i * 3;
      const unsigned int r = colorData[ci+0];
      const unsigned int g = colorData[ci+1];
      const unsigned int b = colorData[ci+2];

      const double colorLength = pow((double)(r*r+g*g+b*b), 0.5);
        
      greyData[i] = (unsigned char) ( 255 * (  normProjRed   * r / colorLength
                                             + normProjGreen * g / colorLength
                                             + normProjBlue  * b / colorLength ) );
      if (greyData[i] < colorThreshold)
        greyData[i] = colorDefault;
    }

    grey->SetColumns(cols);
    grey->SetRows(rows);
    grey->SetImage(greyData);

    return grey;
  }


  GreyImage* ColorImage::ToGreyscale(const unsigned char brightnessThreshold, const unsigned char brightnessDefault) {
    GreyImage* grey = new GreyImage();
    unsigned char* greyData = grey->GetImage();
    unsigned char* colorData = GetImage();

    const int cols = GetColumns();
    const int rows = GetRows();

    const unsigned int greySize = cols*rows*1*sizeof(unsigned char);
    const unsigned int colorSize = greySize*GetSamples();

    greyData = (unsigned char*) malloc(greySize);

    for (unsigned int i = 0; i < greySize; i++) {
      const unsigned int ci = i * 3;
      const unsigned int r = colorData[ci+0];
      const unsigned int g = colorData[ci+1];
      const unsigned int b = colorData[ci+2];
      greyData[i] = (unsigned char) (0.301*(float)r + 0.586*(float)g + 0.113*(float)b);
      if (greyData[i] < brightnessThreshold)
        greyData[i] = brightnessDefault; 
    }

    grey->SetColumns(cols);
    grey->SetRows(rows);
    grey->SetImage(greyData);

    return grey;
  }


  void ColorImage::SaveAsJPEG(std::string filename, int quality) {
    write_JPEG_file(
      (char*)filename.c_str(), quality, GetRows(), GetColumns(), GetImage(), GetSamples()
    );
  }

  void ColorImage::FindLaserCentroid(
    double& centroidx, double& centroidy, const Color& color, 
    const double colorThreshold, const double lightThreshold
  ) {
    centroidx = -1.;
    centroidy = -1.;

    unsigned char* colorData = GetImage();

    const float projRed   = color.red / 255.;
    const float projGreen = color.green / 255.;
    const float projBlue  = color.blue / 255.;
    
    const float projLen = pow( projRed*projRed + projGreen*projGreen + projBlue*projBlue, 0.5 );

    const float normProjRed = projRed/projLen*255;
    const float normProjGreen = projGreen/projLen*255;
    const float normProjBlue = projBlue/projLen*255;

    const unsigned int lightThresholdSquared = (unsigned int) lightThreshold * (unsigned int) lightThreshold;

    const int cols = GetColumns();
    const int rows = GetRows();

    const unsigned int rowSize = cols*GetSamples();

    float cx = 0, cy = 0;
    float cw = 0;

    for (unsigned int row = 0; row < rows; row++) {
      const unsigned int firstOnRow = row*rowSize;

      for (unsigned int c = 0; c < cols; c++) {
        const unsigned int firstIndex = firstOnRow + c*3;
        const unsigned int r = colorData[firstIndex+0];
        const unsigned int g = colorData[firstIndex+1];
        const unsigned int b = colorData[firstIndex+2];
        
        const unsigned int colsquares = r*r+g*g+b*b;
  //      const float grey = (1./3.)*(float)(r+g+b);
        if (colsquares/3 > lightThresholdSquared) {
          const float colorLength = pow((float)(colsquares), 0.5);
          const float colorProjection =  (   normProjRed   * r / colorLength
                                           + normProjGreen * g / colorLength
                                           + normProjBlue  * b / colorLength );
          if (colorProjection >= colorThreshold) {
            const float w = colorProjection*colorProjection;
            cw += w;
            cx += w*c;
            cy += w*row;
          }
        }
      } // end for cols
    } // end for rows

    if (cw > 0) {
      centroidx = cx / cw;
      centroidy = cy / cw;
    }
  }

  void ColorImage::PtrFindLaserCentroid(
    const unsigned char* ptr,  const unsigned int width, const unsigned int height,
    bool reverse,
    double& centroidx, double& centroidy, const Color& color, 
    const double colorThreshold, const double lightThreshold
  ) {
    centroidx = -1.;
    centroidy = -1.;

    const float projRed   = color.red / 255.;
    const float projGreen = color.green / 255.;
    const float projBlue  = color.blue / 255.;
    
    const float projLen = pow( projRed*projRed + projGreen*projGreen + projBlue*projBlue, 0.5 );

    const float normProjRed = projRed/projLen*255;
    const float normProjGreen = projGreen/projLen*255;
    const float normProjBlue = projBlue/projLen*255;

    const unsigned int lightThresholdSquared = (unsigned int) lightThreshold * (unsigned int) lightThreshold;

    const unsigned int rowSize = width*GetSamples();

    float cx = 0, cy = 0;
    float cw = 0;

    const unsigned int redOffset   = (reverse ? 2 : 0);
    const unsigned int greenOffset = 1;
    const unsigned int blueOffset  = (reverse ? 0 : 2);

    for (unsigned int row = 0; row < height; row++) {
      const unsigned int firstOnRow = row*rowSize;

      for (unsigned int c = 0; c < width; c++) {
        const unsigned int firstIndex = firstOnRow + c*3;
        const unsigned int r = ptr[firstIndex+redOffset];
        const unsigned int g = ptr[firstIndex+greenOffset];
        const unsigned int b = ptr[firstIndex+blueOffset];
        
        const unsigned int colsquares = r*r+g*g+b*b;
  //      const float grey = (1./3.)*(float)(r+g+b);
        if (colsquares/3 > lightThresholdSquared) {
          const float colorLength = pow((float)(colsquares), 0.5);
          const float colorProjection =  (   normProjRed   * r / colorLength
                                           + normProjGreen * g / colorLength
                                           + normProjBlue  * b / colorLength );
          if (colorProjection >= colorThreshold) {
            const float w = colorProjection*colorProjection;
            cw += w;
            cx += w*c;
            cy += w*row;
          }
        }
      } // end for cols
    } // end for rows 

    if (cw > 0) {
      centroidx = cx / cw;
      centroidy = cy / cw;
    }
  }

} // end namespace FindLaserCentroid

