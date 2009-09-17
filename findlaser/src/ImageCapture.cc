#include "ImageCapture.h"

#include <vector>
#include <string>
#include <iostream>
#include <sstream>

#include <cstdlib>
#include <cstring>
#include <cerrno>

using std::string;
using std::vector;
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
    : fVerbosity(0), fDevice(device), fError(""), fInitialized(false), fFd(-1), fBuffer(NULL)
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
    fFd = open(fDevice.c_str(), O_RDWR); // TODO investigate O_NONBLOCK
    if (fFd < 0) {
      fError = string("Could not open device ") + fDevice;
      return false;
    }

    if (!GetCapability()) return false;
    if (!ResetCrop()) return false;
    if (!CheckAndResetImageFormat()) return false;
    if (!CheckAndResetControls()) return false;
    
    // FIXME replace with v4l2 equivalent or delete?
    /*if (fVCapability.type & VID_TYPE_MONOCHROME) {
      fError = "Seems to be a monochrome camera. We don't support those at the moment.";
      return false;
    }*/

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
    struct v4l2_control control;
    memset(&control, 0, sizeof(control));

    const unsigned int nControls = 2;
    __u32 controlIds[nControls] = {
      V4L2_CID_BRIGHTNESS, V4L2_CID_CONTRAST
    };
    // FIXME: These are certainly provided by v4l2
    const char* controlNames[nControls] = {
      "brightness",
      "contrast"
    };
    vector<struct v4l2_queryctrl*> queryctrls;
    queryctrls.push_back(&fBrightnessQuery);
    queryctrls.push_back(&fContrastQuery);

    for (unsigned int iControl = 0; iControl < nControls; ++iControl) {
      struct v4l2_queryctrl& queryctrl = *queryctrls[iControl];
      memset(&queryctrl, 0, sizeof(queryctrl));
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

  bool CheckAndResetImageFormat() {
    if (!GetImageFormat()) return false;
    fImageFormat.fmt.pix.pixelformat  = V4L2_PIX_FMT_RGB24; // format, see http://v4l2spec.bytesex.org/spec/c2030.htm#V4L2-PIX-FORMAT
    fImageFormat.fmt.pix.width        = 320; // safe default?
    fImageFormat.fmt.pix.height       = 240; // safe default?
    fImageFormat.fmt.pix.bytesperline = 0; // no padding, get default
    // FIXME: Need to check the interlacing via ".fmt.pix.field"?
    
    return SetImageFormat();
  }

  bool ImageCapture::GetImageFormat() {
    memset(&fImageFormat, 0, sizeof(fImageFormat)); // reset memory
    if (-1 == ioctl(fFd, VIDIOC_G_FMT, &fImageFormat)) {
      if (errno != EINVAL) {
        fError = string("Failed doing a VIDIOC_G_FMT for fetching the image format");
        return false;
      } else {
        fError = string("Failed doing a VIDIOC_G_FMT for fetching the image format");
        return false;
      }
    }
    return true;
  }

  bool ImageCapture::SetImageFormat() {
    if (!fInitialized)
      return false;

    if (-1 == ioctl(fFd, VIDIOC_S_FMT, &fImageFormat)) {
      fError = string("Failed doing a VIDIOC_S_FMT for setting the image format");
      return false;
    }
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

  bool ImageCapture::SetImageSize(const unsigned int width, const unsigned int height) {
    if (!fInitialized) return false;

    // no checking of max width and height because that's not supported by v4l2.
    // Instead, we could use V4L2_TRY_FMT to TRY the desired setup first. Laziness prevails.

    fImageFormat.fmt.pix.width  = width;
    fImageFormat.fmt.pix.height = height;
    return SetImageFormat();
  }

   string ImageCapture::GetError() {
    return fError;
  }

  bool ImageCapture::AdjustBrightness() {
    if (!fInitialized) return false;

    int f = 0;
    unsigned int size = 0;
    do {
      int newbright;
      CaptureImagePointer(size); // writes to fBuffer
      f = GetBrightnessAdjustment(fBuffer, fImageFormat.fmt.pix.width * fImageFormat.fmt.pix.height, &newbright);
      if (f) {
        // FIXME: a) check correctness, b) lose ugly constants
        const float relBrightness = (GetRelBrightness()*65536. + (newbright << 8))/65536.;
        if (!SetRelBrightness(relBrightness))
          return false;
        //cerr << fVPicture.brightness << endl;
      }
    } while (f);

    return true;
  }

  /// Internal aux. method for AdjustBrightness
  int ImageCapture::GetBrightnessAdjustment(unsigned char *image, long size, int *brightness) {
    long i, tot = 0;
    for (i = 0; i < size*3; ++i)
      tot += image[i];
    *brightness = (128 - tot/(size*3)) / 3;
    return !((tot/(size*3)) >= 122 && (tot/(size*3)) <= 134);
    //return !((tot/(size*3)) >= 126 && (tot/(size*3)) <= 130);
  }

  unsigned char* ImageCapture::CaptureImagePointer(unsigned int& size) {
    if (!fInitialized)
      return NULL;
// FIXME: implement reading in v4l2
/*    size = fWindow.width * fWindow.height * 24/8;

    read(fFd, fBuffer, size);
*/
    return fBuffer;
  }
  
  bool ImageCapture::SetControl(__u32 id, __s32 value) {
    v4l2_control control;
    control.id = id;
    control.value = value;

    if (-1 == ioctl(fFd, VIDIOC_S_CTRL, &control)) {
      std::ostringstream o;
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
      std::ostringstream o;
      o << "Error Getting control with id '" << id;
      fError = o.str();
      return -1;
    }
    return control.value;
  }

  /*******************************************************
   * Getters and setters
   */
  bool ImageCapture::SetRelBrightness(float relBrightness) {
    __u32 min = fBrightnessQuery.minimum;
    float brightness = min + (fBrightnessQuery.maximum-min)*relBrightness;
    return SetControl(V4L2_CID_BRIGHTNESS, (__s32)brightness);
  }

  bool ImageCapture::SetRelContrast(float relContrast) {
    __u32 min = fContrastQuery.minimum;
    float contrast = min + (fContrastQuery.maximum-min)*relContrast;
    return SetControl(V4L2_CID_CONTRAST, (__s32)contrast);
  }

  float ImageCapture::GetRelBrightness() {
    __u32 min = fBrightnessQuery.minimum;
    float reduced = GetControl(V4L2_CID_BRIGHTNESS)-min;
    return reduced / (float)(fBrightnessQuery.maximum - min);
  }

  unsigned int ImageCapture::GetRelContrast() {
    __u32 min = fContrastQuery.minimum;
    float reduced = GetControl(V4L2_CID_CONTRAST)-min;
    return reduced / (float)(fContrastQuery.maximum - min);
  }

  bool ImageCapture::SetCaptureProperties(float relBrightness, float relContrast) {
    bool success = SetBrightness(brightness);
    success = SetContrast(contrast) && success;
    return success;
  }

} // end namespace findlaser
