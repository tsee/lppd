#include "Cursor.h"

#include <string>
#include <iostream>

#include <SDL.h>
#include <SDL_image.h>
using namespace std;
using namespace FindLaser;

namespace FindLaser {
  Cursor::Cursor(const string& cursorFile) {
    fCursorSurface = IMG_Load(cursorFile.c_str());
    if (fCursorSurface == NULL) {
      cerr << "Could not load cursor image '" << cursorFile << "', error: " << SDL_GetError() << endl;
      // FIXME throw exception here instead!
      exit(3);
    }
    
    fDestRect   = new SDL_Rect();
    fSourceRect = new SDL_Rect();

    fSourceRect->x = 0;
    fSourceRect->y = 0;
    fSourceRect->w = fCursorSurface->w;
    fSourceRect->h = fCursorSurface->h;

    fDestRect->w = fCursorSurface->w;
    fDestRect->h = fCursorSurface->h;
  }
   

  Cursor::~Cursor() {
    if (fCursorSurface != NULL)
      delete fCursorSurface;
    if (fSourceRect != NULL)
      delete fSourceRect;
    if (fDestRect != NULL)
      delete fDestRect;
  }


  void Cursor::SetDestination(unsigned int x, unsigned int y) {
    fDestRect->x = x - fDestRect->w/2;
    fDestRect->y = y - fDestRect->h/2;
  }


  void Cursor::Draw(SDL_Surface* display) {
    SDL_BlitSurface(fCursorSurface, fSourceRect, display, fDestRect);
  }

  void Cursor::Draw(SDL_Surface* display, unsigned int x, unsigned int y) {
    SetDestination(x, y);
    Draw(display);
  }
} // end namespace FindLaser
