#ifndef __FindLaser__HistoryPoint_h
#define __FindLaser__HistoryPoint_h

#include <ctime>

namespace FindLaser {

  class HistoryPoint {
    public:
      HistoryPoint(const clock_t t, const double x, const double y);

      double GetX() { return fX; }
      void SetX(const double x) { fX = x; }
      double GetY() { return fY; }
      void SetY(const double y) { fY = y; }
      clock_t GetT() { return fTime; }
      void SetT(const clock_t t) { fTime = t; }
      
    private:
      clock_t fTime;
      double fX;
      double fY;
  }; // end class HistoryPoint

} // end namespace FindLaser

#endif
