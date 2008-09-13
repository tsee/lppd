#ifndef __FindLaser__LocalSequenceEventFinder_h
#define __FindLaser__LocalSequenceEventFinder_h

#include <ctime>
#include <vector>
#include <list>
#include "Event.h"
#include "EventFinder.h"

namespace FindLaser {
  class HistoryPoint;
  class LocalSequenceEventFinder : public EventFinder {
    public:
      LocalSequenceEventFinder();

      virtual std::vector<Event> Find(const std::list<HistoryPoint>& points);

      virtual ~LocalSequenceEventFinder() {};

    private:
  }; // end class LocalSequenceEventFinder

} // end namespace FindLaser

#endif
