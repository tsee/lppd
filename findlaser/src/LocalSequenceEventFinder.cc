
#include "LocalSequenceEventFinder.h"

#include <string>
#include <iostream>
#include <ctime>

#include "HistoryPoint.h"
#include "Event.h"

using namespace std;

namespace FindLaser {

  LocalSequenceEventFinder::LocalSequenceEventFinder()
    : fRepetitions(2),
    fMinUntriggeredEvents(2),
    fMinTriggeredEvents(1),
    fMinOnTime(0.15*CLOCKS_PER_SEC),
    fMinPauseTime(0.15*CLOCKS_PER_SEC),
    fMaxPauseTime(1.00*CLOCKS_PER_SEC),
    fVerbosity(0)
  {
    const clock_t repHalf = (clock_t) (fRepetitions / 2);
    fMinTotalTime = (repHalf+1) * fMinOnTime + repHalf*fMinPauseTime;
  }

  LocalSequenceEventFinder::LocalSequenceEventFinder(
    const unsigned int repetitions = 2,
    const unsigned int minTriggeredEvents = 2,
    const unsigned int minUntriggeredEvents = 1,
    const clock_t minOnTime = 0.15*CLOCKS_PER_SEC,
    const clock_t minPause = 0.15*CLOCKS_PER_SEC,
    const clock_t maxPause = 1.00*CLOCKS_PER_SEC
  )
    : fRepetitions(repetitions),
    fMinUntriggeredEvents(minUntriggeredEvents),
    fMinTriggeredEvents(minTriggeredEvents),
    fMinOnTime(minOnTime),
    fMinPauseTime(minPause),
    fMaxPauseTime(maxPause),
    fVerbosity(0)
  {
    const clock_t repHalf = (clock_t) (fRepetitions / 2);
    fMinTotalTime = (repHalf+1) * fMinOnTime + repHalf*fMinPauseTime;
  }

  vector<Event> LocalSequenceEventFinder::Find(list<HistoryPoint>& points) {
    if (fVerbosity > 2)
      cout << "Total time required for a LocalSequenceEventFinder trigger: " << fMinTotalTime << "\n"
           << "min pause: " << fMinPauseTime << "\n"
           << "max pause: " << fMaxPauseTime << "\n"
           << "min on: " << fMinOnTime << "\n"
           << endl;

    const unsigned int nPoints = points.size(); // is this size() O(n)?
    vector<Event> ev;
    if ( nPoints < fRepetitions * fMinTriggeredEvents + (fRepetitions-1) * fMinUntriggeredEvents ) {
      if (fVerbosity > 0)
        cout << "History too short, returning no events" << endl;
      return ev;
    }
    
    const clock_t latest   = points.back().GetT();
    const clock_t earliest = points.front().GetT();
    const clock_t duration = latest-earliest;
    if (duration < fMinTotalTime) {
      if (fVerbosity > 0)
        cout << "History too short in time (need " << fMinTotalTime/CLOCKS_PER_SEC << ", got " << duration/CLOCKS_PER_SEC << ", returning no events" << endl;
      return ev;
    }

    Match matcher;
    matcher.state = NONE;
    matcher.point = points.begin();
    matcher.end = points.end();

    unsigned int repetitions = fRepetitions;

    if (fVerbosity > 1)
      cout << "Initialized matcher, starting the match" << endl;

    while (matcher.point != matcher.end) {
      switch (matcher.state) {
        case NONE:
          MatcherHandleNone(matcher);
          break;
        case PREVOFF:
          MatcherHandleOff(matcher);
          break;
        case PREVON:
          MatcherHandleOn(matcher);
          break;
        default:
          cerr << "unknown state" << endl;
          break;
      }
      ++matcher.point;
    }
    
    return matcher.events;
  } // end Find


  void LocalSequenceEventFinder::MatcherHandleNone(Match& m) {
    HistoryPoint& p = *(m.point);
    if (p.Triggered()) {
      if (fVerbosity > 1)
        cout << "NONE => ON: " << p.GetT() << " streaks: " << m.streaks << endl;
      StartMatcher(m);
    }
    else
      m.state = NONE; // stay off if there's nothing!
  }


  void LocalSequenceEventFinder::MatcherHandleOff(Match& m) {
    HistoryPoint& p = *(m.point);
    if (!p.Triggered()) { // continue matching off-streak
      m.streakLength++;
      return;
    }

    //////////////////////
    // transition to on
    if (fVerbosity > 1)
      cout << "OFF => ON: " << p.GetT() << " streaks: " << m.streaks << endl;

    // check that streak is long enough in n events
    // check that streak is long/short enough in time
    const clock_t streakTime = p.GetT() - m.streakStartTime;
    if (m.streakLength < fMinUntriggeredEvents
        || streakTime < fMinPauseTime
        || streakTime > fMaxPauseTime)
    {
      StartMatcher(m);
      return; // FAIL, restart
    }
    
    // Passed streak if we got here.
    m.state = PREVON;
    m.streakStartTime = p.GetT();
    m.streakLength = 1;

  } // end MatcherHandleOff


  void LocalSequenceEventFinder::MatcherHandleOn(Match& m) {
    HistoryPoint& p = *(m.point);
    if (p.Triggered()) { // continue matching on-streak
      m.streakLength++;
      return;
    }

    //////////////////////
    // transition to off
    if (fVerbosity > 1)
      cout << "ON => OFF: " << p.GetT() << " streaks: " << m.streaks << endl;

    // check that streak is long enough in n events
    if (m.streakLength < fMinTriggeredEvents) {
      ResetMatcher(m);
      return; // FAIL
    }

    // check that streak is long enough in time
    if (p.GetT()-m.streakStartTime < fMinOnTime) {
      ResetMatcher(m);
      return; // FAIL
    }
    
    // Passed streak if we got here.
    m.state = PREVOFF;
    m.streakStartTime = p.GetT();
    m.streakLength = 1;
    m.streaks++;

    // check whether we have a global match
    if (m.streaks >= fRepetitions && p.GetT() - m.matchStartTime > fMinTotalTime) {
      if (fVerbosity > 0)
        cout << "MATCH! streaks: " << m.streaks << " of " << fRepetitions << " matchtime: " << p.GetT()-m.matchStartTime << " of " << fMinTotalTime << endl;
      PosEvent event(p.GetT(), p.GetX(), p.GetY());
      m.events.push_back(event);
      ResetMatcher(m);
      return;
    }

  } // end MatcherHandleOn


  void LocalSequenceEventFinder::ResetMatcher(Match &m) {
    m.state = NONE;
    m.streaks = 0;
    m.streakLength = 0;
  }

  
  void LocalSequenceEventFinder::StartMatcher(Match &m) {
    HistoryPoint& p = *(m.point);
    m.state = PREVON;
    m.streakStartTime = p.GetT();
    m.matchStartTime  = p.GetT();
    m.streakLength = 1;
    m.streaks = 0;
  }

  LocalSequenceEventFinder::~LocalSequenceEventFinder() {}

} // end namespace FindLaser

