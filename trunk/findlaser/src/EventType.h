#ifndef __FindLaser__EventType_h
#define __FindLaser__EventType_h

#include <ctime>
#include <vector>
#include <list>
#include "Event.h"
//#include "HistoryPoint.h"

namespace FindLaser {
  class HistoryPoint;
  class EventType {
    public:
      EventType();
      virtual ~EventType() {};

      virtual std::vector<Event> Find(const std::list<HistoryPoint>& points);

    private:
  }; // end class EventType

} // end namespace FindLaser

#endif
