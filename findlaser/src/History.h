#ifndef __FindLaser_History_h
#define __FindLaser_History_h

#include <queue>
#include <ctime>

namespace FindLaser {
  class HistoryPoint;

  class History {
    public:
      History ();

      void AddPoint(const HistoryPoint histPoint);

      void DiscardOldPoints();

      void SetMemoryLength(const double seconds) { fMemoryLength = seconds/CLOCKS_PER_SEC; }
      double GetMemoryLength() { return fMemoryLength; }

    private:
      double fMemoryLength;
      std::queue<HistoryPoint> fPoints;
      
  }; // end class History

} // end namespace FindLaser

#endif
