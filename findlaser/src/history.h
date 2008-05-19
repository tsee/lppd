#include <queue>
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


