#ifndef __FindLaser__Event_h
#define __FindLaser__Event_h

#include <ctime>

namespace FindLaser {

  class Event {
    public:
      Event(const clock_t t);
      virtual ~Event() {};

      clock_t GetT() { return fTime; }
      void SetT(const clock_t t) { fTime = t; }

    protected:
      Event() {};

    private:
      clock_t fTime;
  }; // end class Event

  class PosEvent : public Event {
    public:
      PosEvent(const clock_t t, const double& x, const double& y);
      virtual ~PosEvent() {};

    private:
      double fX;
      double fY;
  }; // end class Event


} // end namespace FindLaser

#endif
