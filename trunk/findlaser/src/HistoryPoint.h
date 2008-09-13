#ifndef __FindLaser__HistoryPoint_h
#define __FindLaser__HistoryPoint_h

#include <ctime>

namespace FindLaser {

  class HistoryPoint {
    public:
      HistoryPoint(const clock_t t, const double x, const double y, bool triggered);

      double GetX() { return fX; }
      void SetX(const double x) { fX = x; }
      double GetY() { return fY; }
      void SetY(const double y) { fY = y; }
      clock_t GetT() { return fTime; }
      void SetT(const clock_t t) { fTime = t; }

      bool Triggered() { return fTrigger; }
      void SetTrigger(const bool triggered) { fTrigger = triggered; }
      
    private:
      clock_t fTime;
      double fX;
      double fY;
      bool fTrigger;
  }; // end class HistoryPoint

} // end namespace FindLaser

#endif
