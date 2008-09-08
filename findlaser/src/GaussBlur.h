#ifndef __FindLaser__GaussBlur_h
#define __FindLaser__GaussBlur_h

#include <string>

namespace FindLaser {
  class GaussBlur {
    public:
      GaussBlur(const float sigma);
      virtual ~GaussBlur();
      const float* GetKernel() { return fKernel; }
      unsigned int GetKernelSize() { return fNPixels*2 + 1; }
      unsigned int GetNPixels() { return fNPixels; }

    private:
      float GaussFunc(const float sigma, const float x);

      float fSigma;
      float* fKernel;
      unsigned int fNPixels;
  };
} // end namespace FindLaser

#endif
