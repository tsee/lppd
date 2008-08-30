#ifndef __FindLaser__Cursor_h
#define __FindLaser__Cursor_h

#include <string>

class SDL_Surface;
class SDL_Rect;

namespace FindLaser {
  class Cursor {
    public:
      Cursor(const std::string& cursorFile);
      ~Cursor();

      void SetDestination(unsigned int x, unsigned int y);
      void Draw(SDL_Surface* display);
      void Draw(SDL_Surface* display, unsigned int x, unsigned int y);

    private:
      SDL_Surface* fCursorSurface;
      SDL_Rect* fSourceRect;
      SDL_Rect* fDestRect;
  }; // end class Cursor
} // end namespace FindLaser

#endif
