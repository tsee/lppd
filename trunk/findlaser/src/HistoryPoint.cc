
#include "HistoryPoint.h"
#include <string>
#include <iostream>

using namespace std;

namespace FindLaser {

  HistoryPoint::HistoryPoint(const clock_t t, const double x, const double y, const bool triggered)
    : fTime(t), fX(x), fY(y), fTrigger(triggered)
  {
  }

} // end namespace FindLaser
