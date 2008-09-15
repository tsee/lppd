#ifndef __FindLaser__LocalSequenceEventFinder_h
#define __FindLaser__LocalSequenceEventFinder_h

#include <vector>
#include <list>

#include "Event.h"
#include "EventFinder.h"

namespace pcrecpp {class RE;};
namespace FindLaser {
  class HistoryPoint;
  class LocalSequenceEventFinder : public EventFinder {
    public:
      LocalSequenceEventFinder(
        const unsigned int repetitions,
        const unsigned int minTriggeredEvents,
        const unsigned int minUntriggeredEvents,
        const double minOnTime,
        const double minPause
      );

      virtual std::vector<Event> Find(std::list<HistoryPoint>& points);

      virtual ~LocalSequenceEventFinder();

    private:
      enum State { OFF, ON };

      unsigned int fRepetitions;
      unsigned int fMinUntriggeredEvents;
      unsigned int fMinTriggeredEvents;
      double fMinOnTime;
      double fMinPauseTime;
      double fMinTotalTime;
      pcrecpp::RE* fRegexp;
  }; // end class LocalSequenceEventFinder

} // end namespace FindLaser

#endif
