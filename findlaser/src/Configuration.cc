#include "Configuration.h"
#include <string>
#include <fstream>
#include <iostream>
#include <cstdlib>

#include <GeometryCorrection.h>

using namespace std;


namespace FindLaser {

  const unsigned int Configuration::fgCameraBrightnessMin    = 0;
  const unsigned int Configuration::fgCameraBrightnessMax    = 65535;
  const unsigned int Configuration::fgCameraContrastMin      = 0;
  const unsigned int Configuration::fgCameraContrastMax      = 65535;
  const unsigned int Configuration::fgBrightnessThresholdMin = 0;
  const unsigned int Configuration::fgBrightnessThresholdMax = 255;
  const unsigned int Configuration::fgColorThresholdMin      = 0;
  const unsigned int Configuration::fgColorThresholdMax      = 255;

  Configuration::Configuration(const string& fileName)
    : fCameraBrightness(32000), fCameraContrast(32000),
      fBrightnessThreshold(120), fColorThreshold(120),
      fCameraImageSizeX(320), fCameraImageSizeY(240),
      fGeometryCorrection(NULL)
  {
    SetFileName(fileName);
    ReadConfiguration();
  }

  Configuration::~Configuration()
  {
    if (fGeometryCorrection != NULL)
      delete fGeometryCorrection;
  }

  bool Configuration::ReadConfiguration() {
    const string& fileName = GetFileName();

    ifstream fileHandle;
    fileHandle.open(fileName.c_str());
    if ( !fileHandle.is_open() ) {
      cerr << " Could not read configuration file "
           << fileName << endl;
      return false;
    }

    string value;

    if (!fileHandle.good()) return false;
    fileHandle >> value;
    SetCameraImageSizeX((unsigned int)atoi(value.c_str()));

    if (!fileHandle.good()) return false;
    fileHandle >> value;
    SetCameraImageSizeY((unsigned int)atoi(value.c_str()));

    if (!fileHandle.good()) return false;
    fileHandle >> value;
    SetCameraBrightness(atoi(value.c_str()));

    if (!fileHandle.good()) return false;
    fileHandle >> value;
    SetCameraContrast(atoi(value.c_str()));

    if (!fileHandle.good()) return false;
    fileHandle >> value;
    SetBrightnessThreshold(atof(value.c_str()));

    if (!fileHandle.good()) return false;
    fileHandle >> value;
    SetColorThreshold(atof(value.c_str()));

    if (!fileHandle.good()) return false;
    fileHandle >> value;
    SetCameraDevice(value);

    double targetWidth = 0.;
    double targetHeight = 0.;
    if (!fileHandle.good()) return false;
    fileHandle >> targetWidth;
    if (!fileHandle.good()) return false;
    fileHandle >> targetHeight;

    GeometryCorrection* g = new GeometryCorrection();
    g->SetSourceImageDimensions(GetCameraImageSizeX(), GetCameraImageSizeY());
    g->SetTargetImageDimensions(targetWidth, targetHeight);

    // init the corner coordinates
    float xCoord = 0.;
    float yCoord = 0.;
    for (unsigned int i = 0; i < 4; ++i) {
      if (!fileHandle.good()) { delete g; return false; }
      fileHandle >> xCoord;
      if (!fileHandle.good()) { delete g; return false; }
      fileHandle >> yCoord;

      switch (i) {
        case 0:
         g->SetImageUpperLeft(xCoord, yCoord);
         break;
        case 1:
         g->SetImageUpperRight(xCoord, yCoord);
         break;
        case 2:
         g->SetImageLowerRight(xCoord, yCoord);
         break;
        case 3:
         g->SetImageLowerLeft(xCoord, yCoord);
         break;
        default:
         delete g;
         return false;
      };
    }
    fGeometryCorrection = g;

    return true;
  }

  bool Configuration::WriteConfiguration() {
    const string& fileName = GetFileName();
    ofstream fileHandle;
    fileHandle.open(fileName.c_str());
    if ( !fileHandle.is_open() ) {
      cerr << " Could not open configuration file "
           << fileName << " for writing." <<endl;
      return false;
    }

    vector<float> corners;
    fGeometryCorrection->GetImageCornersClockwise(corners);

    fileHandle  << GetCameraImageSizeX()    << "\t" << GetCameraImageSizeY()    << "\n"
                << GetCameraBrightness()    << "\n"
                << GetCameraContrast()      << "\n"
                << GetBrightnessThreshold() << "\n"
                << GetColorThreshold()      << "\n"
                << GetCameraDevice()        << "\n"
                << fGeometryCorrection->GetTargetImageWidth()  << "\t" << fGeometryCorrection->GetTargetImageHeight()  << "\n";
    for (unsigned int i = 0; i < corners.size(); i+=2)
      fileHandle << corners[i] << "\t" << corners[i+1] << "\n";
    fileHandle << endl;

    return true;
  }

  void Configuration::SetGeometryCorrection(GeometryCorrection* g) {
    if (fGeometryCorrection)
      delete fGeometryCorrection;
    fGeometryCorrection = g;
  }

  void Configuration::SetCameraBrightness(const int camBr) {
    if (camBr < fgCameraBrightnessMin)
      fCameraBrightness = fgCameraBrightnessMin;
    else if (camBr > fgCameraBrightnessMax)
      fCameraBrightness = fgCameraBrightnessMax;
    else
      fCameraBrightness = camBr;
  }

  void Configuration::SetCameraContrast(const int camCo) {
    if (camCo < fgCameraContrastMin)
      fCameraContrast = fgCameraContrastMin;
    else if (camCo > fgCameraContrastMax)
      fCameraContrast = fgCameraContrastMax;
    else
      fCameraContrast = camCo;
  }

  void Configuration::SetBrightnessThreshold(const double briThr) {
    if (briThr < fgBrightnessThresholdMin)
      fBrightnessThreshold = fgBrightnessThresholdMin;
    else if (briThr > fgBrightnessThresholdMax)
      fBrightnessThreshold = fgBrightnessThresholdMax;
    else
      fBrightnessThreshold = briThr;
  }

  void Configuration::SetColorThreshold(const double colThr) {
    if (colThr < fgColorThresholdMin)
      fColorThreshold = fgColorThresholdMin;
    else if (colThr > fgColorThresholdMax)
      fColorThreshold = fgColorThresholdMax;
    else
      fColorThreshold = colThr;
  }

} // end namespace FindLaser 

