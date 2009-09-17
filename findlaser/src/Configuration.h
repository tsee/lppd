#ifndef __FindLaser__Configuration_h
#define __FindLaser__Configuration_h

#include <string>

namespace FindLaser {
  class GeometryCorrection;

  class Configuration {
    public:
      Configuration(const std::string& fileName);
      ~Configuration();

      bool WriteConfiguration();

      unsigned int GetCameraImageSizeX() { return fCameraImageSizeX; }
      void SetCameraImageSizeX(const unsigned int sizeX) { fCameraImageSizeX = sizeX; }
      unsigned int GetCameraImageSizeY() { return fCameraImageSizeY; }
      void SetCameraImageSizeY(const unsigned int sizeY) { fCameraImageSizeY = sizeY; }

      float GetCameraRelBrightness() { return fCameraBrightness; }
      void SetCameraRelBrightness(const float camRelBr);
      float GetCameraRelContrast() { return fCameraContrast; }
      void SetCameraRelContrast(const float camRelCo);

      double GetBrightnessThreshold() { return fBrightnessThreshold; }
      void SetBrightnessThreshold(const double briThr);
      double GetColorThreshold() { return fColorThreshold; }
      void SetColorThreshold(const double colThr);

      const std::string& GetCameraDevice() { return fCameraDevice; }
      void SetCameraDevice(const std::string& dev) { fCameraDevice = dev; }

      const std::string& GetFileName() { return fFileName; }
      void SetFileName(const std::string& fileName) { fFileName = fileName; }

      GeometryCorrection* GetGeometryCorrection() { return fGeometryCorrection; }
      void SetGeometryCorrection(GeometryCorrection* g);

      int GetBrightnessThresholdMin() { return fgBrightnessThresholdMin; }
      int GetBrightnessThresholdMax() { return fgBrightnessThresholdMax; }
      int GetColorThresholdMin() { return fgColorThresholdMin; }
      int GetColorThresholdMax() { return fgColorThresholdMax; }

    private:
      bool ReadConfiguration();

      const static unsigned int fgBrightnessThresholdMin;
      const static unsigned int fgBrightnessThresholdMax;
      const static unsigned int fgColorThresholdMin;
      const static unsigned int fgColorThresholdMax;

      std::string fFileName;
      unsigned int fCameraImageSizeX;
      unsigned int fCameraImageSizeY;
      float fCameraBrightness;
      float fCameraContrast;
      double fBrightnessThreshold;
      double fColorThreshold;
      std::string fCameraDevice;
      GeometryCorrection* fGeometryCorrection;
  }; // end class Configuration

} // end namespace FindLaser

#endif
