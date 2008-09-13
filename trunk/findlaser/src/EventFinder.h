#ifndef __FindLaser__EventFinder_h
#define __FindLaser__EventFinder_h

#include <vector>
#include <list>
#include "Event.h"
//#include "HistoryPoint.h"

namespace FindLaser {
  class HistoryPoint;
  class EventFinder {
    public:
      virtual std::vector<Event> Find(const std::list<HistoryPoint>& points) = 0;

      virtual ~EventFinder() {};

    protected:
      EventFinder();
  }; // end class EventFinder

} // end namespace FindLaser

#endif
