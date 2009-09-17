#include "ImageCapture.h"

#include <string>
#include <iostream>
#include <sstream>

using std::string;
using std::cout;
using std::cerr;
using std::endl;

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <stdlib.h>

namespace FindLaser {
  ImageCapture::ImageCapture(string device)
    : fDevice(device), fError(""), fInitialized(false), fFd(-1), fBuffer(NULL), fVerbosity(0)
  {
  }

  ImageCapture::~ImageCapture() {
    if (fInitialized)
      close(fFd);
    if (fBuffer != NULL) {
      free(fBuffer);
      fBuffer = NULL;
    }
  }

  bool ImageCapture::Initialize() {
    fFd = open(fDevice.c_str(), O_RDWR); // investigate O_NONBLOCK
    if (fFd < 0) {
      fError = string("Could not open device ") + fDevice;
      return false;
    }

    if (!GetCapability()) return false;
    if (!GetWindow()) return false;
    if (!GetVideoPicture()) return false;
    
    if (fCapability.type & VID_TYPE_MONOCHROME) {
      fError = "Seems to be a monochrome camera. We don't support those at the moment.";
      return false;
    }

    fVPicture.depth=24;
    fVPicture.palette=VIDEO_PALETTE_RGB24;

    if (!SetVideoPicture()) {
      std::ostringstream o;
      o << "Setting video picture to RGB24 mode failed.";
      if (!fError.length() == 0)
	o << "Reason: '" << fError << "'";
      fError = o.str();
      return false;
    }

    fInitialized = true;
    return true;
  }

  bool ImageCapture::GetCapability() {
    if (ioctl(fFd, VIDIOC_QUERYCAP, &fCapability) != 0) {
      fError = string("Could not get device capability struct via VIDIOGCAP. Is")
               + fDevice + string(" not a V4L2 device?");
      return false;
    }
    return true;
  }

  bool ImageCapture::GetWindow() {
    if (ioctl(fFd, VIDIOCGWIN, &fWindow) < 0) {
      fError = "Could not get device window struct via VIDIOCGWIN.";
      return false;
    }
    return true;
  }

  bool ImageCapture::GetVideoPicture() {
    if (ioctl(fFd, VIDIOCGPICT, &fVPicture) < 0) {
      fError = "Could not get video picture struct via VIDIOCGPICT.";
      return false;
    }
    return true;
  }

  bool ImageCapture::SetWindow() {
    if (ioctl(fFd, VIDIOCSWIN, &fWindow) < 0) {
      fError = "Could not set device window struct via VIDIOCSWIN.";
      return false;
    }
    return true;
  }

  bool ImageCapture::SetVideoPicture() {
    if (fVerbosity)
      cout << "Setting video picture..." << endl;

    if (ioctl(fFd, VIDIOCSPICT, &fVPicture) < 0) {
      fError = "Could not set video picture struct via VIDIOCSPICT.";
      return false;
    }
    if (fBuffer != NULL) {
      free(fBuffer);
      fBuffer = NULL;
    }
    fBuffer = (unsigned char*) malloc(fWindow.width * fWindow.height * 24/8);
    if (fBuffer == NULL) {
      fError = "Could not allocate memory for image buffer: Out of memory!";
      return false;
    }

    if (fVerbosity)
      cout << "Successfully set video picture." << endl;

    return true;
  }

  bool ImageCapture::SetImageSize(unsigned int width, unsigned int height) {
    if (!fInitialized) return false;

    if ( width > fCapability.maxwidth || height > fCapability.maxheight ) {
      fError = "width or height exceed maximum device capability";
      return false;
    }
    else if ( width < fCapability.minwidth || height < fCapability.minheight ) {
      fError = "width or height below minimumdevice capability";
      return false;
    }

    fWindow.width  = width;
    fWindow.height = height;
    return SetWindow();
  }

   string ImageCapture::GetError() {
    return fError;
  }

   bool ImageCapture::SetBrightness(unsigned int brightness) {
    fVPicture.brightness = brightness;
    return SetVideoPicture();
  }

   bool ImageCapture::SetContrast(unsigned int contrast) {
    fVPicture.contrast = contrast;
    return SetVideoPicture();
  }


  unsigned int ImageCapture::GetBrightness() {
    return fVPicture.brightness;
  }

  unsigned int ImageCapture::GetContrast() {
    return fVPicture.contrast;
  }

  bool ImageCapture::SetCaptureProperties(unsigned int brightness, unsigned int contrast) {
    fVPicture.brightness = brightness;
    fVPicture.contrast = contrast;
    return SetVideoPicture();
  }

  bool ImageCapture::AdjustBrightness() {
    if (!fInitialized) return false;

    int f = 0;
    unsigned int size = 0;
    do {
      int newbright;
      CaptureImagePointer(size); // writes to fBuffer
      f = GetBrightnessAdjustment(fBuffer, fWindow.width * fWindow.height, &newbright);
      if (f) {
        fVPicture.brightness += (newbright << 8);
        if (!SetVideoPicture()) return false;
        cerr << fVPicture.brightness << endl;
      }
    } while (f);

    return true;
  }

  int ImageCapture::GetBrightnessAdjustment(unsigned char *image, long size, int *brightness) {
    long i, tot = 0;
    for (i=0;i<size*3;i++)
      tot += image[i];
    *brightness = (128 - tot/(size*3))/3;
    return !((tot/(size*3)) >= 122 && (tot/(size*3)) <= 134);
    //return !((tot/(size*3)) >= 126 && (tot/(size*3)) <= 130);
  }

  unsigned char* ImageCapture::CaptureImagePointer(unsigned int& size) {
    if (!fInitialized)
      return NULL;
    size = fWindow.width * fWindow.height * 24/8;

    read(fFd, fBuffer, size);

    return fBuffer;
  }

} // end namespace findlaser
