
#include "LocalSequenceEventFinder.h"

#include <string>
#include <iostream>

#include "HistoryPoint.h"

using namespace std;

namespace FindLaser {

  LocalSequenceEventFinder::LocalSequenceEventFinder()
  {
  }

  vector<Event> LocalSequenceEventFinder::Find(const list<HistoryPoint>& points) {
    vector<Event> events;
    return events;
  }


} // end namespace FindLaser
