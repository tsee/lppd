
#include <vector>
#include <string>

namespace FindLaser {
  class GreyImage;
  class ColorImage;
  class Color;

  class Image {
    public:
      Image();
      virtual ~Image();

      bool IsEmpty() { return fImage.empty(); }
      void ClearImage();
      std::vector<unsigned char*>& GetImage() { return fImage; }
      virtual int GetSamples() { return 0; }
      int GetColumns() { return fColumns; }
      void SetColumns(int cols) { fColumns = cols; }
      int GetRows() { return fImage.size(); }
      virtual void SaveAsJPEG(std::string filename, int quality) {}
      unsigned char* AsPNM(unsigned int& datasize);

    protected:
      std::vector<unsigned char*> fImage;
      int fColumns;
  };

  class ColorImage : public Image {
    public:
      ColorImage();
      virtual ~ColorImage();

      bool ReadFromFile (const std::string filename);
      int GetSamples() { return 3; }

      bool ReverseCopyFromMemory (unsigned int width, unsigned int height, unsigned char* ptr);

      GreyImage* ColorProjection(const Color& color);

      // at least twice as fast as the generic ColorProjection
      GreyImage* RedProjection();
      GreyImage* GreenProjection();
      GreyImage* BlueProjection();

      GreyImage* NormalizedColorProjection(const Color& color, const unsigned char colorThreshold = 0, const unsigned char colorDefault = 0);

      GreyImage* ToGreyscale(const unsigned char brightnessThreshold = 0, const unsigned char brightnessDefault = 0);


      void FindLaserCentroid(
        double& centroidx, double& centroidy, const Color& color, 
        const double colorThreshold, const double lightThreshold
      );

      void SaveAsJPEG(std::string filename, int quality);

    private:
      GreyImage* FixedColorProjection(unsigned int colorOffset);
  };

  class GreyImage : public Image {
    public:
      GreyImage();
      virtual ~GreyImage();
      
      GreyImage* CutOnThreshold(unsigned char threshold, unsigned char replacement);

      void SaveAsJPEG(std::string filename, int quality);
      int GetSamples() { return 1; }
  };

  // I know, public... but this is just a struct!
  class Color {
    public:
    Color(unsigned char r, unsigned char g, unsigned char b);

    ~Color() {}

    Color GetNormalized();

    unsigned char red;
    unsigned char green;
    unsigned char blue;
  }; // end class Color
}

