
#include "Event.h"
#include <string>
#include <iostream>

using namespace std;

namespace FindLaser {

  Event::Event(const clock_t t)
    : fTime(t)
  {
  }

  PosEvent::PosEvent(const clock_t t, const double& x, const double& y)
    : fX(x),
    fY(y)
  {
    SetT(t);
  }

} // end namespace FindLaser
