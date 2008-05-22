#ifndef __FindLaser__ImageCapture_h
#define __FindLaser__ImageCapture_h

#include <string>
#include <linux/videodev.h>

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

      bool GetCapability();
      bool GetWindow();
      bool SetWindow();
      bool GetVideoPicture();
      bool SetVideoPicture();
      int GetBrightnessAdjustment(unsigned char* image, long size, int* brightness);

      unsigned int fVerbosity;
      std::string fDevice;
      std::string fError;
      bool fInitialized;
      int fFd;
      struct video_capability fCapability;
      struct video_window fWindow;
      struct video_picture fVPicture;
      
      unsigned char* fBuffer;
  };
}

#endif
