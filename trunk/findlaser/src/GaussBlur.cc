
#include "GaussBlur.h"

#include <iostream>
#include <string>
#include <cmath>

using std::cout;
using std::cerr;
using std::endl;
using std::string;

namespace FindLaser {

  GaussBlur::GaussBlur(const float sigma)
    : fSigma(sigma)
  {
    fNPixels = (unsigned int) (fSigma * 6. + 1.);
    
    const unsigned int npix = fNPixels*2 + 1;
    fKernel = new float[npix];
    for (int i = 0; i < (int)npix; i++) {
      fKernel[i] = GaussFunc(sigma, (float)(i-fNPixels));
    }

  }

  GaussBlur::~GaussBlur() {
    delete fKernel;
  }

  float GaussBlur::GaussFunc (const float sigma, const float x) {
    const double factor = 1./sqrt(2*M_PI);
    const double expon  = pow( M_E, -x*x*0.5/(sigma*sigma) );
    return((float) factor / sigma * expon);
  }
} // end namespace FindLaser

