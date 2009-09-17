#ifndef __FindLaser__ImageCapture_h
#define __FindLaser__ImageCapture_h

#include <string>

#include <linux/videodev2.h>


namespace FindLaser {
  class ImageCapture {
    public:
      ImageCapture(std::string device);
      ~ImageCapture();
      
      void SetVerbosity(const unsigned int verb) { fVerbosity = verb; }
      bool Initialize();

      std::string GetError();

      bool AdjustBrightness();

      bool SetImageSize(unsigned int width, unsigned int height);
      bool SetBrightness(unsigned int brightness);
      bool SetContrast(unsigned int contrast);
      bool SetCaptureProperties(unsigned int brightness, unsigned int contrast);

      unsigned int GetBrightness();
      unsigned int GetContrast();


      unsigned char* CaptureImagePointer(unsigned int& size);

    private:

      bool GetCapability(); /// fetch the v4l2 device capabilities as fCapability
      bool ResetCrop(); /// Reset the v4l2 crop settings of the device since they're persistent
      bool CheckAndResetControls(); /// Checks the availability of the required controls and sets them to their default value
      bool GetWindow();
      bool SetWindow();
      bool GetVideoPicture();
      bool SetVideoPicture();
      int GetBrightnessAdjustment(unsigned char* image, long size, int* brightness);

      bool SetControl(__u32 id, __s32 value); /// Set a control value (brightness, contrast...)
      __s32 GetControl(__u32 id); /// Get a control value (brightness, contrast...)

      unsigned int fVerbosity;
      std::string fDevice; /// The path to the device, i.e. /dev/video
      std::string fError;
      bool fInitialized;
      int fFd; /// The device file descriptor
      struct v4l2_capability fVCapability;
      //struct video_window fWindow;
      //struct video_picture fVPicture;
      
      unsigned char* fBuffer;
  };
}

#endif
