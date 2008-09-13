#ifndef __FindLaser__Event_h
#define __FindLaser__Event_h

#include <ctime>

namespace FindLaser {

  class Event {
    public:
      Event(const clock_t t);
      virtual ~Event() {};

    private:
      clock_t fTime;
  }; // end class Event

} // end namespace FindLaser

#endif
