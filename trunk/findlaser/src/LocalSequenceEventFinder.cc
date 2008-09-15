
#include "LocalSequenceEventFinder.h"

#include <string>
#include <iostream>
#include <ctime>
#include <pcrecpp.h>

#include "HistoryPoint.h"

using namespace std;

namespace FindLaser {

  LocalSequenceEventFinder::LocalSequenceEventFinder(
    const unsigned int repetitions = 2,
    const unsigned int minTriggeredEvents = 2,
    const unsigned int minUntriggeredEvents = 1,
    const double minOnTime = 0.15,
    const double minPause = 0.15
  )
    : fRepetitions(repetitions),
    fMinUntriggeredEvents(minUntriggeredEvents),
    fMinTriggeredEvents(minTriggeredEvents),
    fMinOnTime(minOnTime),
    fMinPauseTime(minPause),
    fRegexp(NULL)
  {
    const unsigned int repHalf = fRepetitions / 2;
    fMinTotalTime = (repHalf+1) * fMinOnTime + repHalf*fMinPauseTime;
    char buffer[1000];
    sprintf(
      buffer, "((?:1{%u,}0{%u}){%u})",
      fMinTriggeredEvents, fMinUntriggeredEvents, fRepetitions
    );
    fRegexp = new pcrecpp::RE(buffer);
  }

  vector<Event> LocalSequenceEventFinder::Find(list<HistoryPoint>& points) {
    const unsigned int nPoints = points.size(); // is this size() O(n)?
    vector<Event> ev;
    if ( nPoints < fRepetitions * fMinTriggeredEvents + (fRepetitions-1) * fMinUntriggeredEvents )
      return ev;
    
    const double latest = points.front().GetT();
    const double earliest = points.back().GetT();
    const double duration = latest-earliest;
    if (duration < fMinTotalTime)
      return ev;

    char* states = (char*)malloc(sizeof(char) * nPoints);

    list<HistoryPoint>::iterator curPoint = points.begin();
    for (unsigned int i = 0; i < nPoints; i++) {
      states[i] = curPoint->Triggered() ? 1 : 0 ;
      ++curPoint;
    }

    unsigned int textPos = 0;
    const int npositions = 3;
    int positions[npositions];
    vector<char*> matches;
    vector<char* pos;
    while (textPos < nPoints) {
      int matched = fRegexp->TryMatch(
        states,
        textPos,
        pcrecpp::Anchor::UNANCHORED,
        positions,
        npositions
      );
      if (matched == 0)
        break;
      matches.push_back();
      
    }
    

/*    State state = OFF;
    list<HistoryPoint>::iterator prevPoint = points.begin();
    if (prevPoint == points.end())
      return ev;
    list<HistoryPoint>::iterator curPoint = ++points.begin();
    unsigned int repetitions = fRepetitions;
    if (prevPoint->Triggered())
      state = ON;

    unsigned int streak = 0;
    double timeStreak = 0.;
    while (repetitions > 0 && curPoint != points.end()) {
      if (state == OFF) {

        if (curPoint->Triggered()) {

          if (!prevPoint->Triggered()) {
            if (streak >= fMinUntriggeredEvents && timeStreak >= fMinPauseTime) {
              if (--repetitions == 0)
                break;
            }
            else // reset!
              repetitions = fRepetitions;
            
            state = ON;
            timeStreak = (curPoint->GetT() - prevPoint->GetT()) / 2;
            streak = 1;
          } // end if previous did not trigger
          else { // both triggered
            timeStreak += curPoint->GetT() - prevPoint->GetT();
            streak += 1;
          } // end if both triggered

        } // end if current triggered
        else {
          timeStreak += curPoint->GetT() - prevPoint->GetT();
          streak += 1;
        }


      } // end if state off
      else if (state == ON) {
      } // end if state on

      prevPoint = curPoint;
      curPoint++;
    }
    */
    
  } // end Find

  LocalSequenceEventFinder::~LocalSequenceEventFinder() {
    delete fRegexp;
  }

} // end namespace FindLaser
