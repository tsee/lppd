
#include "EventFinder.h"

#include <string>
#include <iostream>

#include "HistoryPoint.h"

using namespace std;

namespace FindLaser {

  EventFinder::EventFinder()
  {
  }

  vector<Event> EventFinder::Find(const list<HistoryPoint>& points) {
    cerr << "Calling the base EventFinder's Find() method is pointless." <<endl;
    vector<Event> events;
    return events;
  }


} // end namespace FindLaser
