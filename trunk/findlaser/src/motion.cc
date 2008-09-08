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

#include "Color.h"
#include "Image.h"
#include "ColorImage.h"
#include "GreyImage.h"

#include "ImageCapture.h"
#include "Configuration.h"
#include "Cursor.h"

using namespace std;
using namespace FindLaser;

/*************************************/
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

/*************************************/
void toggleFullScreen(
  SDL_Surface** display,
  unsigned int xFull, unsigned int yFull,
  unsigned int xNFull, unsigned yNFull)
{
  static bool fullscreen = true;
  if (fullscreen == false) {
    fullscreen = true;
    *display = SDL_SetVideoMode( xFull, yFull, 0, SDL_SWSURFACE|SDL_FULLSCREEN );
  }
  else {
    fullscreen = false;
    *display = SDL_SetVideoMode( xNFull, yNFull, 0, SDL_SWSURFACE );
  }
}

/*************************************/
bool updateText(
  const string& displayText, SDL_Surface* display
) {
  static SDL_Surface* textSurface = NULL;
  static string renderedText = "";
  static TTF_Font* font = TTF_OpenFont("res/fonts/Vera.ttf", 16);
  static SDL_Color fontBgColor={0,0,0};
  static SDL_Color fontColor={0xff,0xff,0xff};

  if (displayText == string(""))
    return false;
  if (renderedText == displayText)
    return false;

  // cleanup old text
  if (textSurface != NULL) {
    SDL_FreeSurface(textSurface);
    textSurface = NULL;
  }

  if(!(textSurface = TTF_RenderUTF8_Shaded(font,displayText.c_str(),fontColor,fontBgColor))) {
    cerr << "Error with text rendering: " << TTF_GetError() << endl;
    return(1);
  }
  renderedText = displayText;
  SDL_BlitSurface(textSurface, NULL, display, NULL);

  return true;
}

/*************************************/
int main()
{
  const string confFile = "settings.txt";
  Configuration cfg(confFile);

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

  /*if (!cap.AdjustBrightness()) {
    cerr << cap.GetError() << endl;
    return(1);
  }
  */

  cap.SetCaptureProperties(cfg.GetCameraBrightness(), cfg.GetCameraContrast());

  // Initialize SDL display
  if ( SDL_Init(SDL_INIT_VIDEO) < 0 )
  {
    cerr << "Could not initialize SDL: " << SDL_GetError() << endl;
    exit(1);
  }

   	
  // Initialize SDL_ttf
  if(TTF_Init()==-1) {
    cerr << "SDL_TTF: TTF_Init() failed: " << TTF_GetError() << endl;
    exit(2);
  }

  const SDL_VideoInfo *currentVid = SDL_GetVideoInfo();
  const unsigned int screenSizeX = currentVid->current_w, screenSizeY = currentVid->current_h;

  SDL_Surface *display;

  toggleFullScreen(&display, screenSizeX, screenSizeY, imageSX, imageSY);
  if ( display == NULL ) {
    cerr << "Could not create SDL display: " << SDL_GetError() << endl;
    exit(1);
  }
  atexit(SDL_Quit);

  // SDL structures for image display
  SDL_Surface* sdlimage = NULL;
  SDL_RWops *rw = NULL;

  // Load SDL cursor 
  const string cursorFile = "res/images/cursor.png";
  FindLaser::Cursor cursor(cursorFile);

  // Image target rectangle
  SDL_Rect imgSourceRect;
  SDL_Rect imgDestRect;
  imgSourceRect.x = 0;
  imgSourceRect.y = 0;
  imgSourceRect.w = screenSizeX;
  imgSourceRect.h = screenSizeY;

  imgDestRect.x = 0;
  imgDestRect.y = 0;
  imgDestRect.w = screenSizeX;
  imgDestRect.h = screenSizeY;

  // init eventloop
  SDL_Event event;
  int running = 1;

  // init position averaging 
  unsigned int frame = 0;

  // other global state
  bool capture = true;
  bool continuedCapture = false;

  // text display
  string displayText  = "";

  // keys and key conrol
  int lastkey  = -1;
  int keyaccel = 1;

  // which image should be shown?
  // 1 = camera image
  // 2 = brightness projection
  // 3 = difference
  int displayMode = 1;

  // global image storage
  ColorImage* captureImage = new ColorImage();
  GreyImage* prevImage = NULL;

  while(running) {
    while(SDL_PollEvent(&event)) {
      switch(event.type){
        case SDL_KEYDOWN:
          switch (event.key.keysym.sym) {
           case SDLK_q:
            running = 0;
            break;
           case SDLK_a:
            toggleFullScreen(&display, screenSizeX, screenSizeY, imageSX, imageSY);
            break;
           case SDLK_u:
            lastkey = SDLK_u;
            break;
           case SDLK_j:
            lastkey = SDLK_j;
            break;
           case SDLK_i:
            lastkey = SDLK_i;
            break;
           case SDLK_k:
            lastkey = SDLK_k;
            break;
           case SDLK_o:
            lastkey = SDLK_o;
            break;
           case SDLK_l:
            lastkey = SDLK_l;
            break;
           case SDLK_t:
            lastkey = SDLK_t;
            break;
           case SDLK_g:
            lastkey = SDLK_g;
            break;
           case SDLK_c:
            lastkey = SDLK_c;
            break;
           case SDLK_v:
            lastkey = SDLK_v;
            break;
           case SDLK_1:
            displayMode = 1;
            displayText = "Camera image";
            break;
           case SDLK_2:
            displayMode = 2;
            displayText = "Brightess projection";
            break;
           case SDLK_3:
            displayMode = 3;
            displayText = "Difference";
            break;
          } // end switch which key
          break;
        case SDL_QUIT:
          running = 0;
          break;
        case SDL_KEYUP:
          lastkey = -1;
          keyaccel = 1;
          break;
      } // end switch key event type
    } // end while polling

    if (lastkey > -1) {
      switch (lastkey) {
        case SDLK_t:
          cfg.SetBrightnessThreshold( cfg.GetBrightnessThreshold() + keyaccel );
          displayText = string("Light Threshold: ") + stringify(cfg.GetBrightnessThreshold())
                        + string(" / ") + stringify(cfg.GetBrightnessThresholdMax());
          break;
        case SDLK_g:
          cfg.SetBrightnessThreshold( cfg.GetBrightnessThreshold() - keyaccel );
          displayText = string("Light Threshold: ") + stringify(cfg.GetBrightnessThreshold())
                        + string(" / ") + stringify(cfg.GetBrightnessThresholdMax());
          break;
        case SDLK_u:
          cfg.SetCameraBrightness( cfg.GetCameraBrightness() + keyaccel*50 );
          cap.SetBrightness( cfg.GetCameraBrightness() );
          displayText = string("Camera Brightness: ") + stringify(cfg.GetCameraBrightness())
                        + string(" / ") + stringify(cfg.GetCameraBrightnessMax());
          break;
        case SDLK_j:
          cfg.SetCameraBrightness( cfg.GetCameraBrightness() - keyaccel*50 );
          cap.SetBrightness( cfg.GetCameraBrightness() );
          displayText = string("Camera Brightness: ") + stringify(cfg.GetCameraBrightness())
                        + string(" / ") + stringify(cfg.GetCameraBrightnessMax());
          break;
        case SDLK_i:
          cfg.SetCameraContrast( cfg.GetCameraContrast() + keyaccel*200 );
          cap.SetContrast( cfg.GetCameraContrast() );
          displayText = string("Camera Contrast: ") + stringify(cfg.GetCameraContrast())
                        + string(" / ") + stringify(cfg.GetCameraContrastMax());
          break;
        case SDLK_k:
          cfg.SetCameraContrast( cfg.GetCameraContrast() - keyaccel*200 );
          cap.SetContrast( cfg.GetCameraContrast() );
          displayText = string("Camera Contrast: ") + stringify(cfg.GetCameraContrast())
                        + string(" / ") + stringify(cfg.GetCameraContrastMax());
          break;
        case SDLK_c:
          capture = true;
          break;
        case SDLK_v:
          if (continuedCapture == true)
            continuedCapture = false;
          else
            continuedCapture = true;
          break;
      }
      if (keyaccel < 5) keyaccel++;
    } // end we have a key

    if (capture || continuedCapture) {
      capture = false;
      unsigned int size = 0;
      unsigned char* imgptr = cap.CaptureImagePointer(size);
      captureImage->ReverseCopyFromMemory(imageSX, imageSY, imgptr);
    }

    GreyImage *thisImage = captureImage->ToGreyscale();
    GreyImage *diffImage = thisImage->Difference(prevImage);

    unsigned int pnmsize;
    unsigned char* pnmptr;
    if (displayMode == 1) {
      pnmptr = captureImage->AsPNM(pnmsize);
    }
    else if (displayMode == 2) {
      pnmptr = thisImage->AsPNM(pnmsize);
    }
    else if (displayMode == 3) {
      if (diffImage == NULL)
        pnmptr = thisImage->AsPNM(pnmsize);
      else
        pnmptr = diffImage->AsPNM(pnmsize);
    }

    if (sdlimage != NULL) {
      SDL_FreeSurface(sdlimage);
      sdlimage = NULL;
    }
    rw = SDL_RWFromMem(pnmptr, pnmsize);
    sdlimage = IMG_LoadTyped_RW(rw, 0, (char*)"PNM");
    SDL_FreeRW(rw);
    free(pnmptr);
    pnmptr = NULL;

    if (sdlimage == NULL) {
      cerr << "Could not load image: " << SDL_GetError() << endl;
      exit(4);
    }

    float cx = 0, cy = 0;
    if (diffImage != NULL)
      diffImage->CalcCentroid(cx, cy);
  
    frame++;
    cerr << frame;

    double zoomx = (double)currentVid->current_w / sdlimage->w;
    double zoomy = (double)currentVid->current_h / sdlimage->h;
    SDL_Surface* rescaledSdlImage = zoomSurface(sdlimage, zoomx, zoomy, 0);

    //SDL_BlitSurface(QuellSurface, Quellbereich, ZielSurface, Zielbereich)
    SDL_BlitSurface(rescaledSdlImage, &imgSourceRect, display, &imgDestRect);

    if (cx >= 0) {
      double relCX = cx / captureImage->GetColumns();
      double relCY = cy / captureImage->GetRows();
      unsigned int screenCX = (unsigned int) (screenSizeX * relCX);
      unsigned int screenCY = (unsigned int) (screenSizeY * relCY);
      //float x, y;
      //geoCorr->GetImageCoordinates(cx, cy, x, y);
      //screenCX = (unsigned int) x;
      //screenCY = (unsigned int) y;

      cursor.Draw(display, screenCX, screenCY);
      //SDL_BlitSurface(sdlimage, NULL, display, NULL);
      cerr << " " << cx << " " << cy << " - " << screenCX <<" " << screenCY;
    }
    
    SDL_FreeSurface(rescaledSdlImage);
   
    updateText(displayText, display);
    cerr << endl;
    
    //SDL_UpdateRects(display,1,&drect);

    // aktualisiert alles
    SDL_Flip(display);

    delete prevImage;
    prevImage = thisImage;
  } // end while running



  SDL_Delay(300);

  // Das Bitmap-Surface löschen
  SDL_FreeSurface(sdlimage);
  //SDL_FreeSurface(textSurface);
  //textSurface = NULL;
  //TTF_CloseFont(font);
  //font = NULL;
  TTF_Quit();
  SDL_Quit();
}
