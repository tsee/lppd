
#include "History.h"
#include "HistoryPoint.h"
#include <string>
#include <iostream>

using namespace std;

namespace FindLaser {

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

