#ifndef __FindLaser__Image_h
#define __FindLaser__Image_h

#include <string>

namespace FindLaser {
  class Image {
    public:
      Image();
      virtual ~Image();

      bool IsEmpty() { return (fColumns == 0 ? true : false); }
      void ClearImage();
      unsigned char* GetImage() { return fImage; }
      virtual int GetSamples() { return 0; }

      int GetColumns() { return fColumns; }
      void SetColumns(int cols) { fColumns = cols; }
      int GetRows() { return fRows; }
      void SetRows(int rows) { fRows = rows; }

      virtual void SaveAsJPEG(std::string filename, int quality) {}
      unsigned char* AsPNM(unsigned int& datasize);

    protected:
      void SetImage(unsigned char* img) { fImage = img; }

    private:
      unsigned char* fImage;
      int fColumns;
      int fRows;
  }; // end class Image


} // end namespace FindLaser

#endif
