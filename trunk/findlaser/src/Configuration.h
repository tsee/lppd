#include <string>

namespace FindLaser {

  class Configuration {
    public:
      Configuration(const std::string& fileName);

      bool WriteConfiguration();

      unsigned int GetCameraImageSizeX() { return fCameraImageSizeX; }
      void SetCameraImageSizeX(const unsigned int sizeX) { fCameraImageSizeX = sizeX; }
      unsigned int GetCameraImageSizeY() { return fCameraImageSizeY; }
      void SetCameraImageSizeY(const unsigned int sizeY) { fCameraImageSizeY = sizeY; }

      int GetCameraBrightness() { return fCameraBrightness; }
      void SetCameraBrightness(const int camBr) { fCameraBrightness = camBr; }
      int GetCameraContrast() { return fCameraContrast; }
      void SetCameraContrast(const int camCo) { fCameraContrast = camCo; }

      double GetBrightnessThreshold() { return fBrightnessThreshold; }
      void SetBrightnessThreshold(const double briThr) { fBrightnessThreshold = briThr; }
      double GetColorThreshold() { return fColorThreshold; }
      void SetColorThreshold(const double colThr) { fColorThreshold = colThr; }

      const std::string& GetCameraDevice() { return fCameraDevice; }
      void SetCameraDevice(const std::string& dev) { fCameraDevice = dev; }

      const std::string& GetFileName() { return fFileName; }
      void SetFileName(const std::string& fileName) { fFileName = fileName; }

    private:
      bool ReadConfiguration();

      std::string fFileName;
      unsigned int fCameraImageSizeX;
      unsigned int fCameraImageSizeY;
      int fCameraBrightness;
      int fCameraContrast;
      double fBrightnessThreshold;
      double fColorThreshold;
      std::string fCameraDevice;
  }; // end class Configuration

} // end namespace FindLaser


