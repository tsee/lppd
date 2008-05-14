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

#include "myimages.h"
#include "v4lcapture.h"

#include <X11/Xlib.h>

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

void readConfig(
  const std::string& fileName, int& camBrightness,
  int& camContrast, double& colorThreshold,
  double& brightnessThreshold)
{
  //set defaults
  camBrightness       = 32000;
  camContrast         = 32000;
  brightnessThreshold = 120;
  colorThreshold      = 120;

  ifstream fileHandle;
  fileHandle.open(fileName.c_str());
  if ( !fileHandle.is_open() ) {
    cerr << " Could not read configuration file "
         << fileName << endl;
    return;
  }

  string value;

  if (!fileHandle.good()) return;
  fileHandle >> value;
  camBrightness = atoi(value.c_str());

  if (!fileHandle.good()) return;
  fileHandle >> value;
  camContrast = atoi(value.c_str());

  if (!fileHandle.good()) return;
  fileHandle >> value;
  brightnessThreshold = atof(value.c_str());

  if (!fileHandle.good()) return;
  fileHandle >> value;
  colorThreshold = atof(value.c_str());

  return;
}

void writeConfig(
  const std::string& fileName, const int& camBrightness,
  const int& camContrast, const double& colorThreshold,
  const double& brightnessThreshold)
{
  ofstream fileHandle;
  fileHandle.open(fileName.c_str());
  if ( !fileHandle.is_open() ) {
    cerr << " Could not open configuration file "
         << fileName << " for writing." <<endl;
    return;
  }

  string value;

  fileHandle << camBrightness << "\n";
  fileHandle << camContrast   << "\n";
  fileHandle << brightnessThreshold << "\n";
  fileHandle << colorThreshold << endl;

  return;
}

int main()
{

  // Initialize webcam image capturing
  //const unsigned int imageSX = 640, imageSY = 480;
  const unsigned int imageSX = 320, imageSY = 240;
  ImageCapture cap("/dev/video0");

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

  // Camera settings
  int cameraBrightness = cap.GetBrightness();
  const int cameraBrightnessMax = 65536;
  const int cameraBrightnessMin = 0;

  int cameraContrast = cap.GetContrast();
  const int cameraContrastMax = 65536;
  const int cameraContrastMin = 0;

  // spot finding thresholds
  double thresholdColor = 120;
  const double thresholdColorMin = 0;
  const double thresholdColorMax = 255;

  double thresholdLight = 220;
  const double thresholdLightMin = 0;
  const double thresholdLightMax = 255;

  const string confFile = "settings.txt";
  readConfig(confFile, cameraBrightness, cameraContrast, thresholdColor, thresholdLight);

  cap.SetCaptureProperties(cameraBrightness, cameraContrast);

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

   	
  // Initialize STL_ttf
  if(TTF_Init()==-1) {
    printf("TTF_Init: %s\n", TTF_GetError());
    exit(2);
  }
  TTF_Font* font = TTF_OpenFont("res/fonts/Vera.ttf", 16);
  SDL_Color fontBgColor={0,0,0}, fontColor={0xff,0xff,0xff};
  SDL_Surface* textSurface = NULL;


  const SDL_VideoInfo *currentVid = SDL_GetVideoInfo();
  const unsigned int screenSizeX = currentVid->current_w, screenSizeY = currentVid->current_h;

  SDL_Surface *display;

  // display depth 0 means current depth
  display = SDL_SetVideoMode( imageSX, imageSY, 0, SDL_SWSURFACE );
  //display = SDL_SetVideoMode( screenSizeX, screenSizeY, 0, SDL_SWSURFACE|SDL_FULLSCREEN );

  //display = SDL_SetVideoMode( 800, 600, 24, SDL_SWSURFACE);
//  display = SDL_SetVideoMode( 800, 600, 16, SDL_SWSURFACE|SDL_FULLSCREEN );
  //display = SDL_SetVideoMode( 800, 600, 16, SDL_SWSURFACE );
  if ( display == NULL )
  {
    fprintf(stderr, "Konnte kein Fenster 800x600px oeffnen: %s\n",
        SDL_GetError());
    exit(1);
  }
  atexit(SDL_Quit);

  // SDL structures for image display
  SDL_Surface* cursor = NULL;
  SDL_Surface* sdlimage = NULL;
  SDL_RWops *rw = NULL;

  // Load SDL cursor 
  cursor = IMG_Load("res/images/cursor.png");
  if (cursor == NULL) {
    fprintf(stderr, "Das Bild konnte nicht geladen werden:%s\n",
        SDL_GetError());
    exit(-1);
  }

  // Cursor 
  SDL_Rect cursorSourceRect;
  SDL_Rect cursorDestRect;

  cursorSourceRect.x = 0;
  cursorSourceRect.y = 0;
  cursorSourceRect.w = cursor->w;
  cursorSourceRect.h = cursor->h;

  cursorDestRect.w = cursor->w;
  cursorDestRect.h = cursor->h;

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
  double sLastCX = 0.;
  double sLastCY = 0.;
  double lastCX = 0.;
  double lastCY = 0.;
  unsigned int frame = 0;

  // other global state
  bool capture = true;
  bool continuedCapture = false;

  // text display
  string displayText  = "";
  string renderedText = "";

  // keys and key conrol
  int lastkey  = -1;
  int keyaccel = 1;

  // which image should be shown?
  // 1 = camera image
  // 2 = color projection
  // 3 = brightness projection
  int displayMode = 1;

  // global image storage
  ColorImage* captureImage = new ColorImage();

  while(running) {
    while(SDL_PollEvent(&event)) {
      switch(event.type){
        case SDL_KEYDOWN:
          switch (event.key.keysym.sym) {
           case SDLK_q:
            running = 0;
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
           case SDLK_r:
            lastkey = SDLK_r;
            break;
           case SDLK_f:
            lastkey = SDLK_f;
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
            displayText = "Color projection";
            break;
           case SDLK_3:
            displayMode = 3;
            displayText = "Brightness projection";
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
        case SDLK_r:
          thresholdColor += keyaccel;
          if (thresholdColor > thresholdColorMax)
            thresholdColor = thresholdColorMax;
          displayText = string("Color Threshold: ") + stringify(thresholdColor) + string(" / ") + stringify(thresholdColorMax);
          break;
        case SDLK_f:
          thresholdColor -= keyaccel;
          if (thresholdColor < thresholdColorMin)
            thresholdColor = thresholdColorMin;
          displayText = string("Color Threshold: ") + stringify(thresholdColor) + string(" / ") + stringify(thresholdColorMax);
          break;
        case SDLK_t:
          thresholdLight += keyaccel;
          if (thresholdLight > thresholdLightMax)
            thresholdLight = thresholdLightMax;
          displayText = string("Light Threshold: ") + stringify(thresholdLight) + string(" / ") + stringify(thresholdLightMax);
          break;
        case SDLK_g:
          thresholdLight -= keyaccel;
          if (thresholdLight < thresholdLightMin)
            thresholdLight = thresholdLightMin;
          displayText = string("Light Threshold: ") + stringify(thresholdLight) + string(" / ") + stringify(thresholdLightMax);
          break;
        case SDLK_u:
          cameraBrightness += keyaccel*50;
          if (cameraBrightness > cameraBrightnessMax)
            cameraBrightness = cameraBrightnessMax;
          cap.SetBrightness(cameraBrightness);
          displayText = string("Camera Brightness: ") + stringify(cameraBrightness) + string(" / ") + stringify(cameraBrightnessMax);
          break;
        case SDLK_j:
          cameraBrightness -= keyaccel*50;
          if (cameraBrightness < cameraBrightnessMin)
            cameraBrightness = cameraBrightnessMin;
          cap.SetBrightness(cameraBrightness);
          displayText = string("Camera Brightness: ") + stringify(cameraBrightness) + string(" / ") + stringify(cameraBrightnessMax);
          break;
        case SDLK_i:
          cameraContrast += keyaccel*200;
          if (cameraContrast > cameraContrastMax)
            cameraContrast = cameraContrastMax;
          cap.SetContrast(cameraContrast);
          displayText = string("Camera Contrast: ") + stringify(cameraContrast) + string(" / ") + stringify(cameraContrastMax);
          break;
        case SDLK_k:
          cameraContrast -= keyaccel*200;
          if (cameraContrast < cameraContrastMin)
            cameraContrast = cameraContrastMin;
          cap.SetContrast(cameraContrast);
          displayText = string("Camera Contrast: ") + stringify(cameraContrast) + string(" / ") + stringify(cameraContrastMax);
          break;
        case SDLK_c:
          capture = true;
          break;
        case SDLK_v:
          if (continuedCapture == true) {
            continuedCapture = false;
          }
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

    //GreyImage normRedProj = *captureImage->NormalizedColorProjection(red);
    //GreyImage light = *captureImage->ToGreyscale();
    unsigned int pnmsize;
    unsigned char* pnmptr;
    if (displayMode == 1) {
      pnmptr = captureImage->AsPNM(pnmsize);
    }
    else if (displayMode == 2) {
      GreyImage* colorProj = captureImage->NormalizedColorProjection(red, (unsigned char)thresholdColor, 1);
      pnmptr = colorProj->AsPNM(pnmsize);
      delete colorProj;
    }
    else if (displayMode == 3) {
      GreyImage* grey = captureImage->ToGreyscale((unsigned char)thresholdLight, 1);
      pnmptr = grey->AsPNM(pnmsize);
      delete grey;
    }

    if (sdlimage != NULL) {
      SDL_FreeSurface(sdlimage);
      sdlimage = NULL;
    }
    rw = SDL_RWFromMem(pnmptr, pnmsize);
    sdlimage = IMG_LoadTyped_RW(rw, 0, "PNM");
    SDL_FreeRW(rw);
    free(pnmptr);
    pnmptr = NULL;

    if (sdlimage == NULL) {
      fprintf(stderr, "Das Bild konnte nicht geladen werden:%s\n",
          SDL_GetError());
      exit(-1);
    }

    double cx = 0, cy = 0;
    captureImage->FindLaserCentroid(
      cx, cy, red, thresholdColor, thresholdLight    
      //cx, cy, red, thresholdColor, thresholdLight    
    );
//    if ( pow(lastCX-cx,2)+pow(lastCY-cy,2)
  
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
      //double relCX = (0.5*cx+0.3*lastCX+0.2*sLastCX) / captureImage->GetColumns();
      //double relCY = (0.5*cy+0.3*lastCY+0.2*sLastCX) / captureImage->GetRows();
      sLastCX = lastCX;
      sLastCY = lastCY;
      lastCX = cx;
      lastCY = cy;
    
      unsigned int screenCX = (unsigned int) (screenSizeX * relCX);
      unsigned int screenCY = (unsigned int) (screenSizeY * relCY);

      // Set X cursor
/*      Display *display = XOpenDisplay(NULL);
      Window root = DefaultRootWindow(display);
      XWarpPointer(display, None, root, 0, 0, 0, 0, screenCX, screenCY);
      XCloseDisplay(display);
*/
      
      cursorDestRect.x = screenCX-cursorDestRect.w/2;
      cursorDestRect.y = screenCY-cursorDestRect.h/2;
      SDL_BlitSurface(cursor, &cursorSourceRect, display, &cursorDestRect);
      //SDL_BlitSurface(sdlimage, NULL, display, NULL);
      cerr << " " << cx << " " << cy << " - " << screenCX <<" " << screenCY;
    } 
    SDL_FreeSurface(rescaledSdlImage);
   
    // Render text
    if (displayText != string("")) {
      if (renderedText != displayText) {
        if (textSurface != NULL) {
          SDL_FreeSurface(textSurface);
          textSurface = NULL;
        }
        if(!(textSurface = TTF_RenderUTF8_Shaded(font,displayText.c_str(),fontColor,fontBgColor))) {
          cerr << "Error with text rendering: " << TTF_GetError() << endl;
          return(1);
        }
        renderedText = displayText;
      } // end "needs rendering"
      SDL_BlitSurface(textSurface, NULL, display, NULL);
    }
    cerr << endl;
    
    //SDL_UpdateRects(display,1,&drect);

    // aktualisiert alles
    SDL_Flip(display);
  } // end while running



  SDL_Delay(300);

  // Das Bitmap-Surface löschen
  SDL_FreeSurface(sdlimage);
  SDL_FreeSurface(textSurface);
  textSurface = NULL;
  TTF_CloseFont(font);
  font = NULL;
  TTF_Quit();
  SDL_Quit();
  writeConfig(confFile, cameraBrightness, cameraContrast, thresholdColor, thresholdLight);
}
