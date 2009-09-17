#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>

#include <stdlib.h>
#include <unistd.h>

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_rotozoom.h>
#include <SDL_keyboard.h>
#include "SDL_ttf.h"

#include <X11/Xlib.h>

#include "Color.h"
#include "Image.h"
#include "ColorImage.h"
#include "GreyImage.h"

#include "ImageCapture.h"
#include "Configuration.h"
#include "GeometryCorrection.h"

using namespace std;
using namespace FindLaser;

inline std::string stringify(double x)
{
  std::ostringstream o;
  if (!(o << x)) return string("");
  return o.str();
}

inline std::string stringify(int x)
{
  std::ostringstream o;
  if (!(o << x)) return string("");
  return o.str();
}


int main()
{
  const string confFile = "settings.txt";
  Configuration cfg(confFile);
  // Just a pointer to the bowels of the cfg object:
  GeometryCorrection* gCorr = cfg.GetGeometryCorrection();

  // Initialize webcam image capturing
  const unsigned int imageSX = cfg.GetCameraImageSizeX(), imageSY = cfg.GetCameraImageSizeY();
  ImageCapture cap(cfg.GetCameraDevice().c_str());

  if (!cap.Initialize()) {
    cerr << cap.GetError() << endl;
    return(1);
  }
  if (!cap.SetImageSize(imageSX, imageSY)) {
    cerr << cap.GetError() << endl;
    return(1);
  }

  // spot finding thresholds
  const double thresholdColor = cfg.GetColorThreshold();
  const double thresholdLight = cfg.GetBrightnessThreshold();

  // Camera calib settings
  cap.SetCaptureProperties(cfg.GetCameraRelBrightness(), cfg.GetCameraRelContrast());

  // Color objects for color projection
  Color red(255,0,0);
  Color green(0,255,0);
  Color blue(0,0,255);

  // Initialize SDL display
  if ( SDL_Init(SDL_INIT_VIDEO) < 0 )
  {
    fprintf(stderr, "SDL konnte nicht initialisiert werden:  %s\n",
        SDL_GetError());
    exit(1);
  }

  const SDL_VideoInfo *currentVid = SDL_GetVideoInfo();
  const unsigned int screenSizeX = currentVid->current_w, screenSizeY = currentVid->current_h;

  SDL_Surface *display;

  // display depth 0 means current depth
  //display = SDL_SetVideoMode( imageSX, imageSY, 0, SDL_SWSURFACE );
  display = SDL_SetVideoMode( screenSizeX, screenSizeY, 0, SDL_SWSURFACE|SDL_FULLSCREEN );

//  display = SDL_SetVideoMode( 800, 600, 16, SDL_SWSURFACE|SDL_FULLSCREEN );
  if ( display == NULL )
  {
    fprintf(stderr, "Konnte kein Fenster 800x600px oeffnen: %s\n",
        SDL_GetError());
    exit(1);
  }
  atexit(SDL_Quit);

  // SDL structures for image display
  SDL_Surface* target = NULL;
  SDL_Surface* targetEmpty = NULL;
  SDL_RWops *rw = NULL;

  // Load SDL targets 
  target = IMG_Load("res/images/targetSquare.png");
  if (target == NULL) {
    fprintf(stderr, "Das Bild konnte nicht geladen werden:%s\n",
        SDL_GetError());
    exit(-1);
  }
  targetEmpty = IMG_Load("res/images/targetEmpty.png");
  if (targetEmpty == NULL) {
    fprintf(stderr, "Das Bild konnte nicht geladen werden:%s\n",
        SDL_GetError());
    exit(-1);
  }

  // Cursor 
  SDL_Rect targetSourceRect;
  SDL_Rect targetDestRect;

  targetSourceRect.x = 0;
  targetSourceRect.y = 0;
  targetSourceRect.w = target->w;
  targetSourceRect.h = target->h;

  targetDestRect.w = screenSizeX/50;
  if (targetDestRect.w < 5)
    targetDestRect.w = 5;
  targetDestRect.h = screenSizeY/50;
  if (targetDestRect.h < 5)
    targetDestRect.h = 5;

  // init eventloop
  SDL_Event event;
  int running = 1;

  // init position averaging
  // FIXME replace with proper history!
  const unsigned int historyLength = 50;
  double lastCX[historyLength];
  double lastCY[historyLength];
  double distSq[historyLength];
  for (unsigned int i = 0; i < historyLength; i++) {
    lastCX[i] = -1.;
    lastCY[i] = -1.;
  }

  unsigned int frame = 0;

  // global image storage
  ColorImage* captureImage = new ColorImage();

  // 1 = top left, 2 = top right, 3 = bottom right, 4 = bottom left
  unsigned int edge = 1;

  while(running) {
    while(SDL_PollEvent(&event)) {
      switch(event.type){
        case SDL_KEYDOWN:
          switch (event.key.keysym.sym) {
           case SDLK_q:
            running = 0;
            break;
           case SDLK_w:
            SDL_BlitSurface(targetEmpty, &targetSourceRect, display, &targetDestRect);
            edge++;
            break;
          } // end switch which key
          break;
        case SDL_QUIT:
          running = 0;
          break;
        case SDL_KEYUP:
          break;
      } // end switch key event type
    } // end while polling

    unsigned int size = 0;
    unsigned char* imgptr = cap.CaptureImagePointer(size);
    captureImage->ReverseCopyFromMemory(imageSX, imageSY, imgptr);
    
    double cx = 0, cy = 0;
    captureImage->FindLaserCentroid(
      cx, cy, red, thresholdColor, thresholdLight    
    );
  
    //SDL_BlitSurface(QuellSurface, Quellbereich, ZielSurface, Zielbereich)

    switch (edge) {
      case 1:
        targetDestRect.x = 0;
        targetDestRect.y = 0;
        break;
      case 2:
        targetDestRect.x = screenSizeX-targetDestRect.w;
        targetDestRect.y = 0;
        break;
      case 3:
        targetDestRect.x = screenSizeX-targetDestRect.w;
        targetDestRect.y = screenSizeY-targetDestRect.h;
        break;
      case 4:
        targetDestRect.x = 0;
        targetDestRect.y = screenSizeY-targetDestRect.h;
        break;
      case 5:
        running = 0;
        break;
    }
    SDL_BlitSurface(target, &targetSourceRect, display, &targetDestRect);

    for (unsigned int i = historyLength-1; i > 0; i--) {
      lastCX[i] = lastCX[i-1];
      lastCY[i] = lastCY[i-1];
      distSq[i] = distSq[i-1];
    }
    lastCX[0] = cx;
    lastCY[0] = cy;
    distSq[0] = pow(cx-lastCX[1], 2)+pow(cy-lastCY[1], 2);

    const unsigned int maxDistSq = 5*5;

    unsigned int lastInvalid = historyLength;
    for (unsigned int i = 0; i < historyLength; i++) {
      if (lastCX[i] < 0. || distSq[i] > maxDistSq) {
        lastInvalid = i;
        break;
      }
    }

    for (unsigned int i = 0; i < historyLength-1; i++) {
      cerr << lastCX[i] << " ";
    }
    cerr << endl;

    if (cx >= 0. && lastInvalid >= historyLength-1) {
      double avrgX = 0.;
      double avrgY = 0.;
      for (unsigned int i = 0; i < historyLength; i++) {
        avrgX += lastCX[i];
        avrgY += lastCY[i];
      }
      avrgX /= (double)historyLength;
      avrgY /= (double)historyLength;
      cout << avrgX << " " << avrgY << endl;
      SDL_BlitSurface(targetEmpty, &targetSourceRect, display, &targetDestRect);
      edge++;
      for (unsigned int i = 0; i < historyLength; i++) {
        lastCX[i] = -1;
        lastCY[i] = -1;
        distSq[i] = 1e9;
      }
    }

    cerr << cx << " " << cy << endl;
    switch (edge) {
      case 1:
        gCorr->SetImageUpperLeft( cx, cy );
        break;
      case 2:
        gCorr->SetImageUpperRight( cx, cy );
        break;
      case 3:
        gCorr->SetImageLowerRight( cx, cy );
        break;
      case 4:
        gCorr->SetImageLowerLeft( cx, cy );
        break;
    }
   
    // aktualisiert alles
    SDL_Flip(display);
  } // end while running

  SDL_Delay(300);

  cfg.WriteConfiguration();
  
  // Das Bitmap-Surface löschen
  SDL_FreeSurface(target);
  SDL_Quit();

}
