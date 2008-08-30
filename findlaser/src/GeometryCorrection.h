#ifndef __FindLaser__GeometryCorrection_h
#define __FindLaser__GeometryCorrection_h_h

namespace FindLaser {
  class GeometryCorrection {
    public:
      GeometryCorrection();
      ~GeometryCorrection() {}

      void SetSourceImageDimensions( const unsigned int sourceWidth, const unsigned int sourceHeight );
      void SetTargetImageDimensions( const unsigned int targetWidth, const unsigned int targetHeight );
      unsigned int GetSourceImageWidth() { return fSourceWidth; }
      unsigned int GetSourceImageHeight() { return fSourceHeight; }
      unsigned int GetTargetImageWidth() { return fTargetWidth; }
      unsigned int GetTargetImageHeight() { return fTargetHeight; }

      void SetImageUpperLeft( const float x, const float y );
      void SetImageUpperRight( const float x, const float y );
      void SetImageLowerLeft( const float x, const float y );
      void SetImageLowerRight( const float x, const float y );
      
      void GetImageCornersClockwise( float* coords );

      void GetImageCoordinates(
        const float sourceX, const float sourceY,
        float& targetX, float& targetY
      );

    private:
      void CalcImageProperties();

      unsigned int fSourceWidth;
      unsigned int fSourceHeight;
      unsigned int fTargetWidth;
      unsigned int fTargetHeight;
      // The coordinates of the four image corners of the
      // target image in source coordinates
      float fImageUpperLeftX;
      float fImageUpperLeftY;
      float fImageUpperRightX;
      float fImageUpperRightY;
      float fImageLowerLeftX;
      float fImageLowerLeftY;
      float fImageLowerRightX;
      float fImageLowerRightY;

      float fImageUpperY;
      float fImageLowerY;
      float fImageHeight;

      float fImageLowerWidth;
      float fImageUpperWidth;

  }; // end class GeometryCorrection

} // end namespace FindLaser

#endif
