#include <string>
#include <iostream>

#include "History.h"
#include "LocalSequenceEventFinder.h"
#include "Event.h"

using namespace std;
using namespace FindLaser;

int main (int argc, char** argv) {
  History h;
  LocalSequenceEventFinder* finder = new LocalSequenceEventFinder();
  finder->SetVerbosity(5);
  h.AddEventFinder((EventFinder*)finder);

  vector<HistoryPoint> points;
  points.push_back(HistoryPoint(0.1*CLOCKS_PER_SEC, 1.0, 1.0, false));
  points.push_back(HistoryPoint(0.2*CLOCKS_PER_SEC, 1.0, 1.0, true));
  points.push_back(HistoryPoint(0.3*CLOCKS_PER_SEC, 1.0, 1.0, true));
  points.push_back(HistoryPoint(0.4*CLOCKS_PER_SEC, 1.0, 1.0, true));
  points.push_back(HistoryPoint(0.5*CLOCKS_PER_SEC, 1.0, 1.0, true));
  points.push_back(HistoryPoint(0.6*CLOCKS_PER_SEC, 1.0, 1.0, false));
  points.push_back(HistoryPoint(0.7*CLOCKS_PER_SEC, 1.0, 1.0, false));
  points.push_back(HistoryPoint(0.75*CLOCKS_PER_SEC, 1.0, 1.0, false));
  points.push_back(HistoryPoint(0.8*CLOCKS_PER_SEC, 1.0, 1.0, false));
  points.push_back(HistoryPoint(0.9*CLOCKS_PER_SEC, 1.0, 1.0, true));
  points.push_back(HistoryPoint(1.0*CLOCKS_PER_SEC, 1.0, 1.0, true));
  points.push_back(HistoryPoint(1.1*CLOCKS_PER_SEC, 1.0, 1.0, true));
  points.push_back(HistoryPoint(1.2*CLOCKS_PER_SEC, 1.0, 1.0, true));
  points.push_back(HistoryPoint(1.3*CLOCKS_PER_SEC, 1.0, 1.0, true));
  points.push_back(HistoryPoint(1.4*CLOCKS_PER_SEC, 1.0, 1.0, false));
  points.push_back(HistoryPoint(1.5*CLOCKS_PER_SEC, 1.0, 1.0, false));
  points.push_back(HistoryPoint(1.6*CLOCKS_PER_SEC, 1.0, 1.0, false));
  points.push_back(HistoryPoint(1.7*CLOCKS_PER_SEC, 1.0, 1.0, false));

  for (unsigned int i = 0; i < points.size(); i++) {
    h.AddPoint(points[i]);
  }

  h.FindEvents();
  std::queue<Event>& events = h.GetEvents();

  if (events.empty()) {
    cout << "No events found!" << endl;
  }

  while (!events.empty()) {
    Event* e = &(events.front());
    PosEvent* posE = dynamic_cast<PosEvent*>(e);
    if (posE != NULL)
      cout << "Got PosEvent, time: " << posE->GetT() << ", x: " << posE->GetX() << ", y: " << posE->GetY() << endl;
    else
      cout << "Got Event, time: " << e->GetT() << endl;
    events.pop();
  }

}


