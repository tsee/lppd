
#include "HistoryPoint.h"
#include <string>
#include <iostream>

using namespace std;

namespace FindLaser {

  HistoryPoint::HistoryPoint(const clock_t t, const double x, const double y) {
    fTime = t;
    fX = x;
    fY = y;
  }

} // end namespace FindLaser
