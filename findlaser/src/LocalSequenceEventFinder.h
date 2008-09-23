#ifndef __FindLaser__LocalSequenceEventFinder_h
#define __FindLaser__LocalSequenceEventFinder_h

#include <vector>
#include <list>
#include <ctime>


#include "Event.h"
#include "EventFinder.h"

namespace pcrecpp {class RE;};
namespace FindLaser {
  class HistoryPoint;
  
  class LocalSequenceEventFinder : public EventFinder {
    public:
      LocalSequenceEventFinder();

      LocalSequenceEventFinder(
        const unsigned int repetitions,
        const unsigned int minTriggeredEvents,
        const unsigned int minUntriggeredEvents,
        const clock_t minOnTime,
        const clock_t minPause,
        const clock_t maxPause
      );

      virtual std::vector<Event> Find(std::list<HistoryPoint>& points);

      int GetVerbosity() { return fVerbosity; }
      void SetVerbosity(int verb) { fVerbosity = verb; }

      virtual ~LocalSequenceEventFinder();

    private:
      enum MatchState {
        NONE,
        PREVOFF,
        PREVON,
      };

      struct Match {
        MatchState state;
        std::list<HistoryPoint>::iterator point;
        std::list<HistoryPoint>::iterator end;
        clock_t streakStartTime;
        clock_t matchStartTime;
        unsigned int streakLength;
        unsigned int streaks;
        std::vector<Event> events;
      };

      void MatcherHandleNone(Match& m);
      void MatcherHandleOff(Match& m);
      void MatcherHandleOn(Match& m);
      void ResetMatcher(Match& m);
      void StartMatcher(Match& m);

      int fVerbosity;
      unsigned int fRepetitions;
      unsigned int fMinUntriggeredEvents;
      unsigned int fMinTriggeredEvents;
      clock_t fMinOnTime;
      clock_t fMinPauseTime;
      clock_t fMaxPauseTime;
      clock_t fMinTotalTime;

      clock_t fLastTriggeredTime;
  }; // end class LocalSequenceEventFinder

} // end namespace FindLaser

#endif
