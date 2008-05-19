#include "history.h"
#include <string>
#include <iostream>

using namespace std;

namespace FindLaser {

  HistoryPoint::HistoryPoint(const clock_t t, const double x, const double y) {
    fTime = t;
    fX = x;
    fY = y;
  }

  History::History()
    : fMemoryLength(5.0)
  {
  }

  void History::AddPoint(const HistoryPoint histPoint) {
    fPoints.push(histPoint);
  }

  void History::DiscardOldPoints() {
    if (fPoints.empty()) return;
    const clock_t discardBefore = clock()-fMemoryLength;
    while (!fPoints.empty() && fPoints.front().GetT() < discardBefore) {
      fPoints.pop();
    }
    return;
  }


} // end namespace FindLaser 

