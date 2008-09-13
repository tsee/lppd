#ifndef __FindLaser__EventFinder_h
#define __FindLaser__EventFinder_h

#include <ctime>
#include <vector>
#include <list>
#include "Event.h"
//#include "HistoryPoint.h"

namespace FindLaser {
  class HistoryPoint;
  class EventFinder {
    public:
      EventFinder();
      virtual ~EventFinder() {};

      //virtual std::vector<Event> Find(const std::list<HistoryPoint>& points) = 0;
      virtual std::vector<Event> Find(const std::list<HistoryPoint>& points);

    private:
  }; // end class EventFinder

} // end namespace FindLaser

#endif
