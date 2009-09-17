#ifndef __FindLaser__ImageCapture_h
#define __FindLaser__ImageCapture_h

#include <string>

#include <linux/videodev2.h>


namespace FindLaser {
  // TODO investigate setting whiteness (V4L2_CID_WHITENESS) and colour (V4L2_CID_SATURATION) as well
  class ImageCapture {
    public:
      ImageCapture(std::string device);
      ~ImageCapture();
      
      void SetVerbosity(const unsigned int verb) { fVerbosity = verb; }
      bool Initialize();

      std::string GetError(); /// Returns the current error string

      bool AdjustBrightness(); /// Auto-adjust brightness

      bool SetImageSize(const unsigned int width, const unsigned int height);

      bool SetRelBrightness(float relBrightness); /// Set the desired brightness [0,1]
      bool SetRelContrast(float relContrast); /// Set the desired contrast [0,1]
      float GetRelBrightness(); /// Fetch the current brightness [0,1]
      float GetRelContrast(); /// Fetch the current contrast [0,1]

      bool SetCaptureProperties(float relBrightness, float relContrast); /// Set brightness and contrast

      unsigned char* CaptureImagePointer(unsigned int& size);

    private:

      bool GetCapability(); /// fetch the v4l2 device capabilities as fCapability
      bool ResetCrop(); /// Reset the v4l2 crop settings of the device since they're persistent
      bool CheckAndResetControls(); /// Checks the availability of the required controls and sets them to their default value
      bool CheckAndResetImageFormat(); /// Does the initial (safe) settings for the image format and size

      bool GetImageFormat(); /// Fetch the initial image format settings from the device (done during initialization)
      bool SetImageFormat(); /// Set the devices image format settings
      void ReallocBuffer(); /// Re-allocate (or allocate) the buffer using the format's image size

      int GetBrightnessAdjustment(unsigned char* image, long size, int* brightness); /// helper for AdjustBrightness

      bool SetControl(__u32 id, __s32 value); /// Set a control value (brightness, contrast...)
      __s32 GetControl(__u32 id); /// Get a control value (brightness, contrast...)

      unsigned int fVerbosity;
      std::string fDevice; /// The path to the device, i.e. /dev/video
      std::string fError; /// The current error string
      bool fInitialized;
      int fFd; /// The device file descriptor
      struct v4l2_capability fVCapability; /// Describes the capability of the video hardware (done during initialization)
      struct v4l2_queryctrl fBrightnessQuery; /// The general query about the brightness control containing min/max (done during initialization)
      struct v4l2_queryctrl fContrastQuery; /// The general query about the contrast control containing min/max (done during initialization)
      struct v4l2_format fImageFormat; /// Image format info (fetched during initialization)
      
      unsigned char* fBuffer;
  };
}

#endif
