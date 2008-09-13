#ifndef __FindLaser_History_h
#define __FindLaser_History_h

#include <list>
#include <queue>
#include <vector>
#include <ctime>
#include "Event.h"
#include "EventFinder.h"
#include "HistoryPoint.h"

namespace FindLaser {

  class History {
    public:
      History ();
      ~History();

      void AddPoint(const HistoryPoint& histPoint);

      void DiscardOldPoints();

      void SetMemoryLength(const double seconds) { fMemoryLengthInSeconds = seconds; fMemoryLength = seconds/CLOCKS_PER_SEC; }
      double GetMemoryLength() { return fMemoryLengthInSeconds; }

      void AddEventFinder(EventFinder* evType);

      void FindEvents();

      std::queue<Event>& GetEvents() { return fEvents; }
      void ClearEvents() { while (!fEvents.empty()) {fEvents.pop();} }

    private:
      double fMemoryLength;
      double fMemoryLengthInSeconds;
      std::list<HistoryPoint> fPoints;
      std::vector<EventFinder*> fEventFinders;
      std::queue<Event> fEvents;

  }; // end class History

} // end namespace FindLaser

#endif
