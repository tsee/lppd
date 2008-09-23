#include "Color.h"
#include "Image.h"
#include "ColorImage.h"
#include "GreyImage.h"

#include "ImageCapture.h"
#include "Configuration.h"
#include "GeometryCorrection.h"

#include "History.h"
#include "HistoryPoint.h"
#include "Event.h"
#include "LocalSequenceEventFinder.h"

#include <string>
#include <vector>
#include <queue>
#include <iostream>
#include <ctime>

#include <X11/Xlib.h>
#include <stdio.h>


using namespace FindLaser;
using namespace std;

int main (int argc, char**  argv) {
  
  const string confFile = "settings.txt";
  Configuration cfg(confFile);
  // Just a pointer to the bowels of the cfg object:
  GeometryCorrection* geoCorr = cfg.GetGeometryCorrection();

  const unsigned int imageSX = cfg.GetCameraImageSizeX(), imageSY = cfg.GetCameraImageSizeY();

  ImageCapture cap(cfg.GetCameraDevice());
  cap.SetVerbosity(1);

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
  cout << "Brightness:           " << cap.GetBrightness()    << endl;
  cout << "Contrast:             " << cap.GetContrast()    << endl;
  const double thresholdLight = cfg.GetBrightnessThreshold();
  const double thresholdColor = cfg.GetColorThreshold();
  cout << "Brightness threshold: " << thresholdLight    << endl;
  cout << "Color threshold:      " << thresholdColor    << endl;

  Color red(255,0,0);
  Color green(0,255,0);
  Color blue(0,0,255);

  History h;
  LocalSequenceEventFinder* finder = new LocalSequenceEventFinder();
  finder->SetVerbosity(5);
  h.AddEventFinder((EventFinder*)finder);


  const unsigned int screenSizeX = geoCorr->GetTargetImageWidth();
  const unsigned int screenSizeY = geoCorr->GetTargetImageHeight();

  // setup history
  History history;
  LocalSequenceEventFinder* clickFinder = new LocalSequenceEventFinder();
  //clickFinder->SetVerbosity(5);
  history.AddEventFinder((EventFinder*)clickFinder);

  double sLastCX = 0.;
  double sLastCY = 0.;
  double lastCX = 0.;
  double lastCY = 0.;
  unsigned int frame = 0;
  ColorImage img;
  while (1) {
    unsigned int size = 0;
    unsigned char* imgptr = cap.CaptureImagePointer(size);

/*    ColorImage img2;
    img2.ReverseCopyFromMemory(imageSX, imageSY, imgptr);
    unsigned int size2 = 0;
    unsigned char* ppmdata = img2.AsPNM(size2);
    char buffer[1024];
    sprintf(buffer, "foo_%u.pnm", frame);
    FILE *fp = fopen(buffer, "wb");
    fwrite(ppmdata, size2, 1, fp);
    fclose(fp);
    //img2.SaveAsJPEG("foo.jpg",100);
*/

    double cx = 0, cy = 0;
    img.PtrFindLaserCentroid(
      imgptr, imageSX, imageSY, true, cx, cy, red, thresholdColor, thresholdLight
    );

//    if ( pow(lastCX-cx,2)+pow(lastCY-cy,2)
  
    frame++;
    cerr << frame;

    HistoryPoint* hPoint;
    clock_t frameTime = clock();
    if (cx >= 0) {
      double relCX = (0.5*cx+0.3*lastCX+0.2*sLastCX) / imageSX;
      double relCY = (0.5*cy+0.3*lastCY+0.2*sLastCX) / imageSY;
      sLastCX = lastCX;
      sLastCY = lastCY;
      lastCX = cx;
      lastCY = cy;
    
      unsigned int screenCX = (unsigned int) (screenSizeX * relCX);
      unsigned int screenCY = (unsigned int) (screenSizeY * relCY);
      float x, y;
      geoCorr->GetImageCoordinates(cx, cy, x, y);
      screenCX = (unsigned int) x;
      screenCY = (unsigned int) y;

      Display *display = XOpenDisplay(NULL);
      Window root = DefaultRootWindow(display);
      XWarpPointer(display, None, root, 0, 0, 0, 0, screenCX, screenCY);
      XCloseDisplay(display);
      cerr << " " << cx << " " << cy << " - " << screenCX <<" " << screenCY;

      hPoint = new HistoryPoint(frameTime, x/screenSizeX, y/screenSizeY, true);
    }
    else
      hPoint = new HistoryPoint(frameTime, -1., -1., false);

    history.AddPoint(*hPoint);
    delete hPoint;

    // Process events!
    history.FindEvents();
    queue<Event> events = history.GetEvents();
    while (!events.empty()) {
      Event* e = &(events.front());
      PosEvent* posE = dynamic_cast<PosEvent*>(e);
      if (posE != NULL)
        cout << "Got PosEvent, time: " << posE->GetT() << ", x: " << posE->GetX() << ", y: " << posE->GetY() << endl;
      else
        cout << "Got Event, time: " << e->GetT() << endl;
      events.pop();
    }
    history.ClearEvents();
    
    cerr << endl;
  }

  //light.SaveAsJPEG("normRedProj.jpg", 100);
  //

  //GreyImage redCut = *normRedProj.CutOnThreshold((unsigned char)thresholdColor, 0);
  //redCut.SaveAsJPEG("redCut.jpg", 100);
  
  /*unsigned char* ppmdata = redCut.AsPNM(size);
  FILE *fp = fopen("redCut.pnm", "wb");
  fwrite(ppmdata, size, 1, fp);
  fclose(fp);
  */
  return(0);
}


