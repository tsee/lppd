#include "settings.h"
#include <string>
#include <fstream>
#include <iostream>

using namespace std;

namespace FindLaser {

  Configuration::Configuration(const string& fileName)
    : fCameraBrightness(32000), fCameraContrast(32000),
      fBrightnessThreshold(120), fColorThreshold(120)
  {
    SetFileName(fileName);
    ReadConfiguration();
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

    string value;

    fileHandle  << GetCameraImageSizeX()    << "\n"
                << GetCameraImageSizeY()    << "\n"
                << GetCameraBrightness()    << "\n"
                << GetCameraContrast()      << "\n"
                << GetBrightnessThreshold() << "\n"
                << GetColorThreshold()      << "\n"
                << GetCameraDevice()        << "\n"
    << endl;

    return true;
  }

} // end namespace FindLaser 

