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
    if (!ResetCrop()) return false;
    if (!GetWindow()) return false;
    if (!GetVideoPicture()) return false;
    
    // FIXME replace with v4l2 equivalent or delete?
    /*if (fVCapability.type & VID_TYPE_MONOCHROME) {
      fError = "Seems to be a monochrome camera. We don't support those at the moment.";
      return false;
    }*/

    fVPicture.depth=24;
    fVPicture.palette=VIDEO_PALETTE_RGB24;
    // V4L2_PIX_FMT_RGB24

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
    if (ioctl(fFd, VIDIOC_QUERYCAP, &fVCapability) != 0) {
      fError = string("Could not get device capability struct via VIDIOGCAP. Is")
               + fDevice + string(" not a V4L2 device?");
      return false;
    }
    if (!fVCapability.capabilities & V4L2_CAP_VIDEO_CAPTURE) {
      fError = "Device doesn't seem to support video capture.";
      return false;
    }
    // TODO for future: Potentially use streaming. Hint: V4L2_CAP_STREAMING
    return true;
  }

  bool ImageCapture::CheckAndResetControls() {
    // Adapted from v4l2 reference, example 1-9 (Chapter 1-8)
    struct v4l2_queryctrl queryctrl;
    struct v4l2_control control;

    memset(&queryctrl, 0, sizeof(queryctrl));
    memset(&control, 0, sizeof(control));

    const unsigned int nControls = 2;
    __u32 controlIds[nControls] = {
      V4L2_CID_BRIGHTNESS, V4L2_CID_CONTRAST
    };
    // FIXME: These are certainly provided by v4l2
    char* controlNames[nControls] = {
      "brightness",
      "contrast"
    };

    for (unsigned int iControl = 0; iControl < nControls; ++i) {
      __u32 controlId = controlIds[iControl];
      queryctrl.id = controlId;

      if (-1 == ioctl(fFd, VIDIOC_QUERYCTRL, &queryctrl)) {
        if (errno != EINVAL) {
          fError = string("Failed doing a VIDIOC_QUERYCTRL for control ") + string(controlNames[iControl]);
          return false;
        } else {
          fError = string("control '") + string(controlNames[iControl]) + string("' is not supported");
          return false;
        }
      } else if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED) {
        fError = string("control '") + string(controlNames[iControl]) + string("' is not supported");
        return false;
      } else {
        control.id = controlId;
        control.value = queryctrl.default_value;

        if (-1 == ioctl(fFd, VIDIOC_S_CTRL, &control)) {
          fError = string("Error setting control '") + string(controlNames[iControl]) + string("' to default value");
          return false;
        }
      }
    }

    return true;
  }

  bool ImageCapture::ResetCrop() {
    // From v4l2 reference, example 1-11 (Chapter 1-11)
    struct v4l2_cropcap cropcap;
    struct v4l2_crop crop;

    memset(&cropcap, 0, sizeof(cropcap));
    cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (-1 == ioctl(fFd, VIDIOC_CROPCAP, &cropcap)) {
      fError = "VIDIOC_CROPCAP failed";
      return false;
    }

    memset(&crop, 0, sizeof(crop));
    crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    crop.c = cropcap.defrect; 

    /* Ignore if cropping is not supported (EINVAL). */
    if (-1 == ioctl(fFd, VIDIOC_S_CROP, &crop)
        && errno != EINVAL)
    {
      fError = "VIDIOC_S_CROP failed";
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

    if ( width > fVCapability.maxwidth || height > fVCapability.maxheight ) {
      fError = "width or height exceed maximum device capability";
      return false;
    }
    else if ( width < fVCapability.minwidth || height < fVCapability.minheight ) {
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
    return SetControl(V4L2_CID_BRIGHTNESS, (__s32) brightness);
  }

  bool ImageCapture::SetContrast(unsigned int contrast) {
    return SetControl(V4L2_CID_CONTRAST, (__s32) contrast);
  }

  unsigned int ImageCapture::GetBrightness() {
    return (unsigned int) GetControl(V4L2_CID_BRIGHTNESS);
  }

  unsigned int ImageCapture::GetContrast() {
    return (unsigned int) GetControl(V4L2_CID_CONTRAST);
  }

  bool ImageCapture::SetCaptureProperties(unsigned int brightness, unsigned int contrast) {
    bool success = SetBrightness(brightness);
    success = SetContrast(contrast) && success;
    return success;
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
  
  bool ImageCapture::SetControl(__u32 id, __s32 value) {
    v4l2_control control;
    control.id = id;
    control.value = value;

    if (-1 == ioctl(fFd, VIDIOC_S_CTRL, &control)) {
      ostringstream o;
      o << "Error setting control with id '" << id << " to value " << value;
      fError = o.str();
      return false;
    }
    return true;
  }

  __s32 ImageCapture::GetControl(__u32 id) {
    v4l2_control control;
    control.id = id;
    control.value = 0;

    if (-1 == ioctl(fFd, VIDIOC_G_CTRL, &control)) {
      ostringstream o;
      o << "Error Getting control with id '" << id;
      fError = o.str();
      return -1;
    }
    return control.value;
  }

} // end namespace findlaser
