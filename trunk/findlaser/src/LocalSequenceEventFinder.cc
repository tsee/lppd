
#include "LocalSequenceEventFinder.h"

#include <string>
#include <iostream>
#include <ctime>

#include "HistoryPoint.h"
#include "Event.h"

using namespace std;

namespace FindLaser {

  LocalSequenceEventFinder::LocalSequenceEventFinder(
    const unsigned int repetitions = 2,
    const unsigned int minTriggeredEvents = 2,
    const unsigned int minUntriggeredEvents = 1,
    const clock_t minOnTime = 0.15,
    const clock_t minPause = 0.15,
    const clock_t maxPause = 1.00
  )
    : fRepetitions(repetitions),
    fMinUntriggeredEvents(minUntriggeredEvents),
    fMinTriggeredEvents(minTriggeredEvents),
    fMinOnTime(minOnTime),
    fMinPauseTime(minPause),
    fMaxPauseTime(maxPause)
  {
    const clock_t repHalf = (clock_t) (fRepetitions / 2);
    fMinTotalTime = (repHalf+1) * fMinOnTime + repHalf*fMinPauseTime;
  }

  vector<Event> LocalSequenceEventFinder::Find(list<HistoryPoint>& points) {
    const unsigned int nPoints = points.size(); // is this size() O(n)?
    vector<Event> ev;
    if ( nPoints < fRepetitions * fMinTriggeredEvents + (fRepetitions-1) * fMinUntriggeredEvents )
      return ev;
    
    const clock_t latest = points.front().GetT();
    const clock_t earliest = points.back().GetT();
    const clock_t duration = latest-earliest;
    if (duration < fMinTotalTime)
      return ev;

    Match matcher;
    matcher.state = NONE;
    matcher.point = points.begin();
    matcher.end = points.end();

    unsigned int repetitions = fRepetitions;

    while (matcher.point != matcher.end) {
      switch (matcher.state) {
        case NONE:
          MatcherHandleNone(matcher);
          break;
        case PREVOFF:
          MatcherHandleOff(matcher);
          break;
        case PREVON:
          MatcherHandleOff(matcher);
          break;
        default:
          cerr << "unknown state" << endl;
          break;
      }
      ++matcher.point;
    }
    
  } // end Find


  void LocalSequenceEventFinder::MatcherHandleNone(Match& m) {
    HistoryPoint& p = *(m.point);
    if (p.Triggered())
      StartMatcher(m);
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

    // check that streak is long enough in n events
    // check that streak is long/short enough in time
    const clock_t streakTime = p.GetT() - m.streakStartTime;
    if (m.streakLength < fMinUntriggeredEvents
        || streakTime < fMinPauseTime
        || streakTime > fMaxPauseTime)
    {
      ResetMatcher(m);
      m.state = PREVON;
      m.streakStartTime = p.GetT();
      m.matchStartTime = p.GetT();
      m.streakLength = 1;
      return; // FAIL, restart
    }
    
    // Passed streak if we got here.
    m.state = PREVOFF;
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

