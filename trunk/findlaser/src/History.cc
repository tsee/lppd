
#include "History.h"
#include <string>
#include <iostream>

#include "EventType.h"

using namespace std;

namespace FindLaser {

  History::History()
    : fMemoryLength(5.0)
  {
  }

  void History::AddPoint(const HistoryPoint& histPoint) {
    fPoints.push_back(histPoint);
  }

  void History::DiscardOldPoints() {
    if (fPoints.empty()) return;
    const clock_t discardBefore = clock()-fMemoryLength;
    while (!fPoints.empty() && fPoints.front().GetT() < discardBefore) {
      fPoints.pop_front();
    }
    return;
  }

  void History::AddEventType(const EventType& evType) {
    // FIXME check for equality here
    fEventTypes.push_back(evType);
  }

  void History::FindEvents() {
    for (unsigned int i = 0; i < fEventTypes.size(); i++) {
      const vector<Event>& ev = fEventTypes[i].Find(fPoints);
      for (unsigned int i = 0; i < ev.size(); i++) {
        fEvents.push(ev[i]);
      }
    } // end for event types
  }


} // end namespace FindLaser 

