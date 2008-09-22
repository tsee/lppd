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

      double GetX() { return fX; }
      void GetX(const double x) { fX = x; }

      double GetY() { return fY; }
      void GetY(const double y) { fY = y; }

    private:
      double fX;
      double fY;
  }; // end class Event


} // end namespace FindLaser

#endif
