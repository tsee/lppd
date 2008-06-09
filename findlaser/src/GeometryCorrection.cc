#include "GeometryCorrection.h"
#include <cmath>

namespace FindLaser {
  GeometryCorrection::GeometryCorrection()
  : fSourceWidth(0), fSourceHeight(0),
    fTargetWidth(0), fTargetHeight(0)
  {
  }
   
  void GeometryCorrection::SetSourceImageDimensions(
    const unsigned int sourceWidth, const unsigned int sourceHeight
  ) {
    fSourceWidth = sourceWidth;
    fSourceHeight = sourceHeight;
  }

  void GeometryCorrection::SetTargetImageDimensions(
    const unsigned int targetWidth, const unsigned int targetHeight
  ) {
    fTargetWidth = targetWidth;
    fTargetHeight = targetHeight;
  }

  void GeometryCorrection::SetImageUpperLeft( const float x, const float y ) {
    fImageUpperLeftX = x;
    fImageUpperLeftY = y;
    CalcImageProperties();
  }

  void GeometryCorrection::SetImageUpperRight( const float x, const float y ) {
    fImageUpperRightX = x;
    fImageUpperRightY = y;
    CalcImageProperties();
  }

  void GeometryCorrection::SetImageLowerLeft( const float x, const float y ) {
    fImageLowerLeftX = x;
    fImageLowerLeftY = y;
    CalcImageProperties();
  }

  void GeometryCorrection::SetImageLowerRight( const float x, const float y ) {
    fImageLowerRightX = x;
    fImageLowerRightY = y;
    CalcImageProperties();
  }

  void GeometryCorrection::CalcImageProperties() {
    fImageUpperY = (fImageUpperLeftY+fImageUpperRightY)/2.;
    fImageLowerY = (fImageLowerLeftY+fImageLowerRightY)/2.;
    fImageHeight = fImageUpperY - fImageLowerY;

    fImageUpperWidth = fImageUpperRightX - fImageUpperLeftX;
    fImageLowerWidth = fImageLowerRightX - fImageLowerLeftX;
  }

  void GeometryCorrection::GetImageCoordinates(
    const float sourceX, const float sourceY,
    float& targetX, float& targetY
  ) {
    const float relY = (fImageUpperY-sourceY)/fImageHeight;
    targetY = relY * fTargetHeight;

    const float leftXBoundary  = fImageLowerLeftX + (fImageUpperLeftX-fImageLowerLeftX)*relY;
    const float rightXBoundary = fImageLowerRightX + (fImageUpperRightX-fImageLowerRightX)*relY;
    const float thisWidth = rightXBoundary - leftXBoundary;
    
    const float relX = (sourceX-leftXBoundary)/thisWidth;
    targetX = relX * fTargetWidth;
  }

} // end namespace FindLaser
